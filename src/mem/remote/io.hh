#ifndef __REMOTE_IO_H_
#define __REMOTE_IO_H_

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "message.hh"

class Socket {
  protected:
    char snd_buffer[4 * 1024];
    char recv_buffer[128];
    size_t snd_ind;
    int sockfd;
    struct sockaddr_un sockaddr;

  public:
    Socket(const pid_t uid=0);
    ~Socket();
    ssize_t sendMsg(const m5Message& msg);
    /** Immediately send buffered messages. */
    ssize_t flush();
    /** Returns size read */
    ssize_t recvMsg(m5Message& msg) const;
    /** Returns message id on success, <0 on error. */
    ssize_t recvMsg(void* out, const size_t max_len, bool block=true) const;
};

class Server: public Socket {
  private:
    int connfd;

  public:
    Server(const pid_t uid=0);
    ~Server();
    void connect_client();
};

class Client: public Socket {
  public:
    Client(const pid_t uid=0);
};

#endif // __REMOTE_IO_H_
