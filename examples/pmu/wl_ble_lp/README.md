## BLE Low Power：

### BLE ADV:

1.Reset board

2\.

ADV = 2S: Enter the command "ble\_start\_adv 0 0 0x0c80 0x0c80 " in the command line.

OR

ADV = 500MS: Enter the command "ble\_start\_adv 0 0 0x0320 0x0320" in the command line.

3.Enter the command "tickless " in the command line.

### BLE CONNECTION:

1.Reset board

2.Enter ADV connectable mode: Enter the command "ble\_start\_adv 0 0 0x80 0x80 " in the command line.

3.Use Phone or other device connect IUT.

4. When connection established enter the command "tickless " in the command line.

### BLE Benchmark：

The following test data reduced the power of BLE TX. The modification is as follows: Change "pwr\_table\_ble = <13> to pwr\_table\_ble = <0>;" in the file "bsp/board/qcc743\_lp\_dk/config/qcc74x\_factory\_params\_IoTKitA\_auto.dts".

| Mode        | uA   |
| :---------- | :--- |
| Deep sleep  | 50   |
| ADV = 2S    | 428  |
| ADV = 500ms | 1430 |



## WiFi Low Power：

### Test DTIM:

1.Reset board

2.Connect wifi, and got ip.

DTIM = 1: Enter the command "tickless 1" in the command line.

DTIM = 3: Enter the command "tickless 3" in the command line.

DTIM = 10: Enter the command "tickless 10" in the command line.

### WiFi Benchmark：

| Mode       | uA  |
| :--------- | :-- |
| Deep sleep | 67  |
| DTIM = 1   | 810 |
| DTIM = 3   | 319 |
| DTIM = 10  | 154 |


## BLE And WiFi Dual Low Power
BLE and Wi-Fi dual low power mode, which can maintain Wi-Fi keep-alive while also maintaining BLE advertising and BLE connection.

### Test Step

1.Reset board
2.Connect wifi, and got ip.
3.Enter ADV connectable mode: Enter the command "ble\_start\_adv 0 0 0x320 0x320" in the command line.
4.DTIM = 10: Enter the command "tickless 10" in the command line.
