# Hacking and contributing to KernInfra

## Adding more offset / types

It's pretty simple to add types, just define it like this:
```
REMOTETYPE(proc_t,
    REMOTE_FIELD(kuint32_t, p_pid,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_12_0, 0x60)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_13_0, 0x68)
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0_b1, 0x68)
    );

    REMOTE_FIELD(uthread, p_uthlist,
        VERSION_OFF(kCFCoreFoundationVersionNumber_iOS_14_0_b1, 0x98)
    );
)
```
the VERSION_OFF is actually parsed to `if (version > XXX) offset = YYY;`, so in general you can directly add corresponding offset by inserting more VERSION_OFF line while keeping sorted.

