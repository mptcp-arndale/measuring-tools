#define _XOPEN_SOURCE

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

int main(int argc, char** argv)
{
    signal(SIGPIPE, SIG_IGN);

    if ( argc < 3 ) {
        printf("Usage:\n");
        printf("trafficgen [TARGET] [PORT]\n");
        printf("\n");
        printf("This will send dummy data to target until the target "
               "closes connection.\n");
        return 0;
    }

    struct addrinfo* addr_result;

    int result = getaddrinfo(argv[1], argv[2], NULL, &addr_result);
    if ( result != 0 ) {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(result) );
        return -1;
    }

    int s = socket(addr_result->ai_family, SOCK_STREAM, 0);
    if ( s == -1 ) { perror("socket"); return -1; }

    if ( connect(s, addr_result->ai_addr, addr_result->ai_addrlen) != 0 ) {
        perror("connect");
        return -1;
    }

    int mytrue = 1;
    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &mytrue, sizeof(int));
    char dummybuf[1024];
    memset( dummybuf, 'b', 1024 );

    while(1) {
        int result = send( s, dummybuf, 1024, 0 );
        if ( result == -1 && errno == EINTR )
            continue;
        if ( result <= 0 )
            break;
    }
    return 0;
}

