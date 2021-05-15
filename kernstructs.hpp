#include "rw_wrap/rw_wrap.hpp"

#define VERSION_OFF(curver, curoff) if (kCFCoreFoundationVersionNumber > curver) off = curoff;


REMOTETYPE(ubc_info, 
    REMOTE_FIELD(kpointer_t, cs_blobs,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x50)
    );
)

REMOTETYPE(vnode_t,
    REMOTE_FIELD(kuint32_t, v_usecount,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_12_0, 0x60)
    );
    REMOTE_FIELD(kuint32_t, v_iocount,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_12_0, 0x64)
    );

    REMOTE_FIELD(kuint16_t, v_type,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x70)
    );

    REMOTE_FIELD(kuint16_t, v_tag,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x72)
    );
    
    REMOTE_FIELD(kuint16_t, v_id,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x74)
    );

    REMOTE_FIELD(kpointer_t, v_un,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x78)
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

REMOTETYPE(apple_protect_pager_t, // extends from memory_object_t
    REMOTE_FIELD(kpointer_t, backing_object,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x38)
    );
    REMOTE_FIELD(kuint64_t, backing_offset,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x40)
    );
    REMOTE_FIELD(kuint64_t, crypto_backing_offset,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x48)
    );
    REMOTE_FIELD(kuint64_t, crypto_start,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x50)
    );
    REMOTE_FIELD(kuint64_t, crypto_end,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x58)
    );
    REMOTE_FIELD(kpointer_t, crypt_info,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x60)
    );
    
)

REMOTETYPE(_memory_object_t,
    REMOTE_FIELD(kpointer_t, mo_pager_ops,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x8)
    );
)

REMOTETYPE(vm_object_t,
    REMOTE_FIELD(vm_object_t, shadow,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x40)
    );
    REMOTE_FIELD(_memory_object_t, pager,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x48)
    );
)

REMOTETYPE(_vm_map_entry,
    REMOTE_FIELD(_vm_map_entry, link_prev,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x0)
    );
    REMOTE_FIELD(_vm_map_entry, link_next,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x8)
    );
    REMOTE_FIELD(kuint64_t, start,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x10)
    );
    REMOTE_FIELD(kuint64_t, end,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x18)
    );
    REMOTE_FIELD(vm_object_t, vme_object,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x38)
    );
    REMOTE_FIELD(kuint64_t, vme_offset,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x40)
    );
    REMOTE_FIELD(kuint32_t, vme_flags,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x48)
    );
)

REMOTETYPE(_vm_map_t,
    REMOTE_FIELD(_vm_map_entry, link_prev,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x10)
    );
    REMOTE_FIELD(_vm_map_entry, link_next,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x18)
    );
    REMOTE_FIELD(kuint32_t, nentries,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x30)
    );
    REMOTE_FIELD(kuint32_t, page_shift,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x40)
    );
    REMOTE_FIELD(kpointer_t, pmap,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x48)
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

