#define _XOPEN_SOURCE 600

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <time.h>
#include <netdb.h>
#include <stdio.h>
#include <locale.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>

#define PACKET_SIZE 81920
#define TICK_SIZE 5000000 // 5ms

static int64_t max(int64_t a, int64_t b)
{
    return a > b ? a : b;
}

static void assume_valid_fd(const char* failurer, int s)
{
    if ( s == -1 ) {
        perror(failurer);
    }
}

static int64_t base_time;

static int64_t get_time_raw(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t) ts.tv_sec * 1000000000LL + (int64_t) ts.tv_nsec;
}

static int64_t get_time(void)
{
    return get_time_raw()-base_time;
}

int main(int argc, char** argv)
{
    signal(SIGPIPE, SIG_IGN);

    if ( argc < 3 ) {
        printf("Usage:\n");
        printf("stream_measurer [PORT] [BYTES] [COMMAND] [ARGS]\n");
        printf("\n");
        printf("This will expect BYTES number of bytes on given TCP port.\n");
        printf("When the receiving is half-way complete, it will run\n");
        printf("command and the given arguments (if given).\n");
        return 0;
    }

    int s = socket(AF_INET, SOCK_STREAM, 0);
    int enable = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable) );
    assume_valid_fd("socket", s);

    struct sockaddr_in sai;
    memset( &sai, 0, sizeof(sai) );
    sai.sin_port = htons(atoi(argv[1]));
    if ( bind(s, (struct sockaddr*) &sai, sizeof(sai) ) ) {
        perror("bind");
        return -1;
    }
    if ( listen(s, 1) ) {
        perror("listen");
        return -1;
    }
    int new_s = accept(s, NULL, NULL);
    close(s);
    s = new_s;

    int mytrue = 1;
    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &mytrue, sizeof(int));

    signed long long num_bytes = atoll(argv[2]);
    signed long long cutoff = num_bytes / 2;

    char dummy_buf[PACKET_SIZE];
    memset(dummy_buf, 'A', PACKET_SIZE);

    char another_dummy_buf[PACKET_SIZE];
    int forked = 0;

    printf("\"Time(s)\", \"Bandwidth (KiB/s)\"\n");

    size_t received_data_since_last_epoch = 0;
    base_time = get_time_raw();
    int64_t last_epoch = get_time();


    int flags = fcntl(s, F_GETFL, 0);
    fcntl(s, F_SETFL, flags | O_NONBLOCK );

    struct pollfd my_s;
    my_s.fd = s;

    while(num_bytes > 0) {
        ssize_t received_data;

        int64_t now = get_time();
        my_s.events = POLLOUT | POLLIN;
        int result = poll(&my_s, 1, max(0, (TICK_SIZE - (now-last_epoch)) / 1000000));
        if ( result == -1 && errno == EINTR )
            continue;

        if ( result == 1 ) {
            while(1) {
                received_data = recv(s, another_dummy_buf, PACKET_SIZE, 0);
                if ( received_data == -1 && (errno == EINTR || errno == EAGAIN) ) {
                    break;
                } else if ( received_data == -1 ) {
                    fprintf(stderr, "Connection closed by an error.\n");
                    return -1;
                } else if ( received_data == 0 ) {
                    fprintf(stderr, "Connection closed by remote side.\n");
                    return -1;
                }
                received_data_since_last_epoch += received_data;
                num_bytes -= received_data;
            }
        }
        now = get_time();
        if ( now - last_epoch >= TICK_SIZE ) { // 5ms
            printf( "%g, %g\n"
                  , (double) now / 1000000000.0
                  , ((double) received_data_since_last_epoch /
                    ((double) now - (double) last_epoch) * (1000000000.0/1024.0)) );
            last_epoch = now;
            received_data_since_last_epoch = 0;
        }
        if ( num_bytes <= cutoff && !forked && argc >= 4 ) {
            forked = 1;
            fprintf(stderr, "Running your command at exactly %g.\n"
                   , (double) now / 1000000000.0 );
            // Don't stop sending the data for this
            char** copied_args = calloc(1, sizeof(char*) * argc );
            memcpy( &copied_args[0], &argv[3], (argc-3)*sizeof(char*) );

            pid_t t = fork();
            if (t == 0) {
                execvp(argv[3], copied_args);
                return -1;
            }
            // Don't bother with waiting for the child
            free(copied_args);
        }
    }

    return 0;
}

