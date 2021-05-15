#include <stdint.h>
#include <unistd.h>
#include "kerninfra.hpp"

addr_t proc_of_pid(pid_t pid) {
    addr_t procAddr = kernel_read64(allproc);
    uint64_t current_pid = 0;

    while (procAddr) {
        auto proc = proc_t_p(procAddr);
        current_pid = proc.p_pid().load();
        if (current_pid == pid) return procAddr;
        procAddr = proc.nextproc().load();
        //printf("proc_of_pid: %llx %llu\n", procAddr, current_pid);
    }
    return 0;
}

addr_t current_proc() {
    addr_t proc = proc_of_pid(getpid());
    //printf("proc: %llx\n", proc);
    return proc;
}


addr_t vnode_from_fd(int fd) {
    auto proc = proc_t_p(current_proc());
    addr_t ofiles = proc.p_fd().fd_ofiles().load();
    auto fp = fileproc_p(addr_reader(ofiles + 8 * fd));
    return fp.f_fglob().fg_data().load();
}

addr_t lookup_vm_map_entry(addr_t _vmmap, addr_t va) {
    auto vmmap = _vm_map_t_p(_vmmap);
    int n = vmmap.nentries().load();
    //printf("Totally %d vm entries\n", n);
    addr_t curEntry = vmmap.link_next().load();
    for (int i = 0; i < n; i++) {
        auto _curEntry = _vm_map_entry_p(curEntry);
        //printf("    VM Entry: %p - %p\n", (void *)_curEntry.start().load(), (void *)_curEntry.end().load());
        if (va >= _curEntry.start().load() && va < _curEntry.end().load()) {
            return curEntry;
        }
        curEntry = _curEntry.link_next().load();
    }
    return 0;
}