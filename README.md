Dependencies:

sudo apt-get install curl

pip install gevent

pip install redis hiredis progressbar

pip install python-geohash

redis 3.2

hiredis-0.13.3 (the latest one currently)
https://github.com/shawn246/redis_client

cpp redis client acl (https://github.com/zhengshuxin/acl/tree/master/lib_acl_cpp/samples/redis)

How to run this?

redis.conf is included (only disables disk persistance).
Run this as root for better performance.
echo never > /sys/kernel/mm/transparent_hugepage/enabled

src/server.py server listens on 8088 port.


Query examples are in /examples.
