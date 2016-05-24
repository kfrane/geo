CC	= g++

CFLAGS = -std=c++11
SYSLIB = -rdynamic -lhiredis -lpthread -levent
REDIS_BASE_DIR = ../cpp-hiredis-cluster
INCLUDES = -I$(REDIS_BASE_DIR)/include

all: geohash redis_client update_benchmark

geohash: src/geohash.c src/geohash.h
	$(CC) -c $<

redis_client: src/redis_client.cpp src/redis_client.h src/geohash.h
	$(CC) -c $(CFLAGS) $(INCLUDES) $< $(SYSLIB)

update_benchmark: src/update_benchmark.cpp redis_client.o geohash.o
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(SYSLIB) -o$@

.PHONY: clean

clean:
	rm redis_client *.o update_benchmark
