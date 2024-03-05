.. _wifi6_api:

WIFI6
=============
API
------------------
wifi_sta_connect
^^^^^^^^^^^^^^^^
- Command Line Entry

.. code-block:: C

    void wifi_connect_cmd(int argc, char **argv);

- API

.. code-block:: C

    int wifi_mgmr_sta_connect(const wifi_mgmr_sta_connect_params_t *config);

    Brief:
    Station connects to AP.

    Parameters:
    typedef struct {
      /* SSID of target AP */
      char ssid[MGMR_SSID_LEN];
      /* password of target AP */
      char key[MGMR_KEY_LEN];
      /* BSSID of target AP */
      char bssid_str[MGMR_BSSID_LEN];
      /* AKM of target AP, must be in uppercase,
      * OPEN: For open AP
      * WPA: For AP with WPA/PSK security (pre WPA2)
      * RSN or WPA2: For AP with WPA2/PSK security
      * SAE or WPA3: For AP with WPA3/PSK security
      */
      char akm_str[MGMR_BSSID_LEN];
      /* two frequencies can be specified on which AP will be scanned*/
      uint16_t freq1;
      uint16_t freq2;
      /* configurations for protected management frames */
      uint8_t pmf_cfg;
      /* whether to use the dhcp server provided by AP */
      uint8_t use_dhcp;
      /* listen interval for beacon */
      uint8_t listen_interval;
      /* 0 – quick scan and connect to the first matched AP
      *1 – scan on all channels, which is the default option
      */
      uint8_t scan_mode;
      /* 0 – normal connect, 1 – quick connect */
      uint8_t quick_connect;
      /* timeout of connection process */
      int timeout_ms;
    };

- Return

  * 0 on success, a negative number otherwise.

wifi_scan
^^^^^^^^^^^^^^^^

- Command Line Entry

.. code-block:: C

    void wifi_scan_cmd(int argc, char **argv);

- API

.. code-block:: C

    int wifi_mgmr_sta_scan(const wifi_mgmr_scan_params_t *config)

    Brief:
    Start the process of scan.

    Parameters:
    typedef struct {
      /*
      * if specified, only scan the AP whose SSID matches ssid_array
      */
      uint8_t ssid_array[MGMR_SSID_LEN];
      /*
      * if specified, only scan the AP whose BSSID matches bssid
      *c/
      uint8_t bssid[6];
      /* indicate channels on which scan is performed */
      uint8_t channels[MAX_FIXED_CHANNELS_LIMIT];
      /* duration in microseconds for which channel is scanned, default 220000 */
      uint32_t duration;
      } wifi_mgmr_scan_params_t;

- Return

  * 0 on success, a negative number otherwise.

ping
^^^^^^^^^
- Command Line Entry

.. code-block:: C

    components/net/lwip/lwip_apps/ping/ping.c:
    void ping_cmd(char *buf, int len, int argc, char **argv);

- API

  * Please refer to LWIP raw API.

wifi_sta_info
^^^^^^^^^^^^^^^
- Command Line Entry

.. code-block:: C

    void wifi_sta_info_cmd(int argc, char **argv);

- API

.. code-block:: text

    int wifi_sta_ip4_addr_get(uint32_t *addr, uint32_t *mask, uint32_t *gw, uint32_t *dns);

    Brief:
    Get IP information of the station.

    Parameters:
    addr - the IP address
    mask – network mask
    gw – gateway address
    dns – DNS server address

    Return:
    0 on success, a negative number otherwise.

.. code-block:: text

    int wifi_mgmr_sta_rssi_get(int *rssi);

    Brief:
    Get wifi RSSI.

    Parameters:
    rssi – WiFi RSSI

    Return:
    0 on success, a negative number otherwise.

.. code-block:: text

    int wifi_mgmr_tpc_pwr_get(rf_prw_table_t *power_table);

    Brief:
    get wifi power table

    Parameters:
    typedef struct rf_pwr_table {
        int8_t pwr_11b[4];
        int8_t pwr_11g[8];
        int8_t pwr_11n_ht20[8];
        int8_t pwr_11n_ht40[8];
        int8_t pwr_11ac_vht20[10];
        int8_t pwr_11ac_vht40[10];
        int8_t reserved[10];
        int8_t pwr_11ax_he20[12];
        int8_t pwr_11ax_he40[12];
        int8_t reserved2[12];
        int8_t reserved3[12];
    } rf_pwr_table_t;

    Return:
    0 on success, a negative number otherwise.

