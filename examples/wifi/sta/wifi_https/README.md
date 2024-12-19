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

## Note

The array qcc74x_test_cli_key_rsa of https/src/https.c is root CA of your request URL.

## How use wifi https test

### https test get a page

On qcc743 board, using <wifi_sta_connect> command connect your WiFi router

```bash
qxx74x />wifi_sta_connect QCC74X_TEST 12345678
qcc74x />wifi_https_test
dst_addr is 513B7522
                    [I:  28511714][qcc74x_https.c:127]    HTTP/S> qcc74x connect fd = 0x62fd6ae0
qcc74x />[I:  28767428][qcc74x_https.c:137]    HTTP/S> ret = -28
[I:  29784976][qcc74x_https.c:157]    HTTP/S> ret = 0
[I:  29785480][qcc74x_https.c:161]    HTTP/S> total time:1282 ms
[I:  30048487][qcc74x_https.c:169]    HTTP/S> rcv_ret = 692
[I:  30049058][qcc74x_https.c:173]    HTTP/S> proc_head_r 0, status_code 200, body_start_off 438
[I:  30049503][qcc74x_https.c:181]    HTTP/S> Copy to resp @off 0, len 254, 1st char 7B
qcc74x_TcpSslDisconnect end
[I:  30054665][qcc74x_https.c:265]    HTTP/S> total time:1551 ms
[I:  30055242][qcc74x_https.c:266]    HTTP/S> test_1: status_code 200, resp_len 254
resp body: { ...... }
parse json get ip: ip_of_8.8.4.4
parse json get city: city_of_8.8.4.4
```

### http post 

TODO

