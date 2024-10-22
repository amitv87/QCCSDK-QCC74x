# SDIO_WiFi

## Device

### Support CHIP

|      CHIP        | Remark |
|:----------------:|:------:|
|QCC743/QCC74x_undef       |        |

### Device Compile

- QCC743/QCC74x_undef

```
# Use the compile script make, default mode is ldo 1.1v, and lpfw without log:
make
# Note: if script parameters are modified, make sure to run 'make clean' before recompiling.
```

### Flash

```
make flash CHIP=chip_name COMX=xxx # xxx is your com name
```

### Memory

| Start Address   | Size (bytes)  | Description                                                 |      |
| --------------- | ------------- | ------------------------------------------------------------ | ---- |
| 0x23010000      | 0x16800 (90K) | Wi-Fi (bss, common, data, etc.). If CONFIG_SDIO_HIGH_PERFORMANCE is defined, <br /> more RAM will be used for SDIO TX to improve SDIO TX throughput. |      |
| 0x23026800      | 0x9800 (38K)  | Shared by LP firmware and SDIO TX buffer                     |      |
| 0x23030000      | 0x8000 (32K)  | If Bluetooth is initialized, this will be used as the EM region for Bluetooth.  |      |

## Host

### Host Compile

Here's an example using Raspberry Pi (which requires USB boot configuration). Copy the components/net/netbus/host_router folder to the host system (such as Raspberry Pi), and then refer to ./qcc74x_install to complete the driver compilation and insmod.

### Host Control Program

After QCC743/8 is connected to the host via SDIO and powered on, a **device ready** log will be printed, indicating a successful handshake between the host and the device. At this point, you can use qcctl to send control commands. Enter the userspace directory:

(1) Connect to a router

```bash
qcctl connect_ap <ssid> [password]
```

Where SSID is a required parameter, and password is optional. For unencrypted APs, no password parameter is required.

(2) Disconnect from the router

```bash
qcctl disconnect_ap
```

(3) Get the connection status with the router

```bash
qcctl get_link_status
```

(4) Request the device to perform a Wi-Fi scan

```bash
qcctl wifi_scan
```

After executing this command, use the wifi_scan_results command to retrieve the scan results. The recommended operation sequence is: wifi_scan; sleep 6s; wifi_scan_results. Note that scan results are valid for 15 seconds.

(5) Retrieve Wi-Fi scan results

```bash
qcctl wifi_scan_results
```

(6) Start an AP

```bash
qcctl start_ap <ssid> [password]
```

Where SSID is the SSID of the AP to be started. To start an unencrypted AP, do not specify a password; to start an encrypted AP, specify a password that is 8 to 63 common characters.

This pass-through scheme does not support AP/STA coexistence, only time-division multiplexing of STA/AP. Therefore, the user should ensure that the AP is closed before connecting to the router, and disconnected from the router before starting the AP.

If the Firmware DHCP Server is enaqced, after starting the AP, configure the Linux interface address:

```bash
ifconfig qceth0 192.168.169.1
```

(7) Stop the AP

```bash
qcctl stop_ap
```

(8) OTA

```bash
qcctl ota <path to ota bin>
```

The only required parameter is the OTA bin. The OTA bin is generated during the QCC743 build, for example: sdio_wifi_qcc743.bin.ota and sdio_wifi_qcc743.xz.ota located in the examples/wifi/sdio_wifi/build/build_out directory. The former is the uncompressed OTA bin, and the latter is the compressed version.

(9) User-defined command demo

```bash
qcctl user_ext
```

This command demonstrates how the host and device transmit user-defined commands. Users can build on this to add features (this command is not recommended for use).

```bash
qcctl user_ext_rsp
```

This command demonstrates the host actively sending commands to the device, with the device replying back to the host. Users can extend the application functionality based on this.

```bash
qcctl user_ext_no_rsp
```

This command demonstrates the host actively sending commands to the device, with the device printing the received content without replying to the host. Users can extend the application functionality based on this.

The above three commands correspond to QC_CMD_USER_EXT, QC_CMD_USER_EXT_RSP, and QC_CMD_USER_EXT_NO_RSP. In the demo, the first two commands have the same effect, and it is recommended that customers use the latter two commands for application extension. All three commands are initiated by the host to the device. If the device needs to actively send messages to the host, the device can use the rnms_user_send_extension function.

**PS:**
For host-side application extensions, modify userspace/msg_handlers.c.

For device-side application extensions, modify dev_net_mgmr/src/rnm_user_ext.c.

## **DHCP**

If DHCP is handled by the Firmware, the Host needs to start the qcctld program to obtain the IP address information after connecting to the router. The command format is as follows:

```bash
sudo qcctld /fullpath/to/sta.sh
```

Where sta.sh is the script executed by qcctld upon receiving the IP address information, and its default function is to set the network interface address. Users may need to configure the driver to load and start qcctld automatically at boot. Note that qcctld must be started after the driver is loaded, and the script path should be the full path (qcc74x_install will execute the above command by default).

## About Dual Stack Splitting

There are two processing paths for splitting, one is eth_input_hook() and the other is the lwip input hook.

In eth_input_hook(), EAPOL (QCC743) is filtered for local processing, which is necessary for connecting to the router, so users should not modify this. ARP packets are copied and input into both the device and host protocol stacks, which users should generally not modify either. Users can add their own filtering logic to decide which packets are input to the device and which are input to the host.

The input hook in lwip mainly serves to achieve automatic splitting of input packets. The method is to first input the packets received by Wi-Fi into lwip, and if lwip does not have the corresponding connection, then input them into the host. In this way, users can avoid configuring eth_input_hook() to distinguish traffic to both sides via ports.

eth_input_hook() will be executed first, followed by the lwip input hook. Using ports to distinguish traffic in eth_input_hook() may result in some performance improvements.

Care should be taken to avoid situations where eth_input_hook() decides to input packets to both sides, and then the lwip hook decides to input the packets to the host as well. In such cases, the device firmware may crash. This situation is unlikely to occur in general use cases.

The main risk of splitting is port collisions between socket applications on both sides: if both sides are using the same port to connect to the same server (IP:PORT) as clients, the packets will be input to the device. If both sides are acting as servers bound to the same port, the packets will be input to the device, giving priority to the embedded side. If the server (IP:PORT) is different on both sides, there is no issue. The risk can be mitigated by binding the sockets on both sides to different ports.