wifi_state
^^^^^^^^^^^^
- Command Line Entry

.. code-block:: C

    void cmd_wifi_state_get(int argc, char **argv);

- API

.. code-block:: C

    int wifi_mgmr_state_get(void);

    Brief:
    Dump station/ap information.

    Return:
    0 on success, a negative number otherwise.

wifi_sta_rssi
^^^^^^^^^^^^^^
- Command Line Entry

.. code-block:: C

    void cmd_wifi_state_get(int argc, char **argv);

- API

.. code-block:: C

    Please refer to wifi_sta_info

wifi_sta_disconnect
^^^^^^^^^^^^^^^^^^^^^^^^
- Command Line Entry

.. code-block:: C

    void wifi_disconnect_cmd(int argc, char **argv);

- API

.. code-block:: C

    int wifi_sta_disconnect(void);

    Brief:
    Station disconnects from its AP.

- Return

  * 0 on success, a negative number otherwise.

wifi_ap_start
^^^^^^^^^^^^^^^^^^^
- Command Line Entry

.. code-block:: C

    void wifi_mgmr_ap_start_cmd(int argc, char **argv);

- API

.. code-block:: C

    int wifi_mgmr_ap_start(const wifi_mgmr_ap_params_t *config);

    Brief:
    Start wifi AP.

    Parameters:
    typedef struct {
    	/* the SSID of the AP */
    	char *ssid;
    	/* if both key and akm are NULL, the default key is 12345678 */
    	char *key;
    	/* OPEN/WPA/WPA2 */
    	char *akm;
    	/* default channel is 6 */
    	uint8_t channel;
    	/* channel type */
    	uint8_t type;
    	/* whether start dhcpd */
    	bool use_dhcpd;
        /* dhcpd pool start */
    	int start;
        /* dhcpd pool limit */
    	int limit;
        /* AP IP address */
    	uint32_t ap_ipaddr;
        /* AP IP network mask */
    	uint32_t ap_mask;
        /* STA max inactivity when connected */
    	uint32_t ap_max_inactivity;
    	/* whether use hidden SSID */
    	bool hidden_ssid;
    	/* whether enable isolation */
    	bool isolation;
    };

- Return

  * 0 on success, a negative number otherwise.

wifi_ap_stop
^^^^^^^^^^^^^^^
- Command Line Entry

.. code-block:: C

    void wifi_mgmr_ap_stop_cmd(int argc, char **argv);

- API

.. code-block:: C

    int wifi_mgmr_ap_stop(void);

    Brief:
    Stop wifi AP.

- Return

  * 0 on success, a negative number otherwise.

wifi_ap_conf_max_sta
^^^^^^^^^^^^^^^^^^^^^^^^^^^
- Command Line Entry

.. code-block:: C

    void cmd_wifi_ap_conf_max_sta(int argc, char **argv);

- API

.. code-block:: text

    int wifi_mgmr_conf_max_sta(uint8_t max_sta_supported);

    Brief:
    Set the maximum number of station.

    Parameters:
    max_sta_supported – the maximum number of station.

- Return

  * 0 on success, a negative number otherwise.

wifi_ap_mac_get
^^^^^^^^^^^^^^^^^^^^
- Command Line Entry

.. code-block:: C

    void cmd_wifi_ap_mac_get(int argc, char **argv);

- API

.. code-block:: text

    int wifi_mgmr_ap_mac_get(uint8_t mac[6]);

    Brief:
    Get mac address of AP.

    Parameters:
    mac – MAC address of the AP.

- Return

  * 0 on success, a negative number otherwise.

wifi_sta_list
^^^^^^^^^^^^^^
- Command Line Entry

.. code-block:: C

    void wifi_ap_sta_list_get_cmd(int argc, char **argv);

- API

.. code-block:: text

    int wifi_mgmr_ap_sta_info_get(struct wifi_sta_basic_info *sta_info, uint8_t idx);

    Brief: get basic information of station.

    Parameters:
    sta_info:
    typedef struct wifi_sta_basic_info {
        uint8_t sta_idx;
        uint8_t is_used;
        uint8_t sta_mac[6];
        uint16_t aid;
    } wifi_sta_basic_info_t;

    idx – the index of station

