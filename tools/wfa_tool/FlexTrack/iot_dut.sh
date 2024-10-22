#!/bin/sh

export WFA_ENV_AGENT_IPADDR=127.0.0.1
export WFA_ENV_AGENT_PORT=8000

sudo killall -9 wfa_ca
sudo killall -9 wfa_dut
#sudo ifconfig eth0 192.168.250.66 netmask 255.255.255.0

sleep 2
sudo ./wfa_ca eth0 9000 127.0.0.1 8000 `pwd`/logs/wfa_caa_log.txt

