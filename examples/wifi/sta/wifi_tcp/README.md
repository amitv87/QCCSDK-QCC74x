# wifi6 tcp case

## Support CHIP

|      CHIP        | Remark |
|:----------------:|:------:|
|qcc743/qcc744       |        |

## Compile

- qcc743/qcc744

```bash
make CHIP=qcc743 BOARD=qcc743dk
```

## Flash

```bash
make flash COMX=xxx ## xxx is your com name
```

## How use wifi tcp test

example
```             
              ))    (                            ____________
    \|/      )))    ((   \|/  \|/               |            |
     |______           ___|____|___             |  Host pc   |
    |       |         |            |            |____________|
    | qcc743 |         |   Router   |#<-------->#/            /#
    |_______|         |____________|           /____________/#   
   192.168.1.3         192.168.1.1              192.168.1.2    
                      SSID: QCC74x_TEST
                    Password:12345678
```

### tcp client test

On Linux Host pc run netcat command, listen localhost 3365 port

```bash
$ nc -lp 3365
```

On qcc743 board, using <wifi_sta_connect> command connect your WiFi router

```bash
qxx74x />wifi_sta_connect QCC74x_TEST 12345678
qxx74x />wifi_tcp_test 192.168.1.2 3365
qxx74x />tcp client task start ...
Server ip Address : 192.168.1.2:3365
TCP client connect server success!
Press CTRL-C to exit.

```
if connect success, Linux Host pc have receive data the qcc743 send.

### tcp server echo test

On qcc743 Board start tcp server

```bash
qxx74x />wifi_sta_connect QCC74x_TEST 12345678
qxx74x />wifi_tcp_echo_test 3365
qxx74x />tcp server task start ...
Server ip Address : 0.0.0.0:3365
# host pc connected log
new client connected from (192.168.1.2, 57480)
Press CTRL-C to exit.
recv 7 len data

```

On Linux Host pc using <nc> command connect server:

```bash
$ nc -v 192.168.1.3 3365
Connection to 192.168.1.3 3365 port [tcp/*] succeeded!
123456  # enter something
123456  # echo received data

```

