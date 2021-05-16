#include <stdint.h>
#include <unistd.h>
#include <stdexcept>
#include <fmt/format.h>
#include "kerninfra.hpp"

#define FLOG(...) KERNINFRA_LOG(KERNLOG_FUNC, __VA_ARGS__)

addr_t proc_of_pid(pid_t pid) {
    addr_t procAddr = kernel_read64(allproc);
    uint64_t current_pid = 0;

    while (procAddr) {
        auto proc = proc_t_p(procAddr);
        current_pid = proc.p_pid().load();
        if (current_pid == pid) return procAddr;
        procAddr = proc.nextproc().load();
        FLOG("proc_of_pid: proc %llx pid %llu", procAddr, current_pid);
    }
    throw std::runtime_error(fmt::format("proc_of_pid failed to find proc for {}", pid));
}

addr_t current_proc() {
    addr_t proc = proc_of_pid(getpid());
    //printf("proc: %llx\n", proc);
    FLOG("current_proc: found proc %llx", proc);
    return proc;
}


addr_t vnode_from_fd(int fd) {
    auto proc = proc_t_p(current_proc());
    addr_t ofiles = proc.p_fd().fd_ofiles().load();
    FLOG("vnode_from_fd: got ofiles array %llx", ofiles);
    auto fp = fileproc_p(kpointer_t<>(ofiles + 8 * fd).load_addr());
    return fp.f_fglob().fg_data().load();
}

addr_t lookup_vm_map_entry(addr_t _vmmap, addr_t va) {
    auto vmmap = _vm_map_t_p(_vmmap);
    int n = vmmap.nentries().load();
    FLOG("lookup_vm_map_entry: Totally %d vm entries", n);
    addr_t curEntry = vmmap.link_next().load();
    for (int i = 0; i < n; i++) {
        auto _curEntry = _vm_map_entry_p(curEntry);
        FLOG("lookup_vm_map_entry: VM Entry: %p - %p", (void *)_curEntry.start().load(), (void *)_curEntry.end().load());
        if (va >= _curEntry.start().load() && va < _curEntry.end().load()) {
            return curEntry;
        }
        curEntry = _curEntry.link_next().load();
    }
    return 0;
}