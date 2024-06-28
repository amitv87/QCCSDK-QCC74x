# STM32 SPI WiFi iperf test

## Support CHIP

|      CHIP        | Remark |
|:----------------:|:------:|
|qcc743/qcc744       |        |

## How SPI WiFi iperf test

On STM32 board, using <wifi_connect> command connect your WiFi router

```bash
wifi_connect QCC74x_TEST 12345678
```
if connect success, the log of qcc743 will print the log of "CODE_WIFI_ON_GOT_IP", this may require waiting for a few seconds.

On Linux Host pc run the main.py script in this directory.

```bash
$ python3 main.py
```

### tcp server echo test

This is a test for TCP RX direction. On STM32 Board start tcp server, ips <pc_ipaddr>

For example, the client's IP address is 192.168.31.112 (Note that it needs to end with "\r\n")

```bash
ips 192.168.31.112
```

### tcp client echo test

This is a test for TCP TX direction. On STM32 Board start tcp client, ipc <pc_ipaddr>

For example, the client's IP address is 192.168.31.112 (Note that it needs to end with "\r\n")

```bash
ipc 192.168.31.112
```

### udp server echo test

This is a test for UDP RX direction. On STM32 Board start tcp client, ipus <pc_ipaddr>

For example, the client's IP address is 192.168.31.112 (Note that it needs to end with "\r\n")

```bash
ipus 192.168.31.112
```

### udp client echo test

This is a test for UDP TX direction. On STM32 Board start tcp client, ipu <pc_ipaddr>

For example, the client's IP address is 192.168.31.112 (Note that it needs to end with "\r\n")

```bash
ipu 192.168.31.112
```
