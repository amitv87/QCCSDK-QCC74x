
# Guide for MFG Image Switching

This guide demonstrates how to switch to an autoboot version of the MFG image. Follow the steps outlined below, using the example code provided in `examples/wifi/sta/wifi_ota_by_http`. Before proceeding, ensure the `build` directory under `examples/wifi/sta/wifi_ota_by_http` has been cleared.

## Switching to Autoboot MFG Image

To replace the default MFG image with an autoboot image, use the following commands:

```bash
cd bsp/board/qcc743dk/config
cp ../builtin_imgs/mfg_qcc743_gu_34c572ca0_autoboot_v2.49.bin mfg_qcc743_gu_34c572ca0_v2.49.bin
```

The above commands will affect the MFG image in the `build_out` directory, setting it to the autoboot version. This version automatically runs before the normal image upon power-up.

## Recompiling the Example

Recompile the example located at `examples/wifi/sta/wifi_ota_by_http`.

## Flashing the Example

Flash the example using your preferred flashing tool:

```bash
# Replace this line with your flashing tool command
```

## Restarting the Board

After restarting the board, it will automatically enter the MFG mode.

## Switching Back to Normal Image

In the MFG mode, type the `ATSC` command and press Enter. You will see a log with the keyword "Erase", indicating that the default image has been switched back to the normal image:

```shell
[mfg fw] ATSC OK
Erase Flag 00212000,0,0,ffffffff Success
```

## Switching Back to MFG Image
use `mfg` command to switch back to MFG image, if MFG partition is NOT eraed and still valid.
```shell
qcc74x />help
shell commands list:
free
memtrace
help
ps
rate
rc
iperf
set_ipv4
wifi_raw_send
wifi_ap_conf_max_sta
wifi_ap_mac_get
wifi_ap_stop
wifi_ap_start
wifi_sta_del
wifi_sta_list
wifi_sta_info
wifi_sta_ps_off
wifi_sta_ps_on
wifi_sta_autoconnect_disable
wifi_sta_autoconnect_enable
wifi_sta_mac_get
wifi_sta_ssid_passphr_get
wifi_sta_channel
wifi_sta_rssi
wifi_state
wifi_sniffer_off
wifi_sniffer_on
lwip
wifi_sta_disconnect
wifi_sta_connect
wifi_scan
phy
hello
mfg
sysver
reboot
wifi_ota_test

qcc74x />mfg
FG Version: 2.49_ldo

Build Date: Dec 15 2023
Build Time: 16:24:22
------------------------------------------------------------
flash init ret 0
[IRQ] Clearing and Disable all the pending IRQ...
```

## Restart the Board Again

Upon restarting the board, you will enter the normal image mode. You can now perform OTA testing as described in `README.md` located at `examples/wifi/sta/wifi_ota_by_http`.
