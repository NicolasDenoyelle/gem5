#ifndef __MEM_MEMTRACE_HH__
#define __MEM_MEMTRACE_HH__

#include <fstream>

#include "base/statistics.hh"
#include "mem/port.hh"
#include "params/MemTrace.hh"
#include "sim/probe/mem.hh"
#include "sim/sim_object.hh"

/**
 * The communication monitor is a SimObject which can monitor statistics of
 * the communication happening between two ports in the memory system.
 *
 * Currently the following stats are implemented: Histograms of read/write
 * transactions, read/write burst lengths, read/write bandwidth,
 * outstanding read/write requests, read latency and inter transaction time
 * (read-read, write-write, read/write-read/write). Furthermore it allows
 * to capture the number of accesses to an address over time ("heat map").
 * All stats can be disabled from Python.
 */
class MemTrace : public SimObject
{

  public: // Construction & SimObject interfaces

    /** Parameters of communication monitor */
    typedef MemTraceParams Params;
    const Params* params() const
    { return reinterpret_cast<const Params*>(_params); }

    /**
     * Constructor based on the Python params
     *
     * @param params Python parameters
     */
    MemTrace(Params* params);
    ~MemTrace();

    void init() override;
    void regProbePoints() override;

  public: // SimObject interfaces
    Port &getPort(const std::string &if_name,
                  PortID idx=InvalidPortID) override;

  private:

    /**
     * Sender state class for the monitor so that we can annotate
     * packets with a transmit time and receive time.
     */
    class MemTraceSenderState : public Packet::SenderState
    {

      public:

        /**
         * Construct a new sender state and store the time so we can
         * calculate round-trip latency.
         *
         * @param _transmitTime Time of packet transmission
         */
        MemTraceSenderState(Tick _transmitTime)
            : transmitTime(_transmitTime)
        { }

        /** Destructor */
        ~MemTraceSenderState() { }

        /** Tick when request is transmitted */
        Tick transmitTime;

    };

    /**
     * This is the master port of the communication monitor. All recv
     * functions call a function in MemTrace, where the
     * send function of the slave port is called. Besides this, these
     * functions can also perform actions for capturing statistics.
     */
    class MemTraceMasterPort : public MasterPort
    {

      public:

        MemTraceMasterPort(const std::string& _name, MemTrace& _mon)
            : MasterPort(_name, &_mon), mon(_mon)
        { }

      protected:

        void recvFunctionalSnoop(PacketPtr pkt)
        {
            mon.recvFunctionalSnoop(pkt);
        }

        Tick recvAtomicSnoop(PacketPtr pkt)
        {
            return mon.recvAtomicSnoop(pkt);
        }

        bool recvTimingResp(PacketPtr pkt)
        {
            return mon.recvTimingResp(pkt);
        }

        void recvTimingSnoopReq(PacketPtr pkt)
        {
            mon.recvTimingSnoopReq(pkt);
        }

        void recvRangeChange()
        {
            mon.recvRangeChange();
        }

        bool isSnooping() const
        {
            return mon.isSnooping();
        }

        void recvReqRetry()
        {
            mon.recvReqRetry();
        }

        void recvRetrySnoopResp()
        {
            mon.recvRetrySnoopResp();
        }

      private:

        MemTrace& mon;

    };

    /** Instance of master port, facing the memory side */
    MemTraceMasterPort masterPort;

    /**
     * This is the slave port of the communication monitor. All recv
     * functions call a function in MemTrace, where the
     * send function of the master port is called. Besides this, these
     * functions can also perform actions for capturing statistics.
     */
    class MemTraceSlavePort : public SlavePort
    {

      public:

        MemTraceSlavePort(const std::string& _name, MemTrace& _mon)
            : SlavePort(_name, &_mon), mon(_mon)
        { }

      protected:

        void recvFunctional(PacketPtr pkt)
        {
            mon.recvFunctional(pkt);
        }

        Tick recvAtomic(PacketPtr pkt)
        {
            return mon.recvAtomic(pkt);
        }

        bool recvTimingReq(PacketPtr pkt)
        {
            return mon.recvTimingReq(pkt);
        }

        bool recvTimingSnoopResp(PacketPtr pkt)
        {
            return mon.recvTimingSnoopResp(pkt);
        }

        AddrRangeList getAddrRanges() const
        {
            return mon.getAddrRanges();
        }

        void recvRespRetry()
        {
            mon.recvRespRetry();
        }

        bool tryTiming(PacketPtr pkt)
        {
            return mon.tryTiming(pkt);
        }

      private:

        MemTrace& mon;

    };

    /** Instance of slave port, i.e. on the CPU side */
    MemTraceSlavePort slavePort;

    void recvFunctional(PacketPtr pkt);

    void recvFunctionalSnoop(PacketPtr pkt);

    Tick recvAtomic(PacketPtr pkt);

    Tick recvAtomicSnoop(PacketPtr pkt);

    bool recvTimingReq(PacketPtr pkt);

    bool recvTimingResp(PacketPtr pkt);

    void recvTimingSnoopReq(PacketPtr pkt);

    bool recvTimingSnoopResp(PacketPtr pkt);

    void recvRetrySnoopResp();

    AddrRangeList getAddrRanges() const;

    bool isSnooping() const;

    void recvReqRetry();

    void recvRespRetry();

    void recvRangeChange();

    bool tryTiming(PacketPtr pkt);

  protected: // Probe points
    /**
     * @{
     * @name Memory system probe points
     */

    /** Successfully forwarded request packet */
    ProbePoints::PacketUPtr ppPktReq;

    /** Successfully forwarded response packet */
    ProbePoints::PacketUPtr ppPktResp;

    /** @} */

  private:
    std::ofstream dump_file;
    void outputMemoryAccess(PacketPtr pkt);
};

#endif //__MEM_MEMTRACE_HH__
