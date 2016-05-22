Dependencies:

sudo apt-get install curl

pip install gevent

pip install redis hiredis progressbar

pip install python-geohash

redis 3.2

How to run this?

redis.conf is included (only disables disk persistance).
Run this as root for better performance.
echo never > /sys/kernel/mm/transparent_hugepage/enabled

src/server.py server listens on 8088 port.


Query examples are in /examples.
