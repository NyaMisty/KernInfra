#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdarg.h>

typedef uint64_t addr_t;

typedef enum {
    KERNLOG_NONE = 0,
    KERNLOG_FUNC,
    KERNLOG_INIT,
    KERNLOG_KERNRW,
} RWWrapLogLevel;

typedef struct {
    void (*remote_reader)(addr_t addr, void *buf, size_t size);
    void (*remote_writer)(addr_t addr, const void *buf, size_t size);
    addr_t (*addr_reader)(addr_t addr);
    void (*addr_writer)(addr_t addr, addr_t val);
    addr_t (*addr_processor)(addr_t addr);
    void (*vDoLog)(const char * format, va_list args);
    RWWrapLogLevel logLevel;
} KernInfraContext;

extern KernInfraContext kerninfra_context;

extern void kerninfra_log(int ll, const char * format, ...);

#ifdef __cplusplus
}
#endif

#define KERNINFRA_LOG(ll, ...) \
    do {if (ll <= kerninfra_context.logLevel) kerninfra_log(ll, __VA_ARGS__); } while(0);