- Return

  * 0 on success, a negative number otherwise.

wifi_sniffer_on
^^^^^^^^^^^^^^^^^^^^
- Command Line Entry

.. code-block:: C

    void cmd_wifi_sniffer_on(int argc, char **argv);

- API

.. code-block:: text

    Int wifi_mgmr_sniffer_enable(wifi_mgmr_sniffer_item_t sniffer_item);

    Brief:
    Enable wifi sniffer.

    Parameters:
    sniffer_item –
    typedef struct wifi_mgmr_sniffer_item {
        /* interface index */
        char *itf;
        /* channel type */
        uint8_t type;
        /* frequency for Primary 20MHz channel (in MHz) */
        uint16_t prim20_freq;
        /* frequency center of the contiguous channel or center of primary 80+80 (in MHz) */
        uint16_t center1_freq;
        /* frequency center of the non-contiguous secondary 80+80 (in MHz) */
        uint16_t center2_freq;
        /* frame received callback. */
        void *cb;
        /* parameter for the monitor callback. */
        void *cb_arg;
    } wifi_mgmr_sniffer_item_t;

- Return

  * 0 on success, a negative number otherwise.

wifi_sniffer_off
^^^^^^^^^^^^^^^^^^^^^
- Command Line Entry

.. code-block:: C

    void cmd_wifi_sniffer_off(int argc, char **argv);

- API

.. code-block:: C

    int wifi_mgmr_sniffer_disable(wifi_mgmr_sniffer_item_t sniffer_item);

    Brief:
    Disable wifi sniffer.

    Parameters:
    Please refer to wifi_sniffer_on

- Return

  * 0 on success, a negative number otherwise.

CLI Commands
-------------

wifi_sta_connect
^^^^^^^^^^^^^^^^^^^^

Used to connect to AP. After successful connection, the assigned IP address will be printed out.

- The first parameter represents the ssid
- The second parameter represents pwd

wifi_scan
^^^^^^^^^^^^^^^^^^^^

Used to scan AP, no parameters required

ping
^^^^^^^^^^^^^^^^^^^^

For pinging the network

- Fill in the ip address or domain name address in the first parameter

wifi_sta_info
^^^^^^^^^^^^^^^^
This command is used to query the status information of the STA (Station), such as RSSI (Received Signal Strength Indicator), IP address, and power table, and it does not require any parameters.

wifi_state
^^^^^^^^^^^^^^^^
This command is used to query the status information of the connection to the AP (Access Point), and it does not require any parameters.

wifi_sta_rssi
^^^^^^^^^^^^^^^^^
This command is used to query the RSSI (Received Signal Strength Indicator) of the STA (Station), and it does not require any parameters.

wifi_sta_disconnect
^^^^^^^^^^^^^^^^^^^^
This command is used to disconnect from the AP, and it does not require any parameters.

wifi_ap_start
^^^^^^^^^^^^^^^
This command is used to initialize an AP.

- The first parameter represents the SSID (Service Set Identifier).
- The second parameter represents the key.
- The third parameter represents the channel.

For example: wifi_ap_start -s qcc74x -k qcc74x2016 -c 11

wifi_ap_stop
^^^^^^^^^^^^^^^^^
This command is used to close the AP, and it does not require any parameters.

wifi_ap_conf_max_sta
^^^^^^^^^^^^^^^^^^^^^^^
This command is used to set the maximum number of STA devices that can be connected.

- Fill in the maximum number of connections as the first parameter.

For example: wifi_ap_conf_max_sta 3

wifi_ap_mac_get
^^^^^^^^^^^^^^^^^^^
This command is used to obtain the MAC address of the AP, and it does not require any parameters.

wifi_sta_list
^^^^^^^^^^^^^^^^^^^
This command is used to query the information of the connected STA (Station) devices, and it does not require any parameters.

wifi_sniffer_on
^^^^^^^^^^^^^^^^^^^
This command is used to turn on the sniffer mode for network packet capturing.

- The first parameter represents the interface.
- The second parameter represents the frequency.

For example: wifi_sniffer_on -i wl1 -f 2462

wifi_sniffer_off
^^^^^^^^^^^^^^^^^^^^
This command is used to turn off sniffer mode, and it does not require any parameters.

