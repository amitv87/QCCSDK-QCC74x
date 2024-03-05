# Codec [English](README.md)

**特别说明**: 截止到的更新文档为止，已经支持mic、speaker功能，回环待后续完善

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

### 测试mic功能

```
qcc74x />codec_mic
```

### 测试speaker功能

```
qcc74x />codec_speaker
```

### 测试loop（回环）功能

```
qcc74x />codec_loop
```
