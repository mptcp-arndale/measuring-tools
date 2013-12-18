#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main(int argc, char* argv[])
{
    signal(SIGPIPE, SIG_IGN);
    if ( argc < 2 ) {
        printf("Usage:\n");
        printf("echod [PORT]\n");
        printf("\n");
        printf("Listens on the given TCP port and echos back anything sent "
               "to it.\n");
        return 0;
    }

    unsigned short port = (unsigned short) atoi(argv[1]);

    int s = socket(AF_INET, SOCK_STREAM, 0);
    if ( s == -1) {
        perror("socket");
        return -1;
    }

    int enable = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable) );

    struct sockaddr_in sai;
    memset( &sai, 0, sizeof(sai) );
    sai.sin_family = AF_INET;
    sai.sin_port = htons( port );

    if ( bind(s, (struct sockaddr*) &sai, sizeof(sai)) == -1 ) {
        perror("bind");
        return -1;
    }
    if ( listen(s, 1000) == -1 ) {
        perror("listen");
        return -1;
    }

    printf("Listening on port %d.\n", (int) port);

    struct epoll_event ev;
    struct epoll_event evs[100];

    memset( &ev, 0, sizeof(ev) );

    ev.events = EPOLLIN;
    ev.data.fd = s;

    int ep = epoll_create(1000);
    epoll_ctl(ep, EPOLL_CTL_ADD, s, &ev);

    while(1)
    {
        int result = epoll_wait(ep, evs, 100, -1);
        if ( result == -1 && errno == EINTR ) continue;

        for ( int i1 = 0; i1 < result; ++i1 ) {
            int fd = evs[i1].data.fd;
            if ( fd == s ) {
                int new_fd = accept(s, NULL, NULL);
                ev.events = EPOLLIN;
                ev.data.fd = new_fd;
                epoll_ctl(ep, EPOLL_CTL_ADD, new_fd, &ev);
            } else {
                char buf[10000];
                ssize_t recv_b = recv(fd, buf, 10000, 0);
                if ( recv_b <= 0 ) {
                    close(fd);
                } else {
                    send(fd, buf, recv_b, 0);
                }
            }
        }
    }

    return 0;
}

