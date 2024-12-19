# hbn_acomp

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

## Test Environment

```
When measuring current, please connect GPIO13 and GPIO14 to static low level (GND) or high level (3V3).

When testing Acomp wake-up HBN mode, please toggle GPI013 or GPI014.
```