Set PowerTable
-------------------

.. code-block:: c

    int wifi_mgmr_tpc_pwr_set(rf_pwr_table_t *power_table);

Function Parameters
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``power_table`` is a pointer to the ``rf_pwr_table_t`` structure, which contains the transmission power settings for different WiFi standards.

``rf_pwr_table_t`` Structure
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c

    typedef struct rf_pwr_table {
        int8_t pwr_11b[4];
        int8_t pwr_11g[8];
        int8_t pwr_11n_ht20[8];
        int8_t pwr_11n_ht40[8];
        int8_t pwr_11ac_vht20[10];
        int8_t pwr_11ac_vht40[10];
        int8_t reserved[10];
        int8_t pwr_11ax_he20[12];
        int8_t pwr_11ax_he40[12];
        int8_t reserved2[12];
        int8_t reserved3[12];
    } rf_pwr_table_t;

Structure Member
^^^^^^^^^^^^^^^^^^

- ``pwr_11b[4]``: Corresponds to 4 power levels (1Mbps, 2Mbps, 5.5Mbps, 11Mbps) for the 802.11b standard.
- ``pwr_11g[8]``: Corresponds to 8 power levels for the 802.11g standard.
- ``pwr_11n_ht20[8]``: Corresponds to 8 power levels for the 802.11n HT20 mode.
- Other members follow the same pattern.

.. warning::

   Before connecting to an Access Point (AP) or starting a micro Access Point (uAP), make sure to set the PowerTable. This is a crucial step to ensure that the device operates at the correct power levels.

Use the following function to set the PowerTable:

Example
^^^^^^^^

Here is an example code for setting the PowerTable.

.. code-block:: c

    rf_pwr_table_t power_table = {
        .pwr_11b = {20, 20, 20, 20},
        .pwr_11g = {18, 18, 18, 18, 18, 18, 16, 16},
        .pwr_11n_ht20 = {18, 18, 18, 18, 18, 16, 15, 15},
        .pwr_11n_ht40 = {18, 18, 18, 18, 18, 16, 15, 14},
        .pwr_11ac_vht20 = {18, 18, 18, 18, 18, 16, 15, 15, 15, 14},
        .pwr_11ac_vht40 = {18, 18, 18, 18, 18, 16, 15, 14, 14, 13},
        .reserved = {0},
        .pwr_11ax_he20 = {18, 18, 18, 18, 18, 16, 15, 15, 15, 14, 13, 13},
        .pwr_11ax_he40 = {18, 18, 18, 18, 18, 16, 15, 14, 14, 13, 12, 12},
        .reserved2 = {0},
        .reserved3 = {0}
    };
    wifi_mgmr_tpc_pwr_set(&power_table);

Retrieve PowerTable
-------------------------

To retrieve the current PowerTable settings, use the following function:

.. code-block:: c

    int wifi_mgmr_tpc_pwr_get(rf_pwr_table_t *power_table);

Function Parameter
^^^^^^^^^^^^^^^^^^^^^^^^^

- ``power_table``: A pointer to the ``rf_pwr_table_t`` structure that is used to receive the current power settings.

Country Code Management
---------------------------------

This section explains how to set and retrieve the country code for a WiFi device. The country code is used to determine the available WiFi channels.

Setting the Country Code
--------------------------------

Use the following function to set the country code:

.. code-block:: c

    int wifi_mgmr_set_country_code(char *country_code);

Function Parameter
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- ``country_code``: A pointer to a string representing the country code.

Retrieving the Country Code
----------------------------------

Use the following function to retrieve the currently set country code:

.. code-block:: c

    int wifi_mgmr_get_country_code(char *country_code);

Function Parameter
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- ``country_code``: A character array with enough space to receive the current country code.

Supported Country Codes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

When setting the country code for a WiFi device, it is important to note the range of country codes supported by the current API. The country code is primarily used to determine the number of available WiFi channels for the device in different countries or regions. Here are some supported country codes and their corresponding channel numbers.

List of Country Codes
^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. list-table::
   :widths: 25 75
   :header-rows: 1

   * - Country Code
     - Number of Channels
   * - CN
     - 13
   * - JP
     - 14
   * - US
     - 11
   * - EU
     - 13
