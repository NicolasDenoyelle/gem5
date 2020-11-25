#ifndef __REMOTE_MESSAGE_H_
#define __REMOTE_MESSAGE_H_

#include <stdint.h>

#include <cstdio>
#include <cstdlib>
#include <vector>

#include "sim/numa.hh"

#define PTR_OFF(ptr, offset) ((void*)(((char*)(ptr)) + offset))

/**
 * Abstract class representing messages exchanged between the remote controller
 * and the simulator.
 *
 * +--------------+---------------+----------+
 * | uid (8 bits) | len (64 bits) | ******** |
 * +--------------+---------------+----------+
 * <------------------ len ------------------>
 */
class m5Message {
  public:
    /**
     * Message unique id from bytes.
     */
    static uint8_t uid(const void* bytes) {
        return *(uint8_t *) bytes;
    }

    /**
     * Message len in bytes;
     */
    virtual uint64_t len() const = 0;

    /**
     * Message len in bytes from raw message;
     */
    static inline uint64_t len(const void* bytes) {
        return *(uint64_t *) PTR_OFF(bytes, 1);
    }

    /**
     * Must be settable from read bytes.
     */
    virtual void fromBytes(const void *bytes) = 0;

    template <class T=m5Message>
    static T fromBytes(const void *bytes) {
        T t;
        assert(m5Message::uid(bytes) == T::uid());
        t.fromBytes();
        return t;
    }

    /**
     * Must be sendable as bytes.
     * @param out: A buffer of at least len() bytes.
     */
    virtual void copyBytes(void* out) const = 0;
};

/**
 * Class representing an array of messages of the same type.
 * Used to ease sending / receiving fleets of the same type.
 */
template <class T=m5Message>
class Array: public m5Message {
  private:
    std::vector<T> _messages;

  public:

    Array(): _messages() {}
    Array(std::vector<T> __messages): _messages(__messages) {}
    inline const T& front() const { return _messages.front(); }
    inline T& front() { return _messages.front(); }
    inline const T& back() const { return _messages.back(); }
    inline T& back() { return _messages.back(); }
    inline size_t size() const { return _messages.size(); }
    inline void push_back (const T& val) { return _messages.push_back(val); }
    inline void pop_back() { _messages.pop_back(); }
    inline void clear() { _messages.clear(); }

    typedef typename std::vector<T>::iterator iterator;
    typedef typename std::vector<T>::const_iterator const_iterator;
    inline iterator begin() noexcept { return _messages.begin(); }
    inline const_iterator begin() const { return _messages.begin(); };
    inline iterator end() noexcept { return _messages.end(); }
    inline const_iterator end() const { return _messages.end(); };
    bool operator==(const Array<T>& other);
    inline bool operator!=(const Array<T>& other) {
        return !operator==(other);
    }

    //------------------------- Message implementation ---------------------//

    static inline uint8_t uid() { return 0; }
    uint64_t len() const override;
    /**
     * +--------------+---------------+------------------------+...
     * | uid (8 bits) | len (64 bits) | num_messages (64 bits) |...
     * +--------------+---------------+------------------------+...
     * <----------------------------------- len ----------------...
     * ...-------------------+
     * ... messages (n bits) |
     * ...-------------------+
     * ...------------------->
     */
    void copyBytes(void* out) const override;
    void fromBytes(const void *bytes) override;

    Array(const Array& a): _messages(a._messages) {}
};

/**
 * Implementation of Message class for memory accesses.
 */
class MemAccess: public m5Message {
  public:
    typedef enum Type : uint8_t {
        R = (1<<0), // Read memory access
        W = (1<<1), // Write memory access
        RW = (1<<2 | 1<<1), // Read and Write memory access
    } Type;

  private:
    Type _type;
    uint64_t _tick;
    uint64_t _addr;
    uint8_t _numa;
    uint32_t _tid;

  public:
    /** Attributes constructor */
    MemAccess(Type __type= Type::R, uint64_t __tick=0, uint64_t __addr=0,
              uint8_t __numa=0, uint32_t __tid=0):
        _type(__type), _tick(__tick), _addr(__addr), _numa(__numa),
        _tid(__tid) {}

    /** Copy constructor */
    MemAccess(const MemAccess &m):
        _type(m._type), _tick(m._tick), _addr(m._addr), _numa(m._numa),
        _tid(m._tid) {}

    inline Type type() const { return _type; }
    inline uint64_t tick() const { return _tick; }
    inline uint64_t address() const { return _addr; }
    inline uint8_t numaNode() const { return _numa; }
    inline uint32_t ThreadID() const { return _tid; }

    inline bool operator==(const MemAccess& other) {
        return _type == other._type && _addr == other._addr &&
            _numa == other._numa && _tid == other._tid;
    }
    inline bool operator!=(const MemAccess& other) {
        return !operator==(other);
    }

    //------------------------- Message implementation ---------------------//

    static inline uint8_t uid() { return 1; }
    inline uint64_t len() const override { return 31; }

