#include <poll.h>

#include <cstdio>

#include "io.hh"

#define SOCK_TYPE SOCK_SEQPACKET

Socket::Socket(const pid_t uid) {
    sockfd = -1;
    bzero(&sockaddr, sizeof(sockaddr));
    sockaddr.sun_family = AF_UNIX;
    snprintf(sockaddr.sun_path, sizeof(sockaddr.sun_path), "%d", uid);
}

Socket::~Socket(){
    close(sockfd);
    unlink(sockaddr.sun_path);
}

ssize_t Socket::sendMsg(const m5Message& msg) const {
    ssize_t len = msg.len();
    char bytes[len+1];

    bzero(bytes, len+1);
    msg.copyBytes(bytes);
    fprintf(stderr, "send %s\n", bytes);
    len = write(sockfd, bytes, len);

    if (len == -1)
        perror("write");
    return len;
}

ssize_t Socket::recvMsg(m5Message& msg) const {
    ssize_t len = msg.len();
    char bytes[len+1];

    bzero(bytes, len+1);
    len = read(sockfd, bytes, len);
    fprintf(stderr, "receive %s\n", bytes);
    if (len > 0)
        msg.fromBytes(bytes);
    return len;
}

bool Socket::isConnected() const {
    struct pollfd pollfd = {
        .fd = sockfd,
        .events = 0,
        .revents = 0
    };

    if (poll(&pollfd, 1, 100) < 0)
        return false;

    return (pollfd.revents & (POLLHUP | POLLERR | POLLNVAL)) == 0;
}

Server::Server(const pid_t uid): Socket(uid) {
    // Make sure the named socket is not in use.
    unlink(sockaddr.sun_path);

    // Setup connection socket
    connfd = socket(AF_UNIX, SOCK_TYPE, 0);
    if (connfd == -1) {
        perror("socket");
        abort();
    }

    if (bind(connfd, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) == -1) {
        perror("bind");
        goto err_with_sockfd;
    }

    if (listen(connfd, 1) == -1){
        perror("listen");
        goto err_with_fd;
    }
    return;
  err_with_fd:
    unlink(sockaddr.sun_path);
  err_with_sockfd:
    close(connfd);
    abort();
}

void Server::connect_client() {
    printf("System server waiting on a client.\n");
    sockfd = accept(connfd, NULL, NULL);
    if (sockfd == -1) {
        perror("accept");
        unlink(sockaddr.sun_path);
        close(connfd);
        abort();
    }
}

Server::~Server(){
    close(connfd);
}

Client::Client(const pid_t uid): Socket(uid) {
    sockfd = socket(AF_UNIX, SOCK_TYPE, 0);
    if (sockfd == -1) {
        perror("socket");
        goto err;
    }

    if (connect(sockfd, (struct sockaddr *) &sockaddr,
                sizeof(sockaddr)) == -1) {
        perror("connect");
        goto err_with_sockfd;
    }
    return;

  err_with_sockfd:
    close(sockfd);
  err:
    abort();
}
