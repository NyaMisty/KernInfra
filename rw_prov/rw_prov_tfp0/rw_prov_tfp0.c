#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <mach/mach.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <CoreFoundation/CoreFoundation.h>

typedef uint64_t kaddr_t;
task_t tfp0 = TASK_NULL;
kaddr_t kslide;

kern_return_t
init_tfp0(void) {
	kern_return_t ret = task_for_pid(mach_task_self(), 0, &tfp0);
	mach_port_t host;

	if(ret != KERN_SUCCESS) {
		host = mach_host_self();
		if(MACH_PORT_VALID(host)) {
			printf("host: 0x%" PRIX32 "\n", host);
			ret = host_get_special_port(host, HOST_LOCAL_NODE, 4, &tfp0);
			mach_port_deallocate(mach_task_self(), host);
		}
	}
	if(ret == KERN_SUCCESS && MACH_PORT_VALID(tfp0)) {
		return ret;
	}
	return KERN_FAILURE;
}


kern_return_t
mach_vm_write(vm_map_t, mach_vm_address_t, vm_offset_t, mach_msg_type_number_t);

kern_return_t
mach_vm_read_overwrite(vm_map_t, mach_vm_address_t, mach_vm_size_t, mach_vm_address_t, mach_vm_size_t *);

kern_return_t
mach_vm_machine_attribute(vm_map_t, mach_vm_address_t, mach_vm_size_t, vm_machine_attribute_t, vm_machine_attribute_val_t *);

kern_return_t
mach_vm_region(vm_map_t, mach_vm_address_t *, mach_vm_size_t *, vm_region_flavor_t, vm_region_info_t, mach_msg_type_number_t *, mach_port_t *);

#	ifndef MIN
#		define MIN(a, b) ((a) < (b) ? (a) : (b))
#	endif

#	define KADDR_FMT "0x%" PRIX64

static kern_return_t
__kread_buf_tfp0(kaddr_t addr, void *buf, mach_vm_size_t sz) {
	mach_vm_address_t p = (mach_vm_address_t)buf;
	mach_vm_size_t read_sz, out_sz = 0;

	while(sz != 0) {
		read_sz = MIN(sz, vm_kernel_page_size - (addr & vm_kernel_page_mask));
		if(mach_vm_read_overwrite(tfp0, addr, read_sz, p, &out_sz) != KERN_SUCCESS || out_sz != read_sz) {
			return KERN_FAILURE;
		}
		p += read_sz;
		sz -= read_sz;
		addr += read_sz;
	}
	return KERN_SUCCESS;
}

static kern_return_t
__kwrite_buf_tfp0(kaddr_t addr, const void *buf, mach_msg_type_number_t sz) {
	vm_machine_attribute_val_t mattr_val = MATTR_VAL_CACHE_FLUSH;
	mach_vm_address_t p = (mach_vm_address_t)buf;
	mach_msg_type_number_t write_sz;

	while(sz != 0) {
		write_sz = (mach_msg_type_number_t)MIN(sz, vm_kernel_page_size - (addr & vm_kernel_page_mask));
		if(mach_vm_write(tfp0, addr, p, write_sz) != KERN_SUCCESS || mach_vm_machine_attribute(tfp0, addr, write_sz, MATTR_CACHE, &mattr_val) != KERN_SUCCESS) {
			return KERN_FAILURE;
		}
		p += write_sz;
		sz -= write_sz;
		addr += write_sz;
	}
	return KERN_SUCCESS;
}

#define LOADED_KEXT_SUMMARY_HDR_NAME_OFF (0x10)
#define LOADED_KEXT_SUMMARY_HDR_ADDR_OFF (0x60)
#define VM_KERN_MEMORY_OSKEXT (5)
#define kOSBundleLoadAddressKey "OSBundleLoadAddress"

CFDictionaryRef
OSKextCopyLoadedKextInfo(CFArrayRef, CFArrayRef);

