/* Copyright 2020 0x7ff
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "libdimentio.h"
#include "../rw_prov/rw_prov.h"
#include <compression.h>
#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "kern_context.h"

typedef uint64_t addr_t;
#define kaddr_t addr_t

#define PREBOOT_PATH "/private/preboot/"
#define BOOT_PATH "/System/Library/Caches/com.apple.kernelcaches/kernelcache"

#define LZSS_F (18)
#define LZSS_N (4096)
#define LZSS_THRESHOLD (2)
#define KCOMP_HDR_PAD_SZ (0x16C)

#define DER_INT (0x2U)
#define DER_SEQ (0x30U)
#define DER_IA5_STR (0x16U)
#define DER_OCTET_STR (0x4U)
#define RD(a) extract32(a, 0, 5)
#define RN(a) extract32(a, 5, 5)
#define KCOMP_HDR_MAGIC (0x636F6D70U)
#define ADRP_ADDR(a) ((a) & ~0xFFFULL)
#define ADRP_IMM(a) (ADR_IMM(a) << 12U)
#define IO_OBJECT_NULL ((io_object_t)0)
#define ADD_X_IMM(a) extract32(a, 10, 12)
#define kIODeviceTreePlane "IODeviceTree"
#define KCOMP_HDR_TYPE_LZSS (0x6C7A7373U)
#define LDR_X_IMM(a) (sextract64(a, 5, 19) << 2U)
#define IS_ADR(a) (((a) & 0x9F000000U) == 0x10000000U)
#define IS_ADRP(a) (((a) & 0x9F000000U) == 0x90000000U)
#define IS_LDR_X(a) (((a) & 0xFF000000U) == 0x58000000U)
#define IS_ADD_X(a) (((a) & 0xFFC00000U) == 0x91000000U)
#define LDR_W_UNSIGNED_IMM(a) (extract32(a, 10, 12) << 2U)
#define LDR_X_UNSIGNED_IMM(a) (extract32(a, 10, 12) << 3U)
#define IS_LDR_W_UNSIGNED_IMM(a) (((a) & 0xFFC00000U) == 0xB9400000U)
#define IS_LDR_X_UNSIGNED_IMM(a) (((a) & 0xFFC00000U) == 0xF9400000U)
#define ADR_IMM(a) ((sextract64(a, 5, 19) << 2U) | extract32(a, 29, 2))

#ifndef SECT_CSTRING
#	define SECT_CSTRING "__cstring"
#endif

#ifndef SEG_TEXT_EXEC
#	define SEG_TEXT_EXEC "__TEXT_EXEC"
#endif

typedef char io_string_t[512];
typedef mach_port_t io_object_t;
typedef uint32_t IOOptionBits, ipc_entry_num_t;
typedef io_object_t io_service_t, io_connect_t, io_registry_entry_t;

typedef struct {
	struct section_64 s64;
	const char *data;
} sec_64_t;

typedef struct {
	struct symtab_command cmd_symtab;
	sec_64_t sec_text, sec_cstring;
	kaddr_t base, kslide;
	const char *kernel;
	size_t kernel_sz;
	char *data;
} pfinder_t;

kern_return_t
IOServiceClose(io_connect_t);

kern_return_t
IOObjectRelease(io_object_t);

CFMutableDictionaryRef
IOServiceMatching(const char *);

CFDictionaryRef
OSKextCopyLoadedKextInfo(CFArrayRef, CFArrayRef);

io_registry_entry_t
IORegistryEntryFromPath(mach_port_t, const io_string_t);

io_service_t
IOServiceGetMatchingService(mach_port_t, CFDictionaryRef);

kern_return_t
IOServiceOpen(io_service_t, task_port_t, uint32_t, io_connect_t *);

kern_return_t
IORegistryEntrySetCFProperty(io_registry_entry_t, CFStringRef, CFTypeRef);

kern_return_t
mach_vm_write(vm_map_t, mach_vm_address_t, vm_offset_t, mach_msg_type_number_t);

kern_return_t
IOConnectCallStructMethod(io_connect_t, uint32_t, const void *, size_t, void *, size_t *);

CFTypeRef
IORegistryEntryCreateCFProperty(io_registry_entry_t, CFStringRef, CFAllocatorRef, IOOptionBits);

kern_return_t
mach_vm_read_overwrite(vm_map_t, mach_vm_address_t, mach_vm_size_t, mach_vm_address_t, mach_vm_size_t *);

kern_return_t
mach_vm_machine_attribute(vm_map_t, mach_vm_address_t, mach_vm_size_t, vm_machine_attribute_t, vm_machine_attribute_val_t *);

kern_return_t
mach_vm_region(vm_map_t, mach_vm_address_t *, mach_vm_size_t *, vm_region_flavor_t, vm_region_info_t, mach_msg_type_number_t *, mach_port_t *);

extern const mach_port_t kIOMasterPortDefault;


kern_return_t
kread_buf(kaddr_t addr, void *buf, mach_vm_size_t sz) {
	kernel_read(addr, buf, sz);
	return KERN_SUCCESS;
}

kern_return_t
kread_addr(kaddr_t addr, kaddr_t *val) {
	return kread_buf(addr, val, sizeof(*val));
}

kern_return_t
kwrite_buf(kaddr_t addr, const void *buf, mach_msg_type_number_t sz) {
	kernel_write(addr, buf, sz);
	return KERN_SUCCESS;
}

kaddr_t p_kbase = 0, p_kslide = 0, allproc = 0;
static kaddr_t kernproc;

static uint32_t
extract32(uint32_t val, unsigned start, unsigned len) {
	return (val >> start) & (~0U >> (32U - len));
}

static uint64_t
sextract64(uint64_t val, unsigned start, unsigned len) {
	return (uint64_t)((int64_t)(val << (64U - len - start)) >> (64U - len));
}

static size_t
decompress_lzss(const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len) {
	const uint8_t *src_end = src + src_len, *dst_start = dst, *dst_end = dst + dst_len;
	uint16_t i, r = LZSS_N - LZSS_F, flags = 0;
	uint8_t text_buf[LZSS_N + LZSS_F - 1], j;

	memset(text_buf, ' ', r);
	while(src != src_end && dst != dst_end) {
		if(((flags >>= 1U) & 0x100U) == 0) {
			flags = *src++ | 0xFF00U;
			if(src == src_end) {
				break;
			}
		}
		if((flags & 1U) != 0) {
			text_buf[r++] = *dst++ = *src++;
			r &= LZSS_N - 1U;
		} else {
			i = *src++;
			if(src == src_end) {
				break;
			}
			j = *src++;
			i |= (j & 0xF0U) << 4U;
			j = (j & 0xFU) + LZSS_THRESHOLD;
			do {
				*dst++ = text_buf[r++] = text_buf[i++ & (LZSS_N - 1U)];
				r &= LZSS_N - 1U;
			} while(j-- != 0 && dst != dst_end);
		}
	}
	return (size_t)(dst - dst_start);
}

static const uint8_t *
der_decode(uint8_t tag, const uint8_t *der, const uint8_t *der_end, size_t *out_len) {
	size_t der_len;

	if(der_end - der > 2 && tag == *der++) {
		if(((der_len = *der++) & 0x80U) != 0) {
			*out_len = 0;
			if((der_len &= 0x7FU) <= sizeof(*out_len) && (size_t)(der_end - der) >= der_len) {
				while(der_len-- != 0) {
					*out_len = (*out_len << 8U) | *der++;
				}
			}
		} else {
			*out_len = der_len;
		}
		if(*out_len != 0 && (size_t)(der_end - der) >= *out_len) {
			return der;
		}
	}
	return NULL;
}

static const uint8_t *
der_decode_seq(const uint8_t *der, const uint8_t *der_end, const uint8_t **seq_end) {
	size_t der_len;

	if((der = der_decode(DER_SEQ, der, der_end, &der_len)) != NULL) {
		*seq_end = der + der_len;
	}
	return der;
}

static const uint8_t *
der_decode_uint64(const uint8_t *der, const uint8_t *der_end, uint64_t *r) {
	size_t der_len;

	if((der = der_decode(DER_INT, der, der_end, &der_len)) != NULL && (*der & 0x80U) == 0 && (der_len <= sizeof(*r) || (--der_len == sizeof(*r) && *der++ == 0))) {
		*r = 0;
		while(der_len-- != 0) {
			*r = (*r << 8U) | *der++;
		}
		return der;
	}
	return NULL;
}

static void *
kdecompress(const void *src, size_t src_len, size_t *dst_len) {
	const uint8_t *der, *octet, *der_end, *src_end = (const uint8_t *)src + src_len;
	struct {
		uint32_t magic, type, adler32, uncomp_sz, comp_sz;
		uint8_t pad[KCOMP_HDR_PAD_SZ];
	} kcomp_hdr;
	size_t der_len;
	uint64_t r;
	void *dst;

	if((der = der_decode_seq(src, src_end, &der_end)) != NULL && (der = der_decode(DER_IA5_STR, der, der_end, &der_len)) != NULL && der_len == 4 && (memcmp(der, "IMG4", der_len) != 0 || ((der = der_decode_seq(der + der_len, src_end, &der_end)) != NULL && (der = der_decode(DER_IA5_STR, der, der_end, &der_len)) != NULL && der_len == 4)) && memcmp(der, "IM4P", der_len) == 0 && (der = der_decode(DER_IA5_STR, der + der_len, der_end, &der_len)) != NULL && der_len == 4 && memcmp(der, "krnl", der_len) == 0 && (der = der_decode(DER_IA5_STR, der + der_len, der_end, &der_len)) != NULL && (der = der_decode(DER_OCTET_STR, der + der_len, der_end, &der_len)) != NULL && der_len > sizeof(kcomp_hdr)) {
		octet = der;
		memcpy(&kcomp_hdr, octet, sizeof(kcomp_hdr));
		if(kcomp_hdr.magic == __builtin_bswap32(KCOMP_HDR_MAGIC)) {
			if(kcomp_hdr.type == __builtin_bswap32(KCOMP_HDR_TYPE_LZSS) && (kcomp_hdr.comp_sz = __builtin_bswap32(kcomp_hdr.comp_sz)) <= der_len - sizeof(kcomp_hdr) && (kcomp_hdr.uncomp_sz = __builtin_bswap32(kcomp_hdr.uncomp_sz)) != 0 && (dst = malloc(kcomp_hdr.uncomp_sz)) != NULL) {
				if(decompress_lzss(octet + sizeof(kcomp_hdr), kcomp_hdr.comp_sz, dst, kcomp_hdr.uncomp_sz) == kcomp_hdr.uncomp_sz) {
					*dst_len = kcomp_hdr.uncomp_sz;
					return dst;
				}
				free(dst);
			}
		} else if((der = der_decode_seq(der + der_len, src_end, &der_end)) != NULL && (der = der_decode_uint64(der, der_end, &r)) != NULL && r == 1 && der_decode_uint64(der, der_end, &r) != NULL && r != 0 && (dst = malloc(r)) != NULL) {
			if(compression_decode_buffer(dst, r, octet, der_len, NULL, COMPRESSION_LZFSE) == r) {
				*dst_len = r;
				return dst;
			}
			free(dst);
		}
	}
	return NULL;
}

static kern_return_t
find_section(const char *p, struct segment_command_64 sg64, const char *sect_name, struct section_64 *sp) {
	for(; sg64.nsects-- != 0; p += sizeof(*sp)) {
		memcpy(sp, p, sizeof(*sp));
		if((sp->flags & SECTION_TYPE) != S_ZEROFILL) {
			if(sp->offset < sg64.fileoff || sp->size > sg64.filesize || sp->offset - sg64.fileoff > sg64.filesize - sp->size) {
				break;
			}
			if(sp->size != 0 && strncmp(sp->segname, sg64.segname, sizeof(sp->segname)) == 0 && strncmp(sp->sectname, sect_name, sizeof(sp->sectname)) == 0) {
				return KERN_SUCCESS;
			}
		}
	}
	return KERN_FAILURE;
}

static void
sec_reset(sec_64_t *sec) {
	memset(&sec->s64, '\0', sizeof(sec->s64));
	sec->data = NULL;
}

static kern_return_t
sec_read_buf(sec_64_t sec, kaddr_t addr, void *buf, size_t sz) {
	size_t off;

	if(addr < sec.s64.addr || sz > sec.s64.size || (off = addr - sec.s64.addr) > sec.s64.size - sz) {
		return KERN_FAILURE;
	}
	memcpy(buf, sec.data + off, sz);
	return KERN_SUCCESS;
}

static void
pfinder_reset(pfinder_t *pfinder) {
	pfinder->base = 0;
	pfinder->kslide = 0;
	pfinder->data = NULL;
	pfinder->kernel = NULL;
	pfinder->kernel_sz = 0;
	sec_reset(&pfinder->sec_text);
	sec_reset(&pfinder->sec_cstring);
	memset(&pfinder->cmd_symtab, '\0', sizeof(pfinder->cmd_symtab));
}

static void
pfinder_term(pfinder_t *pfinder) {
	free(pfinder->data);
	pfinder_reset(pfinder);
}

static kern_return_t
pfinder_init_file(pfinder_t *pfinder, const char *filename) {
	struct symtab_command cmd_symtab;
	kern_return_t ret = KERN_FAILURE;
	struct segment_command_64 sg64;
	struct mach_header_64 mh64;
	struct load_command lc;
	struct section_64 s64;
	struct fat_header fh;
	struct stat stat_buf;
	struct fat_arch fa;
	const char *p, *e;
	size_t len;
	void *m;
	int fd;

	pfinder_reset(pfinder);
	if((fd = open(filename, O_RDONLY | O_CLOEXEC)) != -1) {
		if(fstat(fd, &stat_buf) != -1 && S_ISREG(stat_buf.st_mode) && stat_buf.st_size > 0) {
			len = (size_t)stat_buf.st_size;
			if((m = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0)) != MAP_FAILED) {
				if((pfinder->data = kdecompress(m, len, &pfinder->kernel_sz)) != NULL && pfinder->kernel_sz > sizeof(fh) + sizeof(mh64)) {
					pfinder->kernel = pfinder->data;
					memcpy(&fh, pfinder->kernel, sizeof(fh));
					if(fh.magic == __builtin_bswap32(FAT_MAGIC) && (fh.nfat_arch = __builtin_bswap32(fh.nfat_arch)) < (pfinder->kernel_sz - sizeof(fh)) / sizeof(fa)) {
						for(p = pfinder->kernel + sizeof(fh); fh.nfat_arch-- != 0; p += sizeof(fa)) {
							memcpy(&fa, p, sizeof(fa));
							if(fa.cputype == (cpu_type_t)__builtin_bswap32(CPU_TYPE_ARM64) && (fa.offset = __builtin_bswap32(fa.offset)) < pfinder->kernel_sz && (fa.size = __builtin_bswap32(fa.size)) <= pfinder->kernel_sz - fa.offset && fa.size > sizeof(mh64)) {
								pfinder->kernel_sz = fa.size;
								pfinder->kernel += fa.offset;
								break;
							}
						}
					}
					memcpy(&mh64, pfinder->kernel, sizeof(mh64));
					if(mh64.magic == MH_MAGIC_64 && mh64.cputype == CPU_TYPE_ARM64 && mh64.filetype == MH_EXECUTE && mh64.sizeofcmds < pfinder->kernel_sz - sizeof(mh64)) {
						for(p = pfinder->kernel + sizeof(mh64), e = p + mh64.sizeofcmds; mh64.ncmds-- != 0 && (size_t)(e - p) >= sizeof(lc); p += lc.cmdsize) {
							memcpy(&lc, p, sizeof(lc));
							if(lc.cmdsize < sizeof(lc) || (size_t)(e - p) < lc.cmdsize) {
								break;
							}
							if(lc.cmd == LC_SEGMENT_64) {
								if(lc.cmdsize < sizeof(sg64)) {
									break;
								}
								memcpy(&sg64, p, sizeof(sg64));
								if(sg64.vmsize == 0) {
									continue;
								}
								if(sg64.nsects != (lc.cmdsize - sizeof(sg64)) / sizeof(s64) || sg64.fileoff > pfinder->kernel_sz || sg64.filesize > pfinder->kernel_sz - sg64.fileoff) {
									break;
								}
								if(sg64.fileoff == 0 && sg64.filesize != 0) {
									if(pfinder->base != 0) {
										break;
									}
									pfinder->base = sg64.vmaddr;
									KERNINFRA_LOG(KERNLOG_INIT, "base: " KADDR_FMT "\n", sg64.vmaddr);
								}
								if(strncmp(sg64.segname, SEG_TEXT_EXEC, sizeof(sg64.segname)) == 0) {
									if(find_section(p + sizeof(sg64), sg64, SECT_TEXT, &s64) != KERN_SUCCESS) {
										break;
									}
									pfinder->sec_text.s64 = s64;
									pfinder->sec_text.data = pfinder->kernel + s64.offset;
									KERNINFRA_LOG(KERNLOG_INIT, "sec_text_addr: " KADDR_FMT ", sec_text_off: 0x%" PRIX32 ", sec_text_sz: 0x%" PRIX64 "\n", s64.addr, s64.offset, s64.size);
								} else if(strncmp(sg64.segname, SEG_TEXT, sizeof(sg64.segname)) == 0) {
									if(find_section(p + sizeof(sg64), sg64, SECT_CSTRING, &s64) != KERN_SUCCESS || pfinder->kernel[s64.offset + s64.size - 1] != '\0') {
										break;
									}
									pfinder->sec_cstring.s64 = s64;
									pfinder->sec_cstring.data = pfinder->kernel + s64.offset;
									KERNINFRA_LOG(KERNLOG_INIT, "sec_cstring_addr: " KADDR_FMT ", sec_cstring_off: 0x%" PRIX32 ", sec_cstring_sz: 0x%" PRIX64 "\n", s64.addr, s64.offset, s64.size);
								}
							} else if(lc.cmd == LC_SYMTAB) {
								if(lc.cmdsize != sizeof(cmd_symtab)) {
									break;
								}
								memcpy(&cmd_symtab, p, sizeof(cmd_symtab));
								KERNINFRA_LOG(KERNLOG_INIT, "cmd_symtab_symoff: 0x%" PRIX32 ", cmd_symtab_nsyms: 0x%" PRIX32 ", cmd_symtab_stroff: 0x%" PRIX32 "\n", cmd_symtab.symoff, cmd_symtab.nsyms, cmd_symtab.stroff);
								if(cmd_symtab.nsyms != 0 && (cmd_symtab.symoff > pfinder->kernel_sz || cmd_symtab.nsyms > (pfinder->kernel_sz - cmd_symtab.symoff) / sizeof(struct nlist_64) || cmd_symtab.stroff > pfinder->kernel_sz || cmd_symtab.strsize > pfinder->kernel_sz - cmd_symtab.stroff || cmd_symtab.strsize == 0 || pfinder->kernel[cmd_symtab.stroff + cmd_symtab.strsize - 1] != '\0')) {
									break;
								}
								pfinder->cmd_symtab = cmd_symtab;
							}
							if(pfinder->base != 0 && pfinder->sec_text.s64.size != 0 && pfinder->sec_cstring.s64.size != 0 && pfinder->cmd_symtab.cmdsize != 0) {
								ret = KERN_SUCCESS;
								break;
							}
						}
					}
				}
				munmap(m, len);
			}
		}
		close(fd);
	}
	if(ret != KERN_SUCCESS) {
		pfinder_term(pfinder);
	}
	return ret;
}

static kaddr_t
pfinder_xref_rd(pfinder_t pfinder, uint32_t rd, kaddr_t start, kaddr_t to) {
	kaddr_t x[32] = { 0 };
	uint32_t insn;

	for(; sec_read_buf(pfinder.sec_text, start, &insn, sizeof(insn)) == KERN_SUCCESS; start += sizeof(insn)) {
		if(IS_LDR_X(insn)) {
			x[RD(insn)] = start + LDR_X_IMM(insn);
		} else if(IS_ADR(insn)) {
			x[RD(insn)] = start + ADR_IMM(insn);
		} else if(IS_ADD_X(insn)) {
			x[RD(insn)] = x[RN(insn)] + ADD_X_IMM(insn);
		} else if(IS_LDR_W_UNSIGNED_IMM(insn)) {
			x[RD(insn)] = x[RN(insn)] + LDR_W_UNSIGNED_IMM(insn);
		} else if(IS_LDR_X_UNSIGNED_IMM(insn)) {
			x[RD(insn)] = x[RN(insn)] + LDR_X_UNSIGNED_IMM(insn);
		} else {
			if(IS_ADRP(insn)) {
				x[RD(insn)] = ADRP_ADDR(start) + ADRP_IMM(insn);
			}
			continue;
		}
		if(RD(insn) == rd) {
			if(to == 0) {
				if(x[rd] < pfinder.base) {
					break;
				}
				return x[rd];
			}
			if(x[rd] == to) {
				return start;
			}
		}
	}
	return 0;
}

static kaddr_t
pfinder_xref_str(pfinder_t pfinder, const char *str, uint32_t rd) {
	const char *p, *e;
	size_t len;

	for(p = pfinder.sec_cstring.data, e = p + pfinder.sec_cstring.s64.size; p != e; p += len) {
		len = strlen(p) + 1;
		if(strncmp(str, p, len) == 0) {
			return pfinder_xref_rd(pfinder, rd, pfinder.sec_text.s64.addr, pfinder.sec_cstring.s64.addr + (kaddr_t)(p - pfinder.sec_cstring.data));
		}
	}
	return 0;
}

static kaddr_t
pfinder_sym(pfinder_t pfinder, const char *sym) {
	const char *p, *strtab = pfinder.kernel + pfinder.cmd_symtab.stroff;
	struct nlist_64 nl64;

	for(p = pfinder.kernel + pfinder.cmd_symtab.symoff; pfinder.cmd_symtab.nsyms-- != 0; p += sizeof(nl64)) {
		memcpy(&nl64, p, sizeof(nl64));
		if(nl64.n_un.n_strx != 0 && nl64.n_un.n_strx < pfinder.cmd_symtab.strsize && (nl64.n_type & (N_STAB | N_TYPE)) == N_SECT && nl64.n_value >= pfinder.base && strcmp(strtab + nl64.n_un.n_strx, sym) == 0) {
			return nl64.n_value + pfinder.kslide;
		}
	}
	return 0;
}

static kaddr_t
pfinder_kernproc(pfinder_t pfinder) {
	kaddr_t ref = pfinder_sym(pfinder, "_kernproc");
	uint32_t insns[2];

	if(ref != 0) {
		return ref;
	}
	for(ref = pfinder_xref_str(pfinder, "\"Should never have an EVFILT_READ except for reg or fifo.\"", 0); sec_read_buf(pfinder.sec_text, ref, insns, sizeof(insns)) == KERN_SUCCESS; ref -= sizeof(*insns)) {
		if(IS_ADRP(insns[0]) && IS_LDR_X_UNSIGNED_IMM(insns[1]) && RD(insns[1]) == 3) {
			return pfinder_xref_rd(pfinder, RD(insns[1]), ref, 0);
		}
	}
	return 0;
}

static kaddr_t
pfinder_allproc(pfinder_t pfinder) {
	kaddr_t ref = pfinder_xref_str(pfinder, "shutdownwait", 2);

    if(ref == 0) {
        ref = pfinder_xref_str(pfinder, "shutdownwait", 3);                                                                                                                 /* msleep */
    }
    return pfinder_xref_rd(pfinder, 8, ref, 0);
}

