# wifi6

## Support CHIP

|      CHIP        | Remark |
|:----------------:|:------:|
|qcc743/qcc744       |        |

## Compile

- qcc743/qcc744

```
make CHIP=qcc743 BOARD=qcc743dk
```

## Flash

```
make flash COMX=xxx ## xxx is your com name
```

## How use wifi udp test

```             
              ))                                 ____________
    \|/      )))         \|/  \|/               |            |
     |______           ___|____|___             |  Host pc   |
    |       |         |            |            |____________|
    | qcc743 |         |   Router   |#<-------->#/            /#
    |_______|         |____________|           /____________/#   
   192.168.1.3         192.168.1.1              192.168.1.2    
                      SSID: QCC74x_TEST
                    Password:12345678
```

### udp echo test

On qcc743 Board start udp server

```bash
qxx74x />wifi_sta_connect QCC74x_TEST 12345678
qxx74x />wifi_udp_echo 3365
qxx74x />udp server task start ...
Server ip Address : 0.0.0.0:3365
udp bind port success!
Press CTRL-C to exit.
recv from 192.168.1.2
recv:X
recv from 192.168.1.2
recv:X
recv from 192.168.1.2
recv:X
recv from 192.168.1.2
recv:X
recv from 192.168.1.2
recv:1234567890

```

On Linux Host pc using <nc> command connect server:

```bash
$ nc -uv 192.168.1.3 3365
Connection to 192.168.1.3 3365 port [udp/*] succeeded!
XXXX
1234567890  # enter something
1234567890  # echo received data

```

