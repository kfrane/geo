CC	= g++

CFLAGS = -std=c++11
SYSLIB = -rdynamic -lhiredis -lpthread -levent
REDIS_BASE_DIR = ../cpp-hiredis-cluster
INCLUDES = -I$(REDIS_BASE_DIR)/include

all: redis_client

redis_client: src/redis_client.cpp src/redis_client.h
	$(CC) $(CFLAGS) $(INCLUDES) src/redis_client.cpp $(SYSLIB) -o redis_client

.PHONY: clean

clean:
	rm redis_client *.o
