#!/bin/bash
/etc/init.d/networking restart				# reload default configurations
ifconfig eth0 up					# activate eth0
ifconfig eth0 172.16.40.1/24				# identify eth0
route add -net 172.16.41.0/24 gw 172.16.40.254		# add route to tux44 in order to access network 172.16.41.0/24
route add default gw 172.16.40.254			# add tux44 default route of tux41 

cp /etc/resolv.conf /etc/resolv.conf.backup
echo "search lixa.netlab.fe.up.pt" > /etc/resolv.conf
echo "nameserver 172.16.1.2" >> /etc/resolv.conf

