#include "mem/record_mem.hh"

// Required from python wrapper: "params/RecordMemory.hh"
RecordMemory* RecordMemoryParams::create()
{
    return new RecordMemory(this);
}

RecordMemory::RecordMemory(const RecordMemoryParams* p) :
    AbstractMemory(p),
    slave_port(name() + ".port", *this),
    master_port(name() + ".port", *this) {}
