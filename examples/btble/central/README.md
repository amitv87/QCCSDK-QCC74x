# Peripheral

This sample description ble peripheral function with READ, WRITE/WRITE NO RESPONSE, INDICATE and NOTIFY properties. Can be linked and communicated by ble app.

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
make flash CHIP=chip_name COMX=xxx # xxx is your com name
```

This central example scans the ble device with the specified name and initiates a connection. After the connection is successful, the ble service discovery is performed. Enable the ccc of the first NOTIFY attribute, and execute the write operation to the first write attribute