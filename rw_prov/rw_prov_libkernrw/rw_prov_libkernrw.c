#include <mach/mach.h>
#include "libkernrw/libkernrw.h"

uint32_t kernel_read32(uint64_t where) {
    uint32_t out = 0;
    kernRW_read32(where, &out);
    return out;
}

uint64_t kernel_read64(uint64_t where) {
    uint64_t out = 0;
    kernRW_read64(where, &out);
    return out;
}

void kernel_write32(uint64_t where, uint32_t what) {
	kernRW_write32(where, what);
}

void kernel_write64(uint64_t where, uint64_t what) {
	kernRW_write64(where, what);
}

void kernel_write(uint64_t where, const void *buf, size_t size) {
	kernRW_writebuf(where, buf, size);
}

void kernel_read(uint64_t where, void *buf, size_t size) {
	kernRW_readbuf(where, buf, size);
}

void kernel_getbase(uint64_t *retkbase, uint64_t *retkslide) {
    uint64_t curKbase = 0;
    kernRW_getKernelBase(&curKbase);
	*retkbase = curKbase;
	*retkslide = 0;
}

int rw_prov_init() {
    return requestKernRw();
}