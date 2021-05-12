#include "rw_wrap/rw_wrap.hpp"

#define VERSION_OFF(curver, curoff) if (kCFCoreFoundationVersionNumber > curver) off = curoff;


REMOTETYPE(vnode_t,
    REMOTE_FIELD(kuint32_t, v_usecount,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_12_0, 0x60)
    );
    REMOTE_FIELD(kuint32_t, v_iocount,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_12_0, 0x64)
    );

    REMOTE_FIELD(mount_t, v_mount,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_12_0, 0xd8)
    );
    
    REMOTE_FIELD(kpointer_t, v_name,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0xb8)
    );
)

REMOTETYPE(mount_t,
    REMOTE_FIELD(kuint32_t, mnt_flag,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_12_0, 0x70)
    );
    REMOTE_FIELD(kuint32_t, mnt_kern_flag,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_12_0, 0x74)
    );

    REMOTE_FIELD(vnode_t, mnt_devvp,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x980)
    );
)

REMOTETYPE(fileglob,
    REMOTE_FIELD(vnode_t, fg_data,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x38)
    );
)


REMOTETYPE(fileproc,
    REMOTE_FIELD(fileglob, f_fglob,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x10)
    );
)

REMOTETYPE(filedesc,
    REMOTE_FIELD(kpointer_t, fd_ofiles,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0)
    );
    REMOTE_FIELD(vnode_t, fd_rdir,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_12_0, 0x38)
    );
    REMOTE_FIELD(vnode_t, fd_cdir,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_12_0, 0x40)
    );
)

REMOTETYPE(_vm_map_t,
    REMOTE_FIELD(kuint32_t, page_shift,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x40)
    );
)


REMOTETYPE(_thread_t,
    REMOTE_FIELD(_vm_map_t, _map,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x348)
    );
)
REMOTETYPE(_task_t,
    REMOTE_FIELD(_vm_map_t, _map,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x28)
    );
)

REMOTETYPE(uthread,
    REMOTE_FIELD(_thread_t, uu_thread,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x120)
    );
)

REMOTETYPE(proc_t,
    REMOTE_FIELD(proc_t, nextproc,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_12_0, 0)
    );

    REMOTE_FIELD(_task_t, task,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x10)
    );

    REMOTE_FIELD(kuint32_t, p_pid,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_12_0, 0x60)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_0, 0x68)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0_b1, 0x68)
    );

    REMOTE_FIELD(uthread, p_uthlist,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0_b1, 0x98)
    );

    REMOTE_FIELD(filedesc, p_fd,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_12_0, 0x100)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_0, 0x108)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0_b1, 0xf8)
    );
)

