uint64_t proc_of_pid(pid_t pid);
uint64_t current_proc();

addr_t vnode_from_fd(int fd);

addr_t lookup_vm_map_entry(addr_t _vmmap, addr_t va);