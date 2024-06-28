## Test mode and testing methods：

### Deep sleep ：

1.Reset board 
2.Enter the command "tickless" in the command line.

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

### Benchmark：

The following test data reduced the power of BLE TX. The modification is as follows: Change "pwr\_table\_ble = <13> to pwr\_table\_ble = <0>;" in the file "bsp/board/qcc743\_lp\_dk/config/qcc74x\_factory\_params\_IoTKitA\_auto.dts".

| Mode        | uA   |
| :---------- | :--- |
| Deep sleep  | 50   |
| ADV = 2S    | 428  |
| ADV = 500ms | 1430 |

