/**
 * @file
 * RecordMemory declaration
 */

#ifndef __MEM_RECORD_MEMORY_HH__
#define __MEM_RECORD_MEMORY_HH__

#include <vector>

#include "base/addr_range_map.hh"
#include "mem/port.hh"
#include "params/RecordMemory.hh"
#include "sim/sim_object.hh"

/**
 * The record memory is a basic single-ported memory controller with
 * a configurable throughput and latency.
 *
 * @sa  \ref gem5MemorySystem "gem5 Memory System"
 */
class RecordMemory: public SimObject
{
  private:
    std::vector<SlavePort*> slavePorts;
    std::vector<MasterPort*> masterPorts;
    AddrRangeMap<PortID, 3> portMap;

    /**
     * Find which port connected to this crossbar (if any) should be
     * given a packet with this address range.
     *
     * @param addr_range Address range to find port for.
     * @return id of port that the packet should be sent out of.
     */
    PortID findPort(AddrRange addr_range);
    PortID findPort(Addr addr);

  public:
    RecordMemory(const RecordMemoryParams *p);

};

#endif //__MEM_RECORD_MEMORY_HH__
