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

## wifi ssl test

On qcc743 board, using <wifi_sta_connect> command connect your WiFi router

On Another linux PC within the local area network, using <sudo openssl s_server -accept 33332 -rev -CAfile ca_1.crt -cert server_1.crt -key server_1.key --debug> command

On qcc743 board, useing <ssl_rev_client> connect to your another PC, and send a string(0123456789), and recv a reversed string.

qcc74x />ssl_rev_client 192.168.1.143 33332
tcp client connect 192.168.1.143:33332
tcp_client_connect fd:1
[MBEDTLS] Performing the SSL/TLS handshake ... 
[MBEDTLS] ...... Verifying peer X.509 certificate ... 
[MBEDTLS] ssl connect ok
[MBEDTLS] ...... > Send to server
[MBEDTLS] 12 bytes sent retry 0
[MBEDTLS] ...... < Read from server
[MBEDTLS] 11 bytes read
[MBEDTLS] received string is : 9876543210
