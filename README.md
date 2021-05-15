# KernInfra

KernInfra is a developer-friendly kernel read-write framework.

## Why KernInfra

KernInfra is built to address the following engineering issues:
- kernel chain RW problem: for operations like `vnode_from_fd = proc_t->p_fd->fd_ofiles[0]->f_fglob->fg_data`
- PAC pointer handling: tons of kxpacd calls
- kernel read address checking: no more ~~Kernel Data Abort~~
- kernel offset management: no more `off_XXX_YYY`
- multiple kern R/W provider: it's 2021, bye-bye tfp0, and hello libkrw/libkernrw
- various simple but useful kernel helper functions: we need patchfinder to find proc, vnodes, tasks, etc.

## How does it look like

- Before KernInfra: R/W `vnode->v_mount->mnt_flag`
```cpp
    printf("got vp: %llx\n", orig);
    uint64_t mount = kernel_read64(orig + off_v_mount);
    uint64_t kxpacd_mount = kxpacd(mount);
    printf("  %s: mount %llx\n", path, kxpacd_mount);
    uint32_t oriflag = kernel_read32(kxpacd_mount + off_mnt_flag);
    printf("  %s: oriflag %x\n", path, oriflag);
    kernel_write32(kxpacd_mount + off_mnt_flag, newflag);
```

- After KernInfra: RW `proc->task->map->page_shift` ( also compatible with Intellisense ;) )
```cpp
    auto curp = proc_t_p(current_proc());
    auto vPageShift = curp.task()._map().page_shift();
    DLOG("original page shift: %d", vPageShift.load());
    vPageShift.store(12);
    DLOG("new page shift: %d", vPageShift.load());
```

## Installation & Usage

1. `git submodule add https://github.com/NyaMisty/kerninfra && git submodule update --init --recursive`
2. insert these lines into theos makefile
    ```Makefile
    XXXX_SUBPROJECTS = kerninfra
    XXXX_LDFLAGS += -Lkerninfra/libs
    XXXX_CCFLAGS += -std=c++2a
    ```
3. include it: `#include "kerninfra/kerninfra.hpp"`
4. call init func: 
    ```cpp
    if (!!init_kerninfra()) {
        fprintf(stderr, "Failed to init kerninfra!!\n");
        exit(1);
    } else {
        DLOG("successfully initialized kerninfra!");
    }
    ```
5. profit~, see my fouldecrypt for a simple example, documents are located in docs/ directory ;)

## Contribute

- You can add more offset into our code, it will benefit EVERYONE. (And maybe we can steal some offset from Odyssey :P)
- You can implement more kernel helper functions
- see more in docs/HACKING.md

## Credits
- includes/fmt: it's fmt.dev's fmt, thanks a lot
- libkrw: thanks @Siguza
- libdimento: thanks @0x7ff (original dev) & @ichitaso (contributor)
