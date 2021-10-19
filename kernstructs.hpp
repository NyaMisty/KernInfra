#include "rw_wrap/rw_wrap.hpp"

#define VERSION_OFF(curver, curoff) if (kCFCoreFoundationVersionNumber >= curver) off = curoff;


REMOTETYPE(ubc_info, 
    REMOTE_FIELD(kpointer_t, cs_blobs,      // can be found in ubc_cs_blob_get, which is usually the next function of _csvnode_print_debug (identifiable by "(unknown vnode name)")
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_5, 0x50)
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

    REMOTE_FIELD(kuint16_t, v_type,         // can be found in ubc_cs_blob_get
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_4, 0x70)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_5, 0x70)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x70)
    );

    REMOTE_FIELD(kuint16_t, v_tag,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x72)
    );
    
    REMOTE_FIELD(kuint16_t, v_id,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x74)
    );

    REMOTE_FIELD(kpointer_t, v_un,          // can be found in ubc_cs_blob_get
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_4, 0x78)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_5, 0x78)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x78)
    );

    REMOTE_FIELD(mount_t, v_mount,          // pretty easy
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_12_0, 0xd8)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_5, 0xd8)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0xd8)
    );
    
    REMOTE_FIELD(kpointer_t, v_name,        // (identifiable by "(unknown vnode name)")
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_4, 0xb8)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_5, 0xb8)
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
    // all offsets can be found in apple_protect_pager_create, which is identifiable using apple_protect_pager_ops (which is a method table that can be easily found with "apple_protect_pager" string)
    REMOTE_FIELD(kpointer_t, backing_object,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_4, 0x38)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_5, 0x38)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x38)
    );
    REMOTE_FIELD(kuint64_t, backing_offset,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_4, 0x40)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_5, 0x40)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x40)
    );
    REMOTE_FIELD(kuint64_t, crypto_backing_offset,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_4, 0x48)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_5, 0x48)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x48)
    );
    REMOTE_FIELD(kuint64_t, crypto_start,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_4, 0x50)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_5, 0x50)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x50)
    );
    REMOTE_FIELD(kuint64_t, crypto_end,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_4, 0x58)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_5, 0x58)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x58)
    );
    REMOTE_FIELD(kpointer_t, crypt_info,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_4, 0x60)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_5, 0x60)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x60)
    );
    
)

REMOTETYPE(_memory_object_t,
    REMOTE_FIELD(kpointer_t, mo_pager_ops, // can be found in apple_protect_pager_create
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_4, 0x8)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_5, 0x8)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x8)
    );
)

REMOTETYPE(vm_object_t,
    REMOTE_FIELD(vm_object_t, shadow,       // can be found in vm_object_shadow (identified by panic string)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_4, 0x40)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_5, 0x40)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x40)
    );
    REMOTE_FIELD(_memory_object_t, pager,   // can be found in functions with reference to vnode_pager_ops (usually doing object->pager->mo_pager_ops))
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_4, 0x48)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_5, 0x48)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x48)
    );
)

REMOTETYPE(_vm_map_entry,
    // all offsets can be found in vm_map_entry_insert (identifiable via reference to vm_map_entry_create, usually the first ref to it)
    REMOTE_FIELD(_vm_map_entry, link_prev,      // infered from struct definition
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_12_0, 0x0)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x0)
    );
    REMOTE_FIELD(_vm_map_entry, link_next,      // infered from struct definition
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_12_0, 0x8)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x8)
    );
    REMOTE_FIELD(kuint64_t, start,              // infered from struct definition
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_12_0, 0x10)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x10)
    );
    REMOTE_FIELD(kuint64_t, end,                // infered from struct definition
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_12_0, 0x18)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x18)
    );
    REMOTE_FIELD(vm_object_t, vme_object,       // can be found in kmem_realloc (identifiable with panic string)       
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_4, 0x38)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_5, 0x38)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x38)
    );
    REMOTE_FIELD(kuint64_t, vme_offset,         // can be found in kmem_alloc's VME_OFFSET_SET pattern
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_4, 0x40)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_5, 0x40)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x40)
    );
    REMOTE_FIELD(kuint32_t, vme_flags,          // inferred from struct definition
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_4, 0x48)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_5, 0x48)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x48)
    );
)

REMOTETYPE(_vm_map_t,
    // all offsets can be found in vm_map_create_options
    REMOTE_FIELD(_vm_map_entry, link_prev,      // infered from struct definition
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_12_0, 0x10)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x10)
    );
    REMOTE_FIELD(_vm_map_entry, link_next,      // infered from struct definition
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_12_0, 0x18)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x18)
    );
    REMOTE_FIELD(kuint32_t, nentries,           // can be found in vm_map_create_options (identifiable using panic string)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_4, 0x30)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_5, 0x30)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x30)
    );
    REMOTE_FIELD(kuint32_t, page_shift,         // can be found in vm_map_create_options (identifiable using panic string)
                                                // can also be cross-validated with vm_map_page_mask() (also presents in mremap_encrypted)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_4, 0x40)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_5, 0x40)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x40)
    );
    REMOTE_FIELD(kpointer_t, pmap,              // can be found in vm_map_create_options (identifiable using panic string)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_4, 0x48)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_5, 0x48)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x48)
    );
)


REMOTETYPE(_thread_t,
    REMOTE_FIELD(_vm_map_t, _map,               // can be found via current_map() call (for example from mremap_encrypted())
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_4, 0x338)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_5, 0x320)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x348)
    );
)
REMOTETYPE(_task_t,
    REMOTE_FIELD(_vm_map_t, _map,               // can be found in task_init() (identifiable with panic string)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_4, 0x28)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_5, 0x28)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x28)
    );
)

REMOTETYPE(uthread,
    REMOTE_FIELD(_thread_t, uu_thread,          // can't be easily found without symbol...
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x120)
    );
)

REMOTETYPE(proc_t,
    REMOTE_FIELD(proc_t, nextproc,              // from struct definition
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_12_0, 0)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0)
    );

    REMOTE_FIELD(_task_t, task,                 // from struct definition
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_12_0, 0x10)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0, 0x10)
    );

    REMOTE_FIELD(kuint32_t, p_pid,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_12_0, 0x60)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_0, 0x68)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0_b1, 0x68)
    );

    REMOTE_FIELD(uthread, p_uthlist,            // can be found in bsd_init
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_4, 0x98)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_5, 0x98)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0_b1, 0x98)
    );

    REMOTE_FIELD(kpointer_t, p_ucred,            // can be found in bsd_init
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_4, 0x100)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_5, 0x100)
    );

    REMOTE_FIELD(filedesc, p_fd,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_12_0, 0x100)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_0, 0x108)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0_b1, 0xf8)
    );
)

