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

## How use wifi mqtt sub test

1. Find some mqtt server or your deployed a mqtt server first. This case default used `test.mosquitto.org:1883` server test; or used EMQX mqtt public server test.
2. Use mqtt client to test if the server is working. You can install `MQTTBox` on windows for testing, you can install it from Microsoft Store. Or use EMQX's online mqtt client for testing.
3. If the server works fine, then you can continue with the next test.
4. connect your WiFi.
5. connect MQTT server. command like `mqtt_sub` or `mqtt_sub <server ip> <server port>`

Steps:

```bash
linux-bash /> mosquitto -p 8883
```

```bash
qxx74x />wifi_sta_connect QCC74x_TEST 12345678
qcc74x />mqtt_sub 192.168.1.143 8883
mqtt_sub listening for 'qcc74x_undef' messages.
Press CTRL-C to exit.
Received publish('qcc74x_undef'): hello
```

```bash
linux-bash /> mosquitto_pub -h 127.0.0.1 -p 8883 -t qcc74x_undef -m hello
```
