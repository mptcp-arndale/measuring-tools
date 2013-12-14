Silly Measuring Tools
=====================

These tools may or may not help at measuring the effects of Multipath TCP.

All of these compile and run on Android using the C compiler from NDK without
any special dependencies.

Use `make` to compile:

    make

Or if you want to use Android C compiler, then set the `CC` environment
variable to the compiler.

    CC=arm-linux-androideabi-gcc make

trafficgen
----------

Dumps traffic to some place.

    trafficgen 1.2.3.4 1234
                  ^      ^
                  |      +--- port
               address

This is somewhat equivalent to:

    nc 1.2.3.4 1234 < /dev/zero

echod
-----

Echo server. That is, if you connect to it and send something, it will send it
right back to you. This can handle multiple connections at the same time and
won't stop by itself.

    echod 1234
            ^
            |
          port

This was written to work with `ipbench` tool that requires an echo server on
the target device.

stream_measurer
---------------

This tool generates .csv output that measures bandwidth usage for received
data.

    stream_measurer 1234 10000000 echo hello
                     ^       ^      ^   ^
                     |       |      +---+---+
                     |       |              |
                     |       +-----+  When the transfer is half-way finished,
                     |             |  run this command with arguments.
             listen on this port   |
                                   |
                    receive this many bytes until closing

This tool writes a .csv file to standard output. There are two columns, first
one is for time, in seconds, and second one is the number of kibibytes per
second of transferred data at that time. The first line is headers.

The last command is for generating an event that can change network conditions.
In Multipath TCP you can knock out one interface with this command.

This is example output (greatly shortened for clarity):

    "Time(s)", "Bandwidth (KiB/s)"
    0.00500511, 1614.04
    0.010013, 1003
    0.015019, 1500
    0.0200241, 1400
    0.02503, 1405.04
    0.030032, 1300
    0.0350391, 1300
    0.0400601, 1301
    ...

