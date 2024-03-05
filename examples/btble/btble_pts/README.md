# btble_cli


## Support CHIP

|      CHIP        | Remark |
|:----------------:|:------:|
|QCC743/QCC74x_undef       |        |
|QCC74x_undef             |  Only for M0 CPU      |

## Compile

- QCC743/QCC74x_undef

```
make CHIP=qcc743 BOARD=qcc743dk
```

- QCC74x_undef

```
make CHIP=qcc74x_undef BOARD=qcc74x_undefdk CPU_ID=m0
```

## Flash

```
make flash CHIP=chip_name COMX=xxx # xxx is your com name
```