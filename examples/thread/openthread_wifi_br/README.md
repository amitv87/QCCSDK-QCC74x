# OpenThread command line example
OpenThread porting is over `components/wireless/lmac154` module.


## Support CHIP

|      CHIP        | Remark |
|:----------------:|:------:|
|QCC743/QCC74x_undef       |        |

## Compile

- QCC743/QCC74x_undef

```
make CHIP=qcc743 BOARD=qcc743dk
```


## Flash

```
make flash CHIP=chip_name COMX=xxx # xxx is your com name
```

### Hardwares

  Prepare:
    - One board, named OTBR, which is flashed this example.
    - One board, named OTCLI, which is flashed openthread command line example.

### Steps

  - Open OTBR UART in your test PC station, and type following command to connect OTBR to Wi-Fi AP.

    ```shell
    qcc74x />wifi_sta_connect <SSID> <PASSWORD>
    ```
    > OTBR will acquire IP address after it connects to Wi-Fi AP successfully;
    > then starts/attaches Thread network, and plays Thread Border Routing forwarding.

    > After IP address acqCopy hex string above for next setup.
  - Type following command to get Thread network credential from OTBR

    ```shell
    qcc74x />otc dataset active -x
    dataset active -x
    0e080000000000010000000300001735060004001fffe002086e0f33c1aa1076d30708fd563c8a2072356f05101f5be8fb42b487278a27619c7a0f2470030f4f70656e5468726561642d3765363501027e6504107ccd496e4c41be0651547fe30355ff330c0402a0f7f8
    Done
    ```

  - Open OTCLI UART in your test PC station, input the following commands to make OTCLI attach to OTBR Thread network.

    ```shell
    qcc74x />otc dataset set active <Thread network credential hex string in last step>
    Done

    qcc74x />otc ifconfig up
    ifconfig up
    Done

    qcc74x />otc thread start
    thread start
    Done
    ```

    After OTCLI attches to OTBR Thread network, it will acquire an OMR IPv6 address. Then, you can ping this OMR IPv6 address from your test PC; or ping your test PC from OTCLI command line.
    > Please make sure both of OTBR and your test PC station connect to same router.
