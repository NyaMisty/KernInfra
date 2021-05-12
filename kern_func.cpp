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