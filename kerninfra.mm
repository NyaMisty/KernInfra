#include "kerninfra.hpp"

#include <string>

void kernVPrintf(const char *fmtstr, va_list args) {
    std::string fmtstr_ = fmtstr;
    fmtstr_ += "\n";
    vprintf(fmtstr_.c_str(), args);
}

KernInfraContext kerninfra_context = {
    .vDoLog = (decltype(kerninfra_context.vDoLog))kernVPrintf,
    .logLevel = KERNLOG_NONE,
};

void kerninfra_log(int ll, const char * format, ...) {
    if (ll <= kerninfra_context.logLevel) {
        va_list ap;
        va_start(ap, format);
        kerninfra_context.vDoLog(format, ap);
        va_end(ap);
    }
}

//int init_kerninfra(int logLevel, void (*vDoLog)(const char *, va_list args) = (decltype(vDoLog))vprintf) {
int init_kerninfra(int logLevel, void (*vDoLog)(const char *, va_list args)) {
    kerninfra_context.logLevel = (decltype(kerninfra_context.logLevel))logLevel;
    if (!!vDoLog) {
        kerninfra_context.vDoLog = vDoLog;
    }
    
    if (rw_prov_init() != 0) {
        printf("failed rw provider's init!\n");
        return 1;
    }
    if (patchfinder_init() != KERN_SUCCESS) {
        printf("failed patchfinder dimentio's init!\n");
        return 1;
    }
    prepare_rw_wrap(&kerninfra_context);

    return 0;
}