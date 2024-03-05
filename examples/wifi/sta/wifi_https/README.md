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

## How use wifi https test

### https test get a page

On qcc743 board, using <wifi_sta_connect> command connect your WiFi router

```bash
qxx74x />wifi_sta_connect QCC74X_TEST 12345678
qxx74x />wifi_https_test
qxx74x />dst_addr is F23265B4
[I:  52207293][qcc74x_https.c:126]    HTTP/S> qcc74x connect fd = 0xa800ee60
[I:  52221230][qcc74x_https.c:136]    HTTP/S> ret = -28
[I:  54080149][qcc74x_https.c:156]    HTTP/S> ret = 0
[I:  54080658][qcc74x_https.c:160]    HTTP/S> total time:1884447 us
[I:  54095499][qcc74x_https.c:168]    HTTP/S> rcv_ret = 1179
[I:  54096043][qcc74x_https.c:172]    HTTP/S> proc_head_r 0, status_code 200, body_start_off 400
[I:  54096479][qcc74x_https.c:180]    HTTP/S> Copy to resp @off 0, len 779, 1st char 3C
qcc74x_tcp_ssl_disconnect end
[I:  54104728][qcc74x_https.c:264]    HTTP/S> total time:1908498 us
[I:  54105226][qcc74x_https.c:265]    HTTP/S> test_1: status_code 200, resp_len 779
[I:  54105654][qcc74x_https.c:267]    HTTP/S> resp body: <!DOCTYPE html>
......

```

### http post 

TODO

