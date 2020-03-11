/**
 * @file
 * RecordMemory declaration
 */

#ifndef __MEM_RECORD_MEMORY_HH__
#define __MEM_RECORD_MEMORY_HH__

#include <list>

#include "mem/abstract_mem.hh"
#include "mem/port.hh"
#include "params/RecordMemory.hh"

/**
 * The record memory is a basic single-ported memory controller with
 * a configurable throughput and latency.
 *
 * @sa  \ref gem5MemorySystem "gem5 Memory System"
 */
class RecordMemory : public AbstractMemory
{
  private:
    class MemorySlavePort : public SlavePort
    {
      private:
        RecordMemory& memory;

      public:
        MemorySlavePort(const std::string& _name, RecordMemory& _memory):
            SlavePort(_name, &_memory), memory(_memory){}

      protected:
        // Required from protocol/atomic.hh: AtomicResponseProtocol
        Tick recvAtomic(PacketPtr pkt) override
        {
            return memory.recvAtomic(pkt);
        }

        // Required from protocol/atomic.hh: AtomicResponseProtocol
        Tick recvAtomicBackdoor(
            PacketPtr pkt, MemBackdoorPtr &_backdoor) override
        {
            return memory.recvAtomicBackdoor(pkt, _backdoor);
        }

        // Required from protocol/functional.hh: FunctionalResponseProtocol
        void recvFunctional(PacketPtr pkt) override
        {
            memory.recvFunctional(pkt);
        }

        // Required from protocol/timing.hh: TimingResponseProtocol
        bool recvTimingReq(PacketPtr pkt) override {
            return memory.recvTimingReq(pkt);
        }

        // Required from protocol/timing.hh: TimingResponseProtocol
        void recvRespRetry() override  {
            memory.recvRespRetry();
        }

        // Required from port.hh: SlavePort
        AddrRangeList getAddrRanges() const override {
            AddrRangeList ranges;
            ranges.push_back(memory.getAddrRange());
            return ranges;
        }
    };

    MemorySlavePort slave_port;

    class MemoryMasterPort : public MasterPort
    {
      private:
        RecordMemory& memory;

      public:
        MemoryMasterPort(const std::string& _name, RecordMemory& _memory):
            MasterPort(_name, &_memory), memory(_memory){}

      protected:
        // Required from protocol/timing.hh: AtomicRequestProtocol
        bool recvTimingResp(PacketPtr pkt) override {
            return memory.recvTimingResp(pkt);
        }

        // Required from protocol/timing.hh: AtomicRequestProtocol

        void recvReqRetry() override {
            memory.recvReqRetry();
        }

        // Required from protocol/timing.hh: AtomicRequestProtocol
        void recvRetrySnoopResp() override {
            memory.recvReqRetry();
        }
    };

    MemoryMasterPort master_port;

  public:
    RecordMemory(const RecordMemoryParams *p);

  protected:
    // Slave port implementation
    Tick recvAtomic(PacketPtr pkt);
    Tick recvAtomicBackdoor(PacketPtr pkt, MemBackdoorPtr &_backdoor);
    void recvFunctional(PacketPtr pkt);
    bool recvTimingReq(PacketPtr pkt);
    void recvRespRetry();
    // Master port implementation
    bool recvTimingResp(PacketPtr pkt);
    void recvReqRetry();
    void recvRetrySnoopResp();
};

#endif //__MEM_RECORD_MEMORY_HH__
