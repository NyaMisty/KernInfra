// tester for rw-wrap
// compile: clang++ -Iincludes kernstructtest.cpp
#include "sysvers.h"
extern double kCFCoreFoundationVersionNumber;

#include "kernstructs.hpp"

double kCFCoreFoundationVersionNumber = 123456.0;

addr_t remote_addr_reader_impl(addr_t addr) {
    printf("kread_addr(0x%llx)\n", addr);
    return (addr & ~0xffff) + 0x10000;
}

void remote_addr_writer_impl(addr_t addr, addr_t val) {
    printf("kwrite_addr(0x%llx)\n", addr);
}

void remote_reader_impl(addr_t addr, void *buf, size_t size) {
    printf("kread(0x%llx, 0x%zx)\n", addr, size);
}

void remote_writer_impl(addr_t addr, const void *buf, size_t size) {
    printf("kwrite(0x%llx, 0x%zx)\n", addr, size);
}

void (*remote_reader)(addr_t addr, void *buf, size_t size) = remote_reader_impl;
void (*remote_writer)(addr_t addr, const void *buf, size_t size) = remote_writer_impl;
addr_t (*addr_reader)(addr_t addr) = remote_addr_reader_impl;
void (*addr_writer)(addr_t addr, addr_t val) = remote_addr_writer_impl;

int main() {
    auto p = proc_t_p(0x2000);
    auto np = p.nextproc().load();
    printf("type(np): %s, np: %llx\n", typeid(np).name(), np);
    p.p_fd().fd_cdir().v_usecount().load();
}