kern_return_t
init_kslide(void) {
	mach_msg_type_number_t cnt = TASK_DYLD_INFO_COUNT;
	vm_region_extended_info_data_t extended_info;
	kaddr_t addr, kext_addr, kext_addr_slid;
	CFDictionaryRef kexts_info, kext_info;
	task_dyld_info_data_t dyld_info;
	char kext_name[KMOD_MAX_NAME];
	CFStringRef kext_name_cf;
	CFNumberRef kext_addr_cf;
	mach_port_t object_name;
	CFArrayRef kext_names;
	mach_vm_size_t sz;

	if(task_info(tfp0, TASK_DYLD_INFO, (task_info_t)&dyld_info, &cnt) == KERN_SUCCESS) {
		kslide = dyld_info.all_image_info_size;
	}
	if(kslide == 0) {
		cnt = VM_REGION_EXTENDED_INFO_COUNT;
		for(addr = 0; mach_vm_region(tfp0, &addr, &sz, VM_REGION_EXTENDED_INFO, (vm_region_info_t)&extended_info, &cnt, &object_name) == KERN_SUCCESS; addr += sz) {
			mach_port_deallocate(mach_task_self(), object_name);
			if(extended_info.protection == VM_PROT_READ && extended_info.user_tag == VM_KERN_MEMORY_OSKEXT) {
				if(__kread_buf_tfp0(addr + LOADED_KEXT_SUMMARY_HDR_NAME_OFF, kext_name, sizeof(kext_name)) == KERN_SUCCESS) {
					printf("kext_name: %s\n", kext_name);
					if(__kread_buf_tfp0(addr + LOADED_KEXT_SUMMARY_HDR_ADDR_OFF, &kext_addr_slid, sizeof(kaddr_t)) == KERN_SUCCESS) {
						printf("kext_addr_slid: " KADDR_FMT "\n", kext_addr_slid);
						if((kext_name_cf = CFStringCreateWithCStringNoCopy(kCFAllocatorDefault, kext_name, kCFStringEncodingUTF8, kCFAllocatorNull)) != NULL) {
							if((kext_names = CFArrayCreate(kCFAllocatorDefault, (const void **)&kext_name_cf, 1, &kCFTypeArrayCallBacks)) != NULL) {
								if((kexts_info = OSKextCopyLoadedKextInfo(kext_names, NULL)) != NULL) {
									if(CFGetTypeID(kexts_info) == CFDictionaryGetTypeID() && CFDictionaryGetCount(kexts_info) == 1 && (kext_info = CFDictionaryGetValue(kexts_info, kext_name_cf)) != NULL && CFGetTypeID(kext_info) == CFDictionaryGetTypeID() && (kext_addr_cf = CFDictionaryGetValue(kext_info, CFSTR(kOSBundleLoadAddressKey))) != NULL && CFGetTypeID(kext_addr_cf) == CFNumberGetTypeID() && CFNumberGetValue(kext_addr_cf, kCFNumberSInt64Type, &kext_addr) && kext_addr_slid > kext_addr) {
										kslide = kext_addr_slid - kext_addr;
									}
									CFRelease(kexts_info);
								}
								CFRelease(kext_names);
							}
							CFRelease(kext_name_cf);
						}
					}
				}
				break;
			}
		}
	}
	if (!kslide) {
		return KERN_FAILURE;
	}
	return KERN_SUCCESS;
}


void kernel_getbase(uint64_t *retkbase, uint64_t *retkslide) {
	*retkbase = 0;
	*retkslide = kslide;
}

uint32_t kernel_read32(uint64_t where) {
    uint32_t out = 0;
    __kread_buf_tfp0(where, &out, sizeof(uint32_t));
    return out;
}

uint64_t kernel_read64(uint64_t where) {
    uint64_t out = 0;
    __kread_buf_tfp0(where, &out, sizeof(uint64_t));
    return out;
}

void kernel_write32(uint64_t where, uint32_t what) {
    uint32_t _what = what;
	__kwrite_buf_tfp0(where, (void *)&_what, sizeof(uint32_t));
}

void kernel_write64(uint64_t where, uint64_t what) {
    uint64_t _what = what;
	__kwrite_buf_tfp0(where, (void *)&_what, sizeof(uint64_t));
}

void kernel_write(uint64_t where, const void *buf, size_t size) {
	__kwrite_buf_tfp0(where, buf, size);
}

void kernel_read(uint64_t where, void *buf, size_t size) {
	__kread_buf_tfp0(where, buf, size);
}

int rw_prov_init() {
	kern_return_t ret = init_tfp0();
	if (ret != KERN_SUCCESS) {
		return ret;
	}
	ret = init_kslide();
	if (ret != KERN_SUCCESS) {
		return ret;
	}
	return ret;
}