static kaddr_t
pfinder_init_kbase(pfinder_t *pfinder) {
	struct mach_header_64 mh64;

	if(pfinder->base + pfinder->kslide > pfinder->base && kread_buf(pfinder->base + pfinder->kslide, &mh64, sizeof(mh64)) == KERN_SUCCESS && mh64.magic == MH_MAGIC_64 && mh64.cputype == CPU_TYPE_ARM64 && mh64.filetype == MH_EXECUTE) {
		pfinder->sec_text.s64.addr += pfinder->kslide;
		pfinder->sec_cstring.s64.addr += pfinder->kslide;
		KERNINFRA_LOG(KERNLOG_INIT, "kbase: " KADDR_FMT ", kslide: " KADDR_FMT "\n", pfinder->base + pfinder->kslide, pfinder->kslide);
		p_kbase = pfinder->base + pfinder->kslide;
		p_kslide = pfinder->kslide;
		return KERN_SUCCESS;
	}
	return KERN_FAILURE;
}

static char *
get_boot_path(void) {
	size_t hash_len, path_len = sizeof(BOOT_PATH);
	io_registry_entry_t chosen;
	struct stat stat_buf;
	const uint8_t *hash;
	CFDataRef hash_cf;
	char *path = NULL;

	if(stat(PREBOOT_PATH, &stat_buf) != -1 && S_ISDIR(stat_buf.st_mode) && (chosen = IORegistryEntryFromPath(kIOMasterPortDefault, kIODeviceTreePlane ":/chosen")) != IO_OBJECT_NULL) {
		if((hash_cf = IORegistryEntryCreateCFProperty(chosen, CFSTR("boot-manifest-hash"), kCFAllocatorDefault, kNilOptions)) != NULL) {
			if(CFGetTypeID(hash_cf) == CFDataGetTypeID() && (hash_len = (size_t)CFDataGetLength(hash_cf) << 1U) != 0) {
				path_len += strlen(PREBOOT_PATH) + hash_len;
				if((path = malloc(path_len)) != NULL) {
					memcpy(path, PREBOOT_PATH, strlen(PREBOOT_PATH));
					for(hash = CFDataGetBytePtr(hash_cf); hash_len-- != 0; ) {
						path[strlen(PREBOOT_PATH) + hash_len] = "0123456789ABCDEF"[(hash[hash_len >> 1U] >> ((~hash_len & 1U) << 2U)) & 0xFU];
					}
				}
			}
			CFRelease(hash_cf);
		}
		IOObjectRelease(chosen);
	}
	if(path == NULL) {
		path_len = sizeof(BOOT_PATH);
		path = malloc(path_len);
	}
	if(path != NULL) {
		memcpy(path + (path_len - sizeof(BOOT_PATH)), BOOT_PATH, sizeof(BOOT_PATH));
	}
	return path;
}

kern_return_t
pfinder_init_offsets(void) {
	kern_return_t ret = KERN_FAILURE;
	pfinder_t pfinder;
	char *boot_path;

	if((boot_path = get_boot_path()) != NULL) {
		KERNINFRA_LOG(KERNLOG_INIT, "boot_path: %s\n", boot_path);
		if(pfinder_init_file(&pfinder, boot_path) == KERN_SUCCESS) {
			//pfinder.kslide = kslide;
			uint64_t prov_kslide = 0, prov_kbase = 0;
			kernel_getbase(&prov_kbase, &prov_kslide);
			if (prov_kbase) {
				pfinder.kslide = prov_kbase - pfinder.base;
			} else if (prov_kslide) {
				pfinder.kslide = prov_kslide;
			} else {
				KERNINFRA_LOG(KERNLOG_INIT, "failed to retrive kslide!!!");
				return KERN_FAILURE;
			}
			
			KERNINFRA_LOG(KERNLOG_INIT, "got kslide: %llx\n", pfinder.kslide);
			if(pfinder_init_kbase(&pfinder) == KERN_SUCCESS && (kernproc = pfinder_kernproc(pfinder)) != 0) {
				KERNINFRA_LOG(KERNLOG_INIT, "kernproc: " KADDR_FMT "\n", kernproc);
				if((allproc = pfinder_allproc(pfinder)) != 0) {
					KERNINFRA_LOG(KERNLOG_INIT, "allproc: " KADDR_FMT "\n", allproc);
					ret = KERN_SUCCESS;
				}
			}
			pfinder_term(&pfinder);
		}
		free(boot_path);
	}
	return ret;
}

void
patchfinder_term(void) {
	setpriority(PRIO_PROCESS, 0, 0);
}

kern_return_t
patchfinder_init(void) {
	if(setpriority(PRIO_PROCESS, 0, PRIO_MIN) != -1 && pfinder_init_offsets() == KERN_SUCCESS) {
		return KERN_SUCCESS;
	}
	patchfinder_term();
	return KERN_FAILURE;
}