#include "mem/record_mem.hh"

// Required from python wrapper: "params/RecordMemory.hh"
RecordMemory* RecordMemoryParams::create()
{
    return new RecordMemory(this);
}

RecordMemory::RecordMemory(const RecordMemoryParams* p) :
    BaseXBar(p) {}

PortID RecordMemory::findPort(AddrRange addr_range)
{
    auto i = portMap.contains(addr_range);
    if (i != portMap.end())
        return i->second;

    fatal("Unable to find destination for %s on %s\n", addr_range.to_string(),
          name());
}

PortID RecordMemory::findPort(Addr addr)
{
    return findPort(RangeSize(addr, 1));
}
