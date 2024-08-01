# pds_io_wakeup


## Support CHIP

|      CHIP        | Remark |
|:----------------:|:------:|
|QCC743/QCC74x_undef       |        |

## Compile

- QCC743/QCC74x_undef

```
make CHIP=qcc743 BOARD=qcc743dk
```

## Flash

```
make flash CHIP=chip_name COMX=xxx # xxx is your com name
```

## Power ON

After powering on, you can see the following log. When you pull the GPIO16/17 pins low, you will see the system reboot, and the log is as follows.

```log
Build:18:27:07,Jul  1 2024                                                                                                                                                                                                                                    
Version of used components:                                                                                                                                                                                                                                   
        Version: component_version_sdk_2.0.32-8-gfc59d2ba-dirty                                                                                                                                                                                               
Current chip device version: 0                                                                                                                                                                                                                                
======== flash cfg ========                                                                                                    
flash size 0x00400000                                                                                                          
jedec id     0xC46016                                                                                                          
mid              0xC4                                                                                                          
iomode           0x04                                                                                                          
clk delay        0x01                                                                                                          
clk invert       0x01                                                                                                          
read reg cmd0    0x05                                                                                                          
read reg cmd1    0x35                                                                                                          
write reg cmd0   0x01                                                                                                          
write reg cmd1   0x31      
qe write len     0x01
cread support    0x01
cread code       0xA0
burst wrap cmd   0x77
===========================
dynamic memory init success, ocram heap size = 434 Kbyte 
sig1:ffffffff        
sig2:0000f32f        
cgen1:9ffffffd       
enter hbn mode       
```