#!bin/sh/

sh /usr/bin/start_ss.sh
mkdir dnsmasq
cd dnsmasq
touch ipset.conf
cp ../gfwlist.txt ./ipset.conf
