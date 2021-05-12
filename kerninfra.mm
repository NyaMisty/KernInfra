#ifdef __cplusplus
extern "C" {
#endif

#include "rw_prov/rw_prov.h"
#include "patchfinder/libdimentio.h"

#ifdef __cplusplus
}
#endif

int init_kerninfra() {
    if (rw_prov_init() != 0) {
        printf("failed rw provider's init!\n");
        return 1;
    }
    if (patchfinder_init() != KERN_SUCCESS) {
        printf("failed patchfinder dimentio's init!\n");
        return 1;
    }
    return 0;
}