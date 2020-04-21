#include "mem/memtrace.hh"

#include <sstream>

#include "base/output.hh"
#include "debug/MemTrace.hh"

MemTrace::MemTrace(Params* params)
    : SimObject(params),
      masterPort(name() + "-master", *this),
      slavePort(name() + "-slave", *this),
      dump_file(simout.resolve(params->dump_file))
{
    DPRINTF(MemTrace, "Created memtrace %s\n", name());
    dump_file << "# tick R/W isNotPrefetch? addr size" << '\n';
}

MemTrace::~MemTrace() {
    dump_file.close();
    inform("Memtrace dumped to %s\n", simout.resolve(params()->dump_file));
}

void outputMemoryAccess(PacketPtr pkt) {
}

MemTrace*
MemTraceParams::create()
{
    return new MemTrace(this);
}

void
MemTrace::init()
{
    // make sure both sides of the monitor are connected
    if (!slavePort.isConnected() || !masterPort.isConnected())
        fatal("Memtrace monitor is not connected on both sides.\n");
}

void
MemTrace::regProbePoints()
{
    ppPktReq.reset(new ProbePoints::Packet(getProbeManager(), "PktRequest"));
    ppPktResp.reset(new ProbePoints::Packet(getProbeManager(), "PktResponse"));
}

Port &
MemTrace::getPort(const std::string &if_name, PortID idx)
{
    if (if_name == "master") {
        return masterPort;
    } else if (if_name == "slave") {
        return slavePort;
    } else {
        return SimObject::getPort(if_name, idx);
    }
}

void
MemTrace::recvFunctional(PacketPtr pkt)
{
    masterPort.sendFunctional(pkt);
}

void
MemTrace::recvFunctionalSnoop(PacketPtr pkt)
{
    slavePort.sendFunctionalSnoop(pkt);
}

Tick
MemTrace::recvAtomic(PacketPtr pkt)
{
    const bool expects_response(pkt->needsResponse() &&
                                !pkt->cacheResponding());
    ProbePoints::PacketInfo req_pkt_info(pkt);
    ppPktReq->notify(req_pkt_info);

    const Tick delay(masterPort.sendAtomic(pkt));

    outputMemoryAccess(pkt);

    // Some packets, such as WritebackDirty, don't need response.
    assert(pkt->isResponse() || !expects_response);
    ProbePoints::PacketInfo resp_pkt_info(pkt);
    ppPktResp->notify(resp_pkt_info);
    return delay;
}

Tick
MemTrace::recvAtomicSnoop(PacketPtr pkt)
{
    return slavePort.sendAtomicSnoop(pkt);
}

bool
MemTrace::recvTimingReq(PacketPtr pkt)
{
    // should always see a request
    assert(pkt->isRequest());

    // Store relevant fields of packet, because packet may be modified
    // or even deleted when sendTiming() is called.
    const ProbePoints::PacketInfo pkt_info(pkt);

    const bool expects_response(pkt->needsResponse() &&
                                !pkt->cacheResponding());

    // If a cache miss is served by a cache, a monitor near the memory
    // would see a request which needs a response, but this response
    // would not come back from the memory. Therefore we additionally
    // have to check the cacheResponding flag
    if (expects_response) {
        pkt->pushSenderState(new MemTraceSenderState(curTick()));
    }

    // Attempt to send the packet
    bool successful = masterPort.sendTimingReq(pkt);

    // If not successful, restore the sender state
    if (!successful && expects_response) {
        delete pkt->popSenderState();
    }

    if (successful) {
        ppPktReq->notify(pkt_info);
    }

    if (successful) {
        DPRINTF(MemTrace, "Forwarded %s request\n", pkt->isRead() ? "read" :
                pkt->isWrite() ? "write" : "non read/write");
        outputMemoryAccess(pkt);
    }
    return successful;
}

bool
MemTrace::recvTimingResp(PacketPtr pkt)
{
    // should always see responses
    assert(pkt->isResponse());

    // Store relevant fields of packet, because packet may be modified
    // or even deleted when sendTiming() is called.
    const ProbePoints::PacketInfo pkt_info(pkt);

    Tick latency = 0;
    MemTraceSenderState* received_state =
        dynamic_cast<MemTraceSenderState*>(pkt->senderState);

    // Restore initial sender state
    if (received_state == NULL)
        panic("Monitor got a response without monitor sender state\n");

    // Restore the state
    pkt->senderState = received_state->predecessor;

    // Attempt to send the packet
    bool successful = slavePort.sendTimingResp(pkt);

    // If packet successfully send, sample value of latency,
    // afterwards delete sender state, otherwise restore state
    if (successful) {
        latency = curTick() - received_state->transmitTime;
        DPRINTF(MemTrace, "Latency: %d\n", latency);
        delete received_state;
    } else {
        // Don't delete anything and let the packet look like we
        // did not touch it
        pkt->senderState = received_state;
    }

    if (successful) {
        ppPktResp->notify(pkt_info);
        DPRINTF(MemTrace, "Received %s response\n", pkt->isRead() ? "read" :
                pkt->isWrite() ?  "write" : "non read/write");
        outputMemoryAccess(pkt);
    }
    return successful;
}

void
MemTrace::recvTimingSnoopReq(PacketPtr pkt)
{
    slavePort.sendTimingSnoopReq(pkt);
}

bool
MemTrace::recvTimingSnoopResp(PacketPtr pkt)
{
    return masterPort.sendTimingSnoopResp(pkt);
}

void
MemTrace::recvRetrySnoopResp()
{
    slavePort.sendRetrySnoopResp();
}

bool
MemTrace::isSnooping() const
{
    // check if the connected master port is snooping
    return slavePort.isSnooping();
}

AddrRangeList
MemTrace::getAddrRanges() const
{
    // get the address ranges of the connected slave port
    return masterPort.getAddrRanges();
}

void
MemTrace::recvReqRetry()
{
    slavePort.sendRetryReq();
}

void
MemTrace::recvRespRetry()
{
    masterPort.sendRetryResp();
}

bool
MemTrace::tryTiming(PacketPtr pkt)
{
    return masterPort.tryTiming(pkt);
}

void
MemTrace::recvRangeChange()
{
    slavePort.sendRangeChange();
}

void MemTrace::outputMemoryAccess(PacketPtr pkt) {
    if (!pkt->hasData() ||
        pkt->isUpgrade() ||
        pkt->isInvalidate() ||
        pkt->isEviction() ||
        pkt->isClean() ||
        pkt->isError() ||
        pkt->isPrint() ||
        pkt->isFlush())
        return;

    std::ostringstream out;

    out << curTick() << ' ';
    if (pkt->isRead())
        out << "Read";
    else if (pkt->isWrite())
        out << "Write";
    else if (pkt->isWriteback())
        out << "Write";
    out << ' ' << (pkt->cmd.isPrefetch() ? '0' : '1');
    out << ' ' << pkt->getAddr();
    out << ' ' << pkt->getSize();
    out << '\n';
    dump_file << out.str();
}
