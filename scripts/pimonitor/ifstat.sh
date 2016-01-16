#!/bin/bash

#wlan0     Link encap:Ethernet  HWaddr 00:e0:4c:0b:f8:b8  
#          inet addr:192.168.110.234  Bcast:192.168.110.255  Mask:255.255.255.0
#          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
#          RX packets:33911 errors:0 dropped:2273 overruns:0 frame:0
#          TX packets:26947 errors:0 dropped:0 overruns:0 carrier:0
#          collisions:0 txqueuelen:1000 
#          RX bytes:5494445 (5.2 MiB)  TX bytes:4564971 (4.3 MiB)

IFACE=$(ifconfig)
echo $IFACE
