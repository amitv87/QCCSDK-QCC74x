# Player Wifi Case[中文](README_zh.md)

**Special Note**: Streaming audio is supported. For streaming media configuration, please refer to `multimedia_user.h`. You can tailor the streaming media functionality by modifying `multimedia_user.h`.

## Chip Support List

|      Chip        | Remark |
|:----------------:|:------:|
| qcc743/qcc744      |        |

## Compilation

- qcc743/qcc744

```
make
```

## Flashing

```
make flash CHIP=qcc743 COMX=/dev/ttyACM0
```

## Running

### Playing Online Streaming Media

```
qxx74x />wifi_sta_connect <ssid> <password>
qxx74x />smta play http://iot-du-home-test.bj.bcebos.com/test/bp_32k_mono_32kbps_03m53s.mp3
```
