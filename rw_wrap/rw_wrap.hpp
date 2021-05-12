#include <stdio.h>
#include <stdint.h>
#include <type_traits>
#include <typeinfo>
#include <exception>
#include <stdexcept>

typedef uint64_t addr_t;


#define BADADDR (addr_t)(-1)

extern void (*remote_reader)(addr_t addr, void *buf, size_t size);
extern void (*remote_writer)(addr_t addr, const void *buf, size_t size);
extern addr_t (*addr_reader)(addr_t addr);
extern void (*addr_writer)(addr_t addr, addr_t val);

template<class P, bool isField>
struct RemotePointer {};

template<class P>
struct RemotePointer<P, true> {
    addr_t m_pointer;
    P &m_parent;
    addr_t m_off;
    
    RemotePointer(P &parent, addr_t off) : m_pointer(BADADDR), m_parent(parent), m_off(off) {}

    addr_t addr() {
        if (m_pointer == BADADDR) {
            //printf("kread(parent.addr() + m_off)\n");
            //m_pointer = addr_reader(m_parent.addr() + m_off);
            m_pointer = m_parent.load() + m_off;
            return m_pointer;
        } else {
            return m_pointer;
        }
    }

    addr_t load() {
        return addr_reader(addr());
    }

    void store(addr_t val) {
        addr_writer(addr(), val);
    }
};

template<class P>
struct RemotePointer<P, false> {
    addr_t m_pointer;
    
    RemotePointer(addr_t pointer) : m_pointer(pointer) {}

    addr_t addr() {
        return m_pointer;
    }

    addr_t load() {
        return addr();
    }
};


template<class T, class P, bool isField>
struct RemoteSimpleType : public RemotePointer<P, isField> {
    using RemotePointer<P, isField>::RemotePointer;
    using RemotePointer<P, isField>::addr;
    T load() {
        auto ret = T();
        remote_reader(addr(), &ret, sizeof(ret));
        return ret;
    }

    void store(T val) {
        remote_writer(addr(), &val, sizeof(val));
    }
};

template<class P = void, bool isField = false>
using kuint32_t = RemoteSimpleType<uint32_t, P, isField>;

template<class P = void, bool isField = false>
using kuint64_t = RemoteSimpleType<uint64_t, P, isField>;

template<class P = void, bool isField = false>
using kpointer_t = RemoteSimpleType<addr_t, P, isField>;

#define THISTYPE typename std::remove_pointer<decltype(this)>::type

#define REMOTETYPE(tpname, tpdef)                            \
template<class P = void, bool isField = false>          \
struct tpname : public RemotePointer<P, isField> {     \
    using RemotePointer<P, isField>::RemotePointer;     \
    tpdef;                                              \
};                                                      \
using tpname##_p = tpname<>;

#define REMOTE_FIELD(fieldtype, fieldname, offs)                           \
    auto fieldname() {                                          \
        addr_t off = -1;                                        \
        offs;                                                   \
        if (off == -1) throw std::runtime_error("failed to get off for " #fieldtype "->" #fieldname); \
        return fieldtype<THISTYPE, true>(*this, off);    \
    }
