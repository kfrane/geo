CC	= g++

CFLAGS = -std=c++11 -O2
SYSLIB = -rdynamic -lhiredis -lpthread -levent
REDIS_BASE_DIR = ../cpp-hiredis-cluster
INCLUDES = -I$(REDIS_BASE_DIR)/include

all: update_benchmark

geohash.o: src/geohash.cpp src/geohash.h
	$(CC) -c $<

redis_client.o: src/redis_client.cpp src/redis_client.h src/geohash.h
	$(CC) -c $(CFLAGS) $(INCLUDES) $< $(SYSLIB)

update_benchmark: src/update_benchmark.cpp redis_client.o geohash.o
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(SYSLIB) -o$@

test: redis_client.o geohash.o
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(SYSLIB) -o$@

.PHONY: clean

clean:
	rm *.o update_benchmark
