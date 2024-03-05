# efuse_getinfo


## Support CHIP

|      CHIP        | Remark |
|:----------------:|:------:|
|QCC74x_undef/QCC74x_undef       |        |
|QCC74x_undef/QCC74x_undef/QCC74x_undef |        |
|QCC743/QCC74x_undef       |        |
|QCC74x_undef             |        |

## Compile

- QCC74x_undef/QCC74x_undef

```
make CHIP=qcc74x_undef BOARD=qcc74x_undefdk
```

- QCC74x_undef/QCC74x_undef/QCC74x_undef

```
make CHIP=qcc74x_undef BOARD=qcc74x_undefdk
```

- QCC743/QCC74x_undef

```
make CHIP=qcc743 BOARD=qcc743dk
```

- QCC74x_undef

```
make CHIP=qcc74x_undef BOARD=qcc74x_undefdk CPU_ID=m0
make CHIP=qcc74x_undef BOARD=qcc74x_undefdk CPU_ID=d0
```

## Flash

```
make flash CHIP=chip_name COMX=xxx # xxx is your com name
```