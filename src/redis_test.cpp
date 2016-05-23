#include <stdlib.h>
#include <stdio.h>
#include "acl_cpp/lib_acl.hpp"

static void test_redis_string(acl::redis_string& cmd, const char* key)
{
    acl::string val("test_value");

    // call redis-server: SET key value
    if (cmd.set(key, val.c_str()) == false)
    {
        printf("redis set error\r\n");
        return;
    }

    // clear the string buf space
    val.clear();

    // reset the redis command object for reusing it
    cmd.clear();

    // call redis-server: GET key
    if (cmd.get(key, val) == false)
        printf("get key error\r\n");
}

static void test_redis_key(acl::redis_key& cmd, const char* key)
{
    if (cmd.exists(key) == false)
        printf("key not exists\r\n");
    else
        printf("key exists\r\n");
}

int main()
{    // init socket module for windows
    acl::acl_cpp_init();

    const char* redis_addr = "127.0.0.1:6379";
    int conn_timeout = 10, rw_timeout = 10;

    // the redis client connection
    acl::redis_client conn(redis_addr, conn_timeout, rw_timeout);

    const char* key = "test_key";

    // test redis STRING command
    // bind redis_string command with redis connection
    acl::redis_string cmd_string(&conn);
    test_redis_string(cmd_string, key);

    // test redis KEY command with the same redis connection
    acl::redis_key cmd_key(&conn);
    test_redis_key(cmd_key, key);
}
