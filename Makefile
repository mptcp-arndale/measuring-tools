CFLAGS := -O2 -Wall -std=c99

.PHONY: clean all

all: trafficgen echod stream_measurer

trafficgen: trafficgen.c
	$(CC) $(CFLAGS) -o trafficgen trafficgen.c

echod: echod.c
	$(CC) $(CFLAGS) -o echod echod.c

stream_measurer: stream_measurer.c
	$(CC) $(CFLAGS) -o stream_measurer stream_measurer.c

clean:
	rm -f echod trafficgen stream_measurer

