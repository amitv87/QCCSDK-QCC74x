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

## How use wifi http test

### http test get some page

On qcc743 board, using <wifi_sta_connect> command connect your WiFi router

```bash
qxx74x />wifi_sta_connect QCC74x_TEST 12345678
qxx74x />wifi_http_test www.gov.cn 
qxx74x />Http client task start ...
Host:www.gov.cn, Server ip Address : 61.136.232.149:80
Http client connect server success!
Press CTRL-C to exit.
HTTP/1.1 200 OK
......

```

### http post 

TODO

### gethostbyname(DNSv6, DNSv4)
```bash
qcc74x />gethostbyname bt.byr.cn
qcc74x />gethostbyname baidu.com
```
