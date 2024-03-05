# Codec [中文](README_zh.md)

**Special Note**: As of the latest update to this documentation, support for mic and speaker functionality is available. Loopback is pending further improvement.

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

### Test Mic Function

```
qxx74x />codec_mic
```

### Test Speaker Function

```
qxx74x />codec_speaker
```

### Test Loop (Loopback) Function

```
qxx74x />codec_loop
```