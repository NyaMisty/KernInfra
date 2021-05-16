#include <unistd.h>
#include "includes/sysvers.h"

#include "includes/kern_context.h"

//#include "rw_prov/rw_prov.h"
//#include "patchfinder/libdimentio.h"
extern "C" {

#include "rw_prov/rw_prov.h"
#include "patchfinder/libdimentio.h"

}


#include "kernstructs.hpp"
#include "kern_func.hpp"

int init_kerninfra(int logLevel, void (*vDoLog)(const char *, va_list args) = NULL);