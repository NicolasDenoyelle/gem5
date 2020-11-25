#include <poll.h>

#include <cstdio>

#include "io.hh"

#define SOCK_TYPE SOCK_STREAM

Socket::Socket(const pid_t uid): snd_ind(0), sockfd(-1) {
    bzero(&snd_buffer, sizeof(snd_buffer));
    bzero(&sockaddr, sizeof(sockaddr));
    sockaddr.sun_family = AF_UNIX;
    snprintf(sockaddr.sun_path, sizeof(sockaddr.sun_path), "%d", uid);
}

Socket::~Socket(){
    flush();
    close(sockfd);
    unlink(sockaddr.sun_path);
}

ssize_t Socket::flush(){
    ssize_t w;
    size_t tot = 0;

    while (tot < snd_ind) {
        if ((w = write(sockfd, snd_buffer+tot, snd_ind-tot)) < 0)
            return w;
        tot += w;
    }
    snd_ind = 0;
    return tot;
}

ssize_t Socket::sendMsg(const m5Message& msg) {
    ssize_t err, len = msg.len();

    if ((snd_ind + len) >= sizeof(snd_buffer) && (err = flush()) < 0)
        return err;

    msg.copyBytes(snd_buffer + snd_ind);
    snd_ind += len;
    return len;
}

ssize_t Socket::recvMsg(void* out, const size_t max_len, bool block) const {
    ssize_t err;
    uint8_t id;
    uint64_t len;

    if (block)
        err = read(sockfd, &id, sizeof(id));
    else
        err = recv(sockfd, &id, sizeof(id), MSG_DONTWAIT);
    if (err != sizeof(id))
        return err;
    if (err <= 0)
        return err;
    err = read(sockfd, &len, sizeof(len));
    if (err != sizeof(len))
        return err;
    if (max_len < len)
        return -1;
    bzero((char*)out, len);
    *(uint8_t*) out = id;
    *(uint64_t*) ((char*)out + sizeof(id)) = len;
    err = sizeof(id) + sizeof(len);
    err = read(sockfd, (char*)out + err, len - err);

    if (err < 0)
        return err;

    return (ssize_t) m5Message::uid(out);
}

ssize_t Socket::recvMsg(m5Message& msg) const {
    ssize_t len = msg.len();
    char bytes[len+1];

    bzero(bytes, len+1);
    len = read(sockfd, bytes, len);
    if (len > 0)
        msg.fromBytes(bytes);
    return len;
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