    /**
     * Convert MemAccess to bytes.
     * +--------------+---------------+---------------+----------------+...
     * | uid (8 bits) | len (64 bits) | type (8 bits) | tick (64 bits  |...
     * +--------------+---------------+---------------+----------------+...
     * <------------------------------------------- len ----------------...
     * ...----------------+---------------+---------------+
     * ... addr (64 bits) | numa (8 bits) | tid (32 bits) |
     * ...----------------+---------------+---------------+
     * ...------------------------------------------------>
     */
    inline void copyBytes(void* out) const override {
        *(uint8_t *) (char*) out = MemAccess::uid();
        *(uint64_t *) PTR_OFF(out, 1) = len();
        *(uint8_t *) PTR_OFF(out, 9) = _type;
        *(uint64_t *) PTR_OFF(out, 10) = _tick;
        *(uint64_t *) PTR_OFF(out, 18) = _addr;
        *(uint8_t *) PTR_OFF(out, 26) = _numa;
        *(uint32_t *) PTR_OFF(out, 27) = _tid;
    }

    /** Constructor from byte stream  */
    void fromBytes(const void *bytes) override {
        _type = (Type) *(uint8_t*) PTR_OFF(bytes, 9);
        _tick = *(uint64_t *) PTR_OFF(bytes, 10);
        _addr = *(uint8_t *) PTR_OFF(bytes, 18);
        _numa = *(uint8_t *) PTR_OFF(bytes, 26);
        _tid = *(uint32_t *) PTR_OFF(bytes, 27);
    }
};

/**
 * Implementation of Message class for mbind syscall request.
 */
class MBind: public m5Message {
  public:
    typedef enum Flags: uint8_t {
        MPOL_MF_STRICT = NUMA::MPOL_MF_STRICT,
        MPOL_MF_MOVE = NUMA::MPOL_MF_MOVE,
        MPOL_MF_MOVE_ALL = NUMA::MPOL_MF_MOVE_ALL,
        MPOL_F_NODE = NUMA::MPOL_F_NODE,
        MPOL_F_ADDR = NUMA::MPOL_F_ADDR,
        MPOL_F_MEMS_ALLOWED = NUMA::MPOL_F_MEMS_ALLOWED,
    } Flags;

    typedef enum Mode: uint8_t {
        MPOL_DEFAULT = NUMA::MPOL_DEFAULT,
        MPOL_PREFERRED = NUMA::MPOL_PREFERRED,
        MPOL_BIND = NUMA::MPOL_BIND,
        MPOL_INTERLEAVE = NUMA::MPOL_INTERLEAVE,
        MPOL_LOCAL = NUMA::MPOL_LOCAL,
    } Mode;

  private:
    uint64_t _addr;
    uint64_t _size;
    uint64_t _nodeset;
    Mode _mode;
    uint8_t _flags;

  public:
    /** Attributes constructor */
    MBind(uint64_t addr=0, uint64_t size=0, uint64_t nodeset=0,
          Mode mode=Mode::MPOL_DEFAULT, uint8_t flags=Flags::MPOL_MF_MOVE):
        _addr(addr), _size(size), _nodeset(nodeset), _mode(mode),
        _flags(flags) {}

    /** Copy constructor */
    MBind(const MBind &m):
        _addr(m._addr), _size(m._size), _nodeset(m._nodeset),
        _mode(m._mode), _flags(m._flags) {}

    inline NUMA::MemPolicy toMemPolicy() const {
        return NUMA::MemPolicy(&_nodeset, 64, _mode, _flags);
    }

    inline uint64_t addr() const {return _addr;}
    inline uint64_t size() const {return _size;}
    inline uint64_t nodeset() const {return _nodeset;}
    inline Mode mode() const {return _mode;}
    inline uint8_t flags() const {return _flags;}

    inline bool operator==(const MBind& other) {
        return _addr == other._addr && _nodeset == other._nodeset &&
            _mode == other._mode && _flags == other._flags;
    }

    inline bool operator!=(const MBind& other) {
        return !operator==(other);
    }

    //------------------------- Message implementation ---------------------//

    static inline uint8_t uid() { return 2; }
    inline uint64_t len() const override { return 35; }

    /**
     * Convert MemAccess to bytes.
     * +--------------+---------------+----------------+----------------+...
     * | uid (8 bits) | len (64 bits) | addr (64 bits) | size (64 bits) |...
     * +--------------+---------------+----------------+----------------+...
     * <------------------------------------------- len -----------------...
     * ...-------------------+---------------+-----------------+
     * ... nodeset (64 bits) | mode (8 bits) | flags (8) bits) |
     * ...-------------------+---------------+-----------------+
     * ...----------------------------------------------------->
     */
    inline void copyBytes(void* out) const override {
        *(uint8_t *) (char*) out = MBind::uid();
        *(uint64_t *) PTR_OFF(out, 1) = len();
        *(uint64_t *) PTR_OFF(out, 9) = _addr;
        *(uint64_t *) PTR_OFF(out, 17) = _size;
        *(uint64_t *) PTR_OFF(out, 25) = _nodeset;
        *(uint8_t *) PTR_OFF(out, 33) = _mode;
        *(uint8_t *) PTR_OFF(out, 34) = _flags;
    }

    /** Constructor from byte stream  */
    void fromBytes(const void *bytes) override {
        _addr = *(uint64_t*) PTR_OFF(bytes, 9);
        _size = *(uint64_t *) PTR_OFF(bytes, 17);
        _nodeset = *(uint64_t *) PTR_OFF(bytes, 25);
        _mode = (Mode) *(uint8_t *) PTR_OFF(bytes, 33);
        _flags = *(uint8_t *) PTR_OFF(bytes, 34);
    }
};

#endif // __REMOTE_MESSAGE_H_
