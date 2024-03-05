# OpenThread command line example
OpenThread porting is over `components/wireless/lmac154` module. 

This is an example for OpenThread command line example.

## Support CHIP

|      CHIP        | Remark |
|:----------------:|:------:|
|QCC743/QCC74x_undef       |        |

## Compile
- QCC743/QCC74x_undef
    - Build for OpenThread FTD example
        ```shell
        make CHIP=qcc743 BOARD=qcc743dk OT_FTD=1
        ```
    
    - Build for OpenThread MTD example
        ```shell
        make CHIP=qcc743 BOARD=qcc743dk OT_FTD=0
        ```

- OpenThread feature options

  Please free modify OpenThread feature options in `openthread-core-proj-config.h`.

## Flash

```
make flash CHIP=chip_name COMX=xxx # xxx is your com name
```

## How to run

By default, command line example was built with SDK command set and uses `otc` to start OpenThread Command.

Prepare two boards with command line example programmed.

- Create an OpenThread with on board as below

  ```shell
  # Configure active dataset 
  qcc74x />otc dataset set active 0e080000000000000000000300000f35060004001fffe00208dead00beef00cafe0708fddead00beef0000051000112233445566778899aabbccddeeff030a4f70656e54687265616404109ce7b658f9eb6d53275154280792d3df0c0402a0f7f801026677
  dataset set active 0e080000000000000000000300000f35060004001fffe00208dead00beef00cafe0708fddead00beef0000051000112233445566778899aabbccddeeff030a4f70656e54687265616404109ce7b658f9eb6d53275154280792d3df0c0402a0f7f801026677
  Done
  
  # Show active dataset
  > qcc74x />otc dataset init active 
  dataset init active
  Done
  > qcc74x />otc dataset
  dataset
  Active Timestamp: 0
  Channel: 15
  Channel Mask: 0x07fff800
  Ext PAN ID: dead00beef00cafe
  Mesh Local Prefix: fdde:ad00:beef:0::/64
  Network Key: 00112233445566778899aabbccddeeff
  Network Name: OpenThread
  PAN ID: 0x6677
  PSKc: 9ce7b658f9eb6d53275154280792d3df
  Security Policy: 672 onrc 0
  Done
  
  # Start Thread network
  > qcc74x />otc ifconfig up
  ifconfig up
  Done
  > qcc74x />otc thread start
  thread start
  Done
  
  # After a while, Thread network created and becomes the leader
  > qcc74x />otc state
  state
  leader
  Done
  
  # Show IP addresses
  qcc74x />otc ipaddr
  ipaddr
  fdde:ad00:beef:0:0:ff:fe00:fc00
  fdde:ad00:beef:0:0:ff:fe00:8400
  fdde:ad00:beef:0:341c:558f:b78d:71f9
  fe80:0:0:0:7062:2b0c:b541:e9be
  Done
  
  ```

- Configure another board to attach the Thread network above

  ```shell
  # Configure active dataset 
  qcc74x />otc dataset set active 0e080000000000000000000300000f35060004001fffe00208dead00beef00cafe0708fddead00beef0000051000112233445566778899aabbccddeeff030a4f70656e54687265616404109ce7b658f9eb6d53275154280792d3df0c0402a0f7f801026677
  dataset set active 0e080000000000000000000300000f35060004001fffe00208dead00beef00cafe0708fddead00beef0000051000112233445566778899aabbccddeeff030a4f70656e54687265616404109ce7b658f9eb6d53275154280792d3df0c0402a0f7f801026677
  Done
  
  # Let this board attach to the Thread network
  > qcc74x />otc ifconfig up
  ifconfig up
  Done
  > qcc74x />otc thread start
  thread start
  Done
  
  # After a while, this board attaches and becomes router role.
  qcc74x />otc state
  state
  router
  Done
  
  # Ping IP address of the lead
  qcc74x />otc ping fdde:ad00:beef:0:0:ff:fe00:fc00
  ping fdde:ad00:beef:0:0:ff:fe00:fc00
  qcc74x />16 bytes from fdde:ad00:beef:0:0:ff:fe00:fc00: icmp_seq=1 hlim=64 time=14ms
  1 packets transmitted, 1 packets received. Packet loss = 0.0%. Round-trip min/avg/max = 14/14.0/14 ms.
  Done
  ```

  
