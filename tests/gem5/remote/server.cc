#include <cassert>

#include "mem/remote/io.hh"
#include "mem/remote/message.hh"

int main(void) {
    MemAccess msg = MemAccess(MemAccess::Type::RW, 232830, 0, 1, 4354353);
    MBind chk, mb = MBind(0x435436534, 4096*16, 0x2,
                          MBind::Mode::MPOL_INTERLEAVE, 0);

    // Initialize server
    Server server = Server();
    server.connect_client();

    server.sendMsg(msg);
    server.recvMsg(chk);

    assert(mb == chk);
    return 0;
}
