## Prepare to check
1. Append content to your /etc/hosts, this localhost_1 is COMMON NAME of certificate.
```
127.0.0.1   localhost_1
```
2. Start Borker backend
```
mosquitto -c ssl/mos.conf
```
3. Subscribe test title
```
mosquitto_sub -h localhost_1 -p 8883 --cafile ssl/ca_1.crt -t test
```
4. Publish hello message for test title
```
mosquitto_pub -h localhost_1 -p 8883 --cafile ssl/ca_1.crt -t test -m hello
```
5. connect is ok, can receive hello from subscribe.

## DEMO as subscribe
1. stop previous mosquitto task, mosquitto_sub task and mosquitto_pub task.
2. Start Borker backend
```
mosquitto -c ssl/mos.conf
```
3. Connect your ssid, and subscribe listen.
```
qcc74x /> wifi_sta_connect your_ssid 12345678
qcc74x /> mqtts_sub 192.168.1.143 8883
tcp client connect 192.168.1.143:8883
tcp_client_connect fd:1
[MBEDTLS] Performing the SSL/TLS handshake ... 
[MBEDTLS] ...... Verifying peer X.509 certificate ... 
[MBEDTLS] ssl connect ok
mqtts_sub listening for 'qcc74x_undef' messages.
```
4. Publish hello message for test title, and receive from your qcc74x
```
mosquitto_pub -h localhost_1 -p 8883 --cafile ssl/ca_1.crt -t test -m hello

Received publish('qcc74x_undef'): hello
Received publish('qcc74x_undef'): hello
```

## DEMO as publish
1. stop previous mosquitto task, mosquitto_sub task and mosquitto_pub task.
2. Start Borker backend, and subscribe
```
mosquitto -c ssl/mos.conf
mosquitto_sub -h localhost_1 -p 8883 --cafile ssl/ca_1.crt -t test
```
3. Connect your ssid, and loop publish
```
qcc74x /> wifi_sta_connect your_ssid 12345678
qcc74x />mqtts_pub 192.168.1.143 8883
tcp client connect 192.168.1.143:8883
tcp_client_connect fd:1
[MBEDTLS] Performing the SSL/TLS handshake ... 
[MBEDTLS] ...... Verifying peer X.509 certificate ... 
[MBEDTLS] ssl connect ok
mqtts_pub is ready to begin publishing the time.
mqtts_pub published : "{"hello mqtt by qcc743/8 !"}
Now time is 2023-06-16 00:00:00"
mqtts_pub published : "{"hello mqtt by qcc743/8 !"}
Now time is 2023-06-16 00:00:03"
mqtts_pub published : "{"hello mqtt by qcc743/8 !"}
```
4. Get message from your subcribe
```
{"hello mqtt by qcc743/8 !"}
Now time is 2023-06-16 00:00:00
{"hello mqtt by qcc743/8 !"}
Now time is 2023-06-16 00:00:03
{"hello mqtt by qcc743/8 !"}
Now time is 2023-06-16 00:00:06
{"hello mqtt by qcc743/8 !"}
Now time is 2023-06-16 00:00:09
{"hello mqtt by qcc743/8 !"}
Now time is 2023-06-16 00:00:12
{"hello mqtt by qcc743/8 !"}
Now time is 2023-06-16 00:00:00
{"hello mqtt by qcc743/8 !"}
Now time is 2023-06-16 00:00:03
```
