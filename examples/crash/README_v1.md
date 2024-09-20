# crash_v1

## Support only linux

## Support CHIP

|      CHIP        | Remark |
|:----------------:|:------:|
|qcc743/qcc744       |        |


## Compile

- qcc743/qcc744
- comment CONFIG_COREDUMP_V2, default is v1

```
make CHIP=qcc743 BOARD=qcc743dk
```

## tools

|      OS          | tools suffix|
|:----------------:|:-----------:|
|ubuntu            |-ubuntu      |


## perl environment

in linux, perl is POSIX standard environment.


## Run environment

|      OS          | env               |
|:----------------:|:-----------------:|
|ubuntu            |bash terminal      |


## Flash

```
make flash CHIP=chip_name COMX=xxx # xxx is your com name
```

## coredump in flash usage

1. Enter crash in cli, wait log print. At the same time, a coredump will be generated in the core partition.

2. Get bin format crash binary  (press CHIP_EN, click RESET and release CHIP_EN)

```
(qcc743dk)
../../tools/qcc74x_tools/QConn_Flash/QConn_Flash_Cmd-ubuntu --port your-serial-port --read --start 0x2f8000 --len 0x80000 --file crash.bin
mv ../../tools/qcc74x_tools/QConn_Flash/crash.bin .

(qcc744dk)
../../tools/qcc74x_tools/QConn_Flash/QConn_Flash_Cmd-ubuntu --port your-serial-port --read --start 0x6e0000 --len 0x480000 --file crash.bin
mv ../../tools/qcc74x_tools/QConn_Flash/crash.bin .
```

3. Start bugkiller server

```
../../tools/crash_tools/v1/bugkiller_linux_amd64 -d crash.bin -e build/build_out/crash_qcc743.elf -p 1234
```

4. Start debug

comment `restore build/build_out/crash_qcc743.elf` and `restore crash.bin` in gdb.init file.
```
riscv64-unknown-elf-gdb -x gdb.init -se build/build_out/crash_qcc743.elf
```

## coredump in log usage

1. Enter crash in cli, wait log print.

2. Save log to 1.log file, and capture all coredump from 1.log file

```
../../tools/crash_tools/crash_capture.pl 1.log
[crash_capture] find 1 coredump
....
[crash_capture] find n coredump
```

3. Step 2 will generate n files, cordump1 ~ coredumpn, select the crash you are interested in.

```
../../tools/crash_tools/v1/bugkiller_dump_linux_amd64 -bin crash.bin -log ../../tools/crash_tools/output/coredump1
../../tools/crash_tools/v1/bugkiller_linux_amd64 -d crash.bin -e build/build_out/crash_qcc743.elf -p 1234
```

4. Start debug

comment `restore build/build_out/crash_qcc743.elf` and `restore crash.bin` in gdb.init file.
```
riscv64-unknown-elf-gdb -x gdb.init -se build/build_out/crash_qcc743.elf
```

## Note

1. please kill bugkiller_linux_amd64 process after debug.

2. Because of the driver of linux ch347, after a crash outputs a large amount of logs, the devices needs to be unplugged to make ch347 work correctly.
