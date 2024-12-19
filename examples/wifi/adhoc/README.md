# wifi6 adhoc case

## Support CHIP

|      CHIP        | Remark |
|:----------------:|:------:|
|qcc743/qcc744       |        |

## Compile

- qcc743/qcc744

```bash
make CHIP=qcc743 BOARD=qcc743dk
```

## Flash

```bash
make flash COMX=xxx ## xxx is your com name
```

## Ad-hoc mode example
Prepare two qcc743 Board, namely Board A and Board B.

On both Board A and B, start ad-hoc mode, `-c 14` means ad-hoc mode will work on channel 14:
```bash
qxx74x />wifi_adhoc_start -c 14

```
start to send packets for testing on Board A, it will send 10 data packets:
```bash
qxx74x />wifi_adhoc_send

```
You will see the printout of the data packets on the serial port log of board B.


## Test status for Requirements
| Requirements                                                | Test Status |
|------------------------------------------------------------|-------------|
| Connectionless communication for the QCOM Proprietary mesh network | Tested      |
| Transmit/Receive 802.11 Data Frames in the 4-Address Frame Format | Tested      |
| IEEE 802.11b only                                         | Tested      |
| 2.4GHz Channels 1-13                                     | Tested      |
| 2.4GHz Channel 14                                        | Tested      |
| Fix Transmission rate (11b) for Adhoc network only        | Tested     |
| Set number of transmission retries (11b) for Adhoc network only | Tested     |
| Get the number of frames remaining in each Tx queue       | Tested     |
| Set/Get Tx power                                         | Tested     |
| Set/Get Operation Channel                                | Tested     |
| Set/Get Transmission rate (11b) for Adhoc network        | Tested    |
| Set/Get number of transmission retries (11b) for Adhoc network only | Tested     |
| Set/Get RTS threshold                                    | Tested     |
| Set/Get CWmin, CWmax, and AIFS for each Tx Queue          | Tested     |
| Get the number of frames remaining in each Tx queue      | Tested     |
| Data Frame Tx/Rx for the proprietary adhoc mode           | Tested     |



## API Reference
```c
int wifi_mgmr_adhoc_start(const wifi_mgmr_adhoc_start_params_t *config)
```
---
Enable Ad-hoc mode
### Parameters
- **config**: Pointer to configuration of enable adhoc mode

### Return Value
- 0: succeed

### Example
Please refer to demo in examples/wifi/adhoc/wifi_adhoc.c

```c
int wifi_mgmr_adhoc_stop(void)
```
---
Disable Ad-hoc mode

### Return Value
- 0: succeed

### Example
Please refer to demo in examples/wifi/adhoc/wifi_adhoc.c

```c
int wifi_mgmr_adhoc_pkt_send(const wifi_mgmr_adhoc_pkt_params_t *config)
```
---
Send Ethernet frame in Ad-hoc mode
### Parameters
- **config**: Pointer to configuration of sending Ethernet frame

### Return Value
- 0: succeed
- -1: failed to send

### Example
Please refer to demo in examples/wifi/adhoc/wifi_adhoc.c

```c
int wifi_mgmr_set_tx_queue_params(int queue, int aifs, int cw_min,
                           int cw_max, int burst_time)
```
---
Set CWmin, CWmax, AIFS and burst_time for specified Tx Queue
### Parameters
- **queue**: 0: AC_VO
             1: AC_VI
             2: AC_BE
             3: AC_BK
- **aifs**: Arbitration InterFrame Space Number
- **cw_min**: Contention Window minimum
- **cw_max**: Contention Window maximum
- **burst_time**: txop = (burst_time * 100 + 16) / 32

### Return Value
- 0: succeed
- -1: failed to set

```c
int wifi_mgmr_get_tx_queue_params(uint8_t queue, uint8_t *aifsn, uint16_t *cwmin, uint16_t *cwmax)
```
---
Get CWmin, CWmax, and AIFS for specified Tx Queue
### Parameters
- **queue**: 0: AC_VO
             1: AC_VI
             2: AC_BE
             3: AC_BK
- **aifs**: Pointer to get arbitration InterFrame Space Number
- **cw_min**: Pointer to get contention Window minimum
- **cw_max**: Pointer to get contention Window maximum

### Return Value
- 0: succeed
- -1: failed to get

```c
int wifi_mgmr_get_remaining_tx(uint8_t *tx0_cnt, uint8_t *tx1_cnt, uint8_t *tx2_cnt, uint8_t *tx3_cnt)
```
---
Get the number of frames remaining in each Tx queue
### Parameters
- **tx0_cnt**: Pointer to get the number of frames remaining in AC_BK queue
- **tx1_cnt**: Pointer to get the number of frames remaining in AC_BE queue
- **tx2_cnt**: Pointer to get the number of frames remaining in AC_VI queue
- **tx3_cnt**: Pointer to get the number of frames remaining in AC_VO queue

### Return Value
- 0: succeed


```c
int8_t wifi_mgmr_adhoc_set_rate(uint8_t rate)
```
---
Set transmission rate (11b) for Adhoc network
### Parameters
- **rate**: 0: 1Mbps
            1: 2Mbps
            2: 5.5Mbps
            3: 11Mbps

### Return Value
- 0: succeed


```c
int8_t wifi_mgmr_adhoc_set_rate(uint8_t rate)
```
---
Set transmission rate (11b) for Adhoc network
### Parameters
- **rate**: 0: 1Mbps
            1: 2Mbps
            2: 5.5Mbps
            3: 11Mbps

### Return Value
- 0: succeed

```c
uint8_t wifi_mgmr_adhoc_get_rate(void);
```
---
Get transmission rate (11b) for Adhoc network

### Return Value
- 0: 1Mbps
- 1: 2Mbps
- 2: 5.5Mbps
- 3: 11Mbps


```c
int8_t wifi_mgmr_adhoc_set_rts_thrshold(uint8_t rts_thrshold)
```
---
Set RTS threshold for Adhoc network
### Parameters
- **rts_thrshold**: value of RTS Threshold

### Return Value
- 0: succeed


```c
uint8_t wifi_mgmr_adhoc_get_rts_thrshold(void);
```
---
Get RTS threshold for Adhoc network

### Return Value
-  value of RTS Threshold

```c
int8_t wifi_mgmr_adhoc_set_tx_power(int8_t tx_power)
```
---
Set power level used to transmit for Adhoc network
### Parameters
- **tx_power**: 0x80: -128 dBm
                0xff: -1 dBm
                0x00: 0 dBm
                0x01: 1 dBm
                0x7F: 127 dBm

### Return Value
- 0: succeed

```c
int8_t wifi_mgmr_adhoc_get_tx_power(void);
```
---
Get power level used to transmit for Adhoc network
### Return Value
- value of power level

```c
int8_t wifi_mgmr_adhoc_set_retry_limit(uint8_t retry_limit)
```
---
Set retry limit used to transmit for Adhoc network
### Parameters
- **retry_limit**: value of retry limit
### Return Value
- 0: succeed


```c
uint8_t wifi_mgmr_adhoc_get_retry_limit(void)
```
---
Get retry limit used to transmit for Adhoc network
### Return Value
- value of retry limit


```c
uint8_t wifi_mgmr_adhoc_get_channel(void)
```
---
Get channel used to transmit for Adhoc network
### Return Value
- value of channel