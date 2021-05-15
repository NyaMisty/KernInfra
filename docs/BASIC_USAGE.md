# Basic usage of KernInfra

Lets begin with a sample code from kern_func.cpp:

```cpp
addr_t proc_of_pid(pid_t pid) {
    addr_t procAddr = kernel_read64(allproc);
    uint64_t current_pid = 0;

    while (procAddr) {
        auto proc = proc_t_p(procAddr);
        current_pid = proc.p_pid().load();
        if (current_pid == pid) return procAddr;
        procAddr = proc.nextproc().load();
        printf("proc_of_pid: %llx %llu\n", procAddr, current_pid);
    }
    return 0;
}
```

## 1. initialize a remote object

To begin with, we'll need to create an object like this: `auto proc = proc_t_p(procAddr)`

With this statement, we created a proc object, which represents a proc_t object located at procAddr in kernel.
(Note: The proc_t_p here is a short hand for proc_t<>)

## 2. access the field of a remote object

With this object, you can access the remote fields using something like proc.p_pid().
This will create a new object represents a pid located at proc's p_pid offset. But as you can feel from my explaination, the proc.p_pid() call is NOT actually reading the p_pid field. Instead, it's more like a &proc->p_pid, which just store a offset from the parent pointer.

The reason to do this, is that in this way we can build a reference to a field, which then enable us to do various things like chaining these field access.

So to actually reading the pid, you would have to do proc.p_pid().load(), and correspondingly, use proc.p_pid().store(12345) to store a new value.

## 3. chaining

Often you have to access several levels of structs, but only to retrive a single field. So in these cases, you can chain these operations, like `proc.p_fd().fd_ofiles().load()`. 

This will goes into the deepest level of field, and then retrive the value. The actual kernel R/W only happens at the last .load() call, (i.e. all previous calls are building references based on pointers and offsets.).

## 4. caching

Kernel R/W is really expensive operation, so we can't simply chaining all the time. In KernInfra, each object has its pointer cache. Once an object has been used, it will save the pointer value retrived from parent struct's field, so next time it will not query the long chain again.

Example:
```cpp
    auto curp = proc_t_p(current_proc());
    auto vPageShift = curp.task()._map().page_shift();
    DLOG("original page shift: %d", vPageShift.load());
    vPageShift.store(12);
    DLOG("new page shift: %d", vPageShift.load());
```

In this examle we need to use page_shift multiple time, so we saved the object representing pageShift, and then use it multiple times.

On the first time (being loaded in DLOG), KernInfra will resolve vPageShift's actual address in kernel memory, and then load the value.
Then in the later call, KernInfra will directly use the resolved address to perform R/W operations.

## 5. address checking and PAC handling

When doing kernel research, one of the most frustrating thing is panicking devices due to accidentally operating on a NULL or userspace pointer, or forgetting to remove the PAC pointer. The only thing we got is blaming ourselves and wasting time on re-jailbreaking. 

With KernInfra, this will no longer be the problem. On each kernel read/write, KernInfra will check if the address looks like a kernel address (not Null, top 25 bit is totally 1), on invalid access, it will throw an exception telling which address is invalid,

For PAC, in iOS14, quite a few data structures are protected by a so-called technique "data-PAC", which is encrypting some crucial kernel pointer in the struct. As the AAPL intentionally deleted all PAC-related information in kernel, it's laborious to check if a pointer is PAC-ed and removetp the PAC tag accordingly.

With KernInfra, we offer you a special load_addr method. Using this method, KernInfra will automatically check the address, and return you an unPACed pointer. If you want to get the original pointer value, just use the original load() method.

## 6. offsets

KernInfra will also manages struct's offsets for you. In kernstructs.hpp, you can see the field definitions of a remote struct. The offset is automatically setup by comparing kCFCoreFoundationVersionNumber.

If you want to add missing offsets, see HACKING.md, and welcome to submit a PR if you are sure about the offset,

