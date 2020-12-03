#include <sys/types.h>
#include <sys/wait.h>

#include <csignal>
#include <ctime>
#include <iostream>

#include "mem/remote/io.hh"
#include "mem/remote/message.hh"

#define test_assert(code) do {                                               \
        printf("%20s | %-20s ...", __FUNCTION__, #code);                \
        int result = (code);                                            \
        if (!result) {                                                  \
            printf("\r%20s | %-20s (failure) - %s:%d.\n",               \
                   __FUNCTION__, #code, __FILE__, __LINE__);            \
            abort();                                                    \
        }  else                                                         \
            printf("\r%20s | %-20s (success)\n", __FUNCTION__, #code);  \
    } while (0);

void test_MemAccess(){
    MemAccess msg = MemAccess(MemAccess::Type::RW, 2345235, 0, 1, 4354353);
    MemAccess chk;
    char bytes[msg.len()];

    msg.copyBytes(bytes);
    chk.fromBytes(bytes);
    test_assert(msg == chk);
}

void test_Array(){
    Array<MemAccess> msg;
    Array<MemAccess> chk;

    msg.push_back(MemAccess(MemAccess::Type::RW, 343535, 0, 1, 4354353));
    msg.push_back(MemAccess(MemAccess::Type::W, 9343535, 346547, 0, 435));

    char bytes[msg.len()];

    msg.copyBytes(bytes);
    chk.fromBytes(bytes);
    test_assert(msg == chk);
}

void test_send_recv() {
    pid_t parent = getpid(), child;
    MemAccess msg = MemAccess(MemAccess::Type::RW, 2903847, 32940, 1, 4354353);
    MBind mb = MBind(0x435436534, 4096*16, 0x2,
                     MBind::Mode::MPOL_INTERLEAVE, 0);

    child = fork();
    if (child < 0) {
        perror("fork");
        abort();
    } else if (child == 0) {
        MBind chk(0,0,0);

        // Resume parent
        kill(parent, SIGCONT);
        // Wait server initialization to connect.
        raise(SIGSTOP);
        Client client = Client();
        // Client connected, sending, reveiving
        client.sendMsg(msg);
        client.recvMsg(chk);
        test_assert(mb == chk);
        exit(0);
    } else {
        int status = 0;
        // Wait child spawn
        raise(SIGSTOP);
        MemAccess chk;
        // Initialize server
        Server server = Server();
        // Resume sender connection to server.
        kill(child, SIGCONT);
        server.connect_client();

        // Wait sent message. Send other message
        server.recvMsg(chk);
        server.sendMsg(mb);
        test_assert(msg == chk);
        waitpid(child, &status, 0);
    }
}

int main(void) {
    test_MemAccess();
    test_Array();
    test_send_recv();
    return 0;
}
