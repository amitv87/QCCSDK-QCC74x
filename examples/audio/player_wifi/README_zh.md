# Player Wifi Case[English](README.md)

**特别说明**: 播放音频流，至于流媒体的配置可以参考multimedia_user.h，通过修改multimedia_user.h可以完成对流媒体功能的裁剪

## 芯片支持列表

|      芯片        | 备注    |
|:----------------:|:------:|
|QCC743/QCC74x_undef       |        |
|QCC74x_undefp/QCC74x_undef      |        |

## 编译

- QCC743/QCC74x_undef

```
make
```

## 烧录

```
make flash CHIP=qcc743 COMX=/dev/ttyACM0
```

## 运行

### 在线播放流媒体

```
qcc74x />wifi_sta_connect <ssid> <password>
qcc74x />qcc74x />smta play http://iot-du-home-test.bj.bcebos.com/test/bp_32k_mono_32kbps_03m53s.mp3
```
