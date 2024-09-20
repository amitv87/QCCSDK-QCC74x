# crash_v2

## Support linux/win/macos

## Support CHIP

|      CHIP        | Remark |
|:----------------:|:------:|
|qcc743/qcc744       |        |

## Compile

- qcc743/qcc744

```
enable set(CONFIG_COREDUMP_V2 1) in proj.conf.

make CHIP=qcc743 BOARD=qcc743dk
```


## tools

|      OS          | tools suffix|
|:----------------:|:-----------:|
|ubuntu            |-ubuntu      |
|macos             |-macos       |
|windows           |.exe         |


## perl environment

in macos/linux, perl is POSIX standard environment, in windows you can install by https://strawberryperl.com/


## Run environment

|      OS          | env               |
|:----------------:|:-----------------:|
|ubuntu            |bash terminal      |
|macos             |bash terminal      |
|windows           |powershell terminal|


## Flash

```
make flash CHIP=chip_name COMX=xxx # xxx is your com name
```

## coredump in flash usage

1. Enter crash in cli, wait log print. At the same time, a coredump will be generated in the core partition.

2. Get bin format crash binary (press CHIP_EN, click RESET and release CHIP_EN)

```
(qcc743dk)
../../tools/qcc74x_tools/QConn_Flash/QConn_Flash_Cmd-ubuntu --port your-serial-port --read --start 0x2f8000 --len 0x80000 --file crash.bin
mv ../../tools/qcc74x_tools/QConn_Flash/crash.bin .

(qcc744dk)
../../tools/qcc74x_tools/QConn_Flash/QConn_Flash_Cmd-ubuntu --port your-serial-port --read --start 0x6e0000 --len 0x480000 --file crash.bin
mv ../../tools/qcc74x_tools/QConn_Flash/crash.bin .
```

3. Start gdb server emulator

```
../../tools/crash_tools/v2/bugkiller-ubuntu
```

4. Start debug

```
riscv64-unknown-elf-gdb -x gdb.init
```

## coredump in log usage

1. Enter crash in cli, wait log print.

2. Save log to 1.log file, and capture all coredump from 1.log file

```
../../tools/crash_tools/crash_capture_v2.pl 1.log
mv ../../tools/crash_tools/output/coredump1 crash.bin
```

3. Start gdb server emulator

```
../../tools/crash_tools/v2/bugkiller-ubuntu
```


4. Start debug

```
riscv64-unknown-elf-gdb -x gdb.init
```

## Note

1. Because of the driver of linux ch347, after a crash outputs a large amount of logs, the devices needs to be unplugged to make ch347 work c
orrectly.
