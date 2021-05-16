#include "../rw_prov/rw_prov.h"

#include <kern_context.h>
#include "rw_wrap.hpp"
#include <exception>
#include <stdexcept>
#define FMT_HEADER_ONLY
#include <fmt/format.h>
//#include <format>

#define MIN_KERNEL_ADDRESS 0xffffff0000000000
#define MAX_KERNEL_ADDRESS 0xfffffffff0000000

void check_addr_valid(addr_t addr, size_t size) {
    //fmt::print("kernel access: {:#x}\n", addr);
    if (addr < MIN_KERNEL_ADDRESS || addr + size < MIN_KERNEL_ADDRESS || addr + size > MAX_KERNEL_ADDRESS) {
        throw std::out_of_range(fmt::format("kernel read out-of-range: {:#x}", addr));
    }
}

constexpr int t1sz_boot = 25;
constexpr addr_t PACMASK = ~((1ULL << (64U - t1sz_boot)) - 1U);
uint64_t kxpacd(uint64_t pacPtr) {
    if(t1sz_boot != 0) {
		pacPtr |= PACMASK;
	}
    return pacPtr;
}


addr_t remote_addr_processor_impl(addr_t addr) {
    if (!addr) {
        return addr;
    }
    if (!(addr & PACMASK)) {
        throw std::out_of_range(fmt::format("kernel got out-of-range pointer: {:#x}", addr));
    }
    return kxpacd(addr);
}

addr_t remote_addr_reader_impl(addr_t addr) {
    check_addr_valid(addr, sizeof(addr_t));
    addr_t ret = kernel_read64(addr);
    return remote_addr_processor_impl(ret);
}

void remote_addr_writer_impl(addr_t addr, addr_t val) {
    check_addr_valid(addr, sizeof(addr_t));
    //addr_t pacaddr = kxpacd(addr);
    //kernel_write64(pacaddr, val);
    kernel_write64(addr, val);
}

void remote_reader_impl(addr_t addr, void *buf, size_t size) {
    check_addr_valid(addr, size);
    kernel_read(addr, buf, size);
}

void remote_writer_impl(addr_t addr, const void *buf, size_t size) {
    check_addr_valid(addr, size);
    kernel_write(addr, buf, size);
}

void prepare_rw_wrap(KernInfraContext *kerninfra_context) {
    kerninfra_context->remote_reader = remote_reader_impl,
    kerninfra_context->remote_writer = remote_writer_impl,
    kerninfra_context->addr_reader = remote_addr_reader_impl,
    kerninfra_context->addr_writer = remote_addr_writer_impl,
    kerninfra_context->addr_processor = remote_addr_processor_impl,
    0;
    return;
}
