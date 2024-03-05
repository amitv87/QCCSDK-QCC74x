.. _ble-index:

BLE
==================

Overview
----------
- Features supported by BLE:

     + Bluetooth HOST feature

         - Roles supported by GAP: Peripheral and Central, Observer and Broadcaster
         - Roles supported by GATT: Server and Client
         - Supports pairing including secure connection features in Bluetooth 4.2
         - Supports permanent storage of Bluetooth-specific settings and data

     + Bluetooth mesh features

         - Supports Relay, Friend Node, Low-Power Node (LPN) and GATT Proxy roles
         - Supports two types of Provisioning bearers (PB-ADV & PB-GATT)

- BLE protocol stack architecture:

                        .. figure:: img/ble1.png

    + There are 3 main layers in total, which together form a complete Bluetooth low energy protocol stack

         - Host: This layer sits below the application and consists of multiple (non-real-time) network and transport protocols that enable applications to communicate with peer devices in a standard and interoperable manner.
         - Controller: The controller implements the Link Layer (LE LL), a low-level real-time protocol that together with the radio hardware provides standard interoperability for over-the-air communications. The LL handles the reception and transmission of packets, ensures data delivery, and handles all LL control procedures.
         - Radio Hardware: Implements the required analog and digital baseband functional blocks, allowing the link layer firmware to transmit and receive in the 2.4GHz band of the spectrum.

- Main control Host:

                        .. figure:: img/ble2.png
                           :align: center
                           :scale: 70%

    * The Bluetooth Host layer implements all high-level protocols and profiles, and most importantly it provides high-level APIs for applications

         - HCI: Host and controller interface
         - L2CAP: Logical Link Control and Adaptation Protocol
         - GATT: Generic Attribute Profile
         - GAP: Generic Access Profile
         - SMP: Security Manager Configuration Layer (Security Manager Specification)

- Application

     * The application layer is where users develop actual Bluetooth applications, including necessary protocol stack parameter settings and calls to various functional functions. We analyze it from two devices: Bluetooth slave and Bluetooth master.

         * Bluetooth slave

             - Initialization of related hardware and basic services
             - Set broadcast parameters: broadcast data, broadcast interval, scanning response and other parameters or data
             - Set Profile: add slave services, characteristic values, and set callback functions to receive host data, etc.
             - Set pairing parameters (optional)
             - Start the broadcast and start running
             - Waiting for related events and event processing, such as receiving data from the host, being linked, etc.

         * Bluetooth host

             - Initialization of related hardware and basic services
             - Set scanning parameters
             - Set connection parameters
             - Set pairing parameters (optional)
             - Start the protocol stack and start running
             - Waiting for related events and event processing, such as scanning events, slave Notify events, etc.

CLI Commands
-------------

ble_init
^^^^^^^^^^^^^^^^^^^^

- Command function: ble general initialization, this command needs to be entered before all ble cli operations
- Parameters: None
- Example: Enter the command ``ble_init``

    .. figure:: img/image2.png
       :alt:

ble_auth
^^^^^^^^^^^^^^^^^^^^

- Command function: register SMP interface function
- Parameters: None
- Example: Enter the command ``ble_auth``

    .. figure:: img/image3.png
       :alt:

ble_unpair
^^^^^^^^^^^^^^^^^^^^

- Command function: clear pairing key
- The first parameter indicates the device address type

   - 0: The device represents the public address type
   - 1: Indicates that the device address is of random type
   - 2: Indicates that the device address is a resolvable address or a Public address
   - 3: Indicates that the device address is a resolvable address or a random address

- The second parameter represents the device address, with the high byte first and the low byte last. If it is 0, it means clearing the keys of all devices.
- Example: Enter the command ``ble_unpair 0 0``

    .. figure:: img/image21.png
       :alt:

ble_start_adv
^^^^^^^^^^^^^^^^^^^^

- Command function representation: turn on broadcast
- The first parameter indicates the broadcast type

   - 0: adv_ind can be connected and scanned;
   - 1: adv_scan_ind cannot be connected but can be scanned;
   - 2: adv_nonconn_ind cannot be connected and cannot be scanned;
   - 3: adv_direct_ind The device connection that can be specified cannot be scanned

- The second parameter indicates broadcast mode

   - 0: General discoverable;
   - 1: non discoverable;
   - 2: limit discoverable;

- The third parameter represents the minimum broadcast gap, which is calculated as 0.625ms * N, and the range should be between 20 ms to 10.24 s
- The fourth parameter represents the maximum broadcast gap, which is calculated as 0.625ms * N, and the range should be between 20 ms to 10.24 s
- Example: Enter the command ``ble_start_adv 0 0 0x80 0x80``

    .. figure:: img/image4.png
       :alt:

ble_stop_adv
^^^^^^^^^^^^^^^^^^^^

- Command function: Stop ADV broadcast
- Parameters: None
- Example: Enter the command ``ble_stop_adv``

    .. figure:: img/image17.png
       :alt:

ble_start_scan
^^^^^^^^^^^^^^^^^^^^

- Command function: Indicates scanning broadcast equipment
- The first parameter indicates the scan type

   - 0: Indicates that the scan passive type only listens to broadcast data
   - 1: Indicates scan active. It not only monitors but also sends scan_req packets when the conditions are met.

- The second parameter indicates filtering device broadcast packets

   - 0: Indicates that duplicate filtering is not enabled
   - 1: Indicates enabling duplicate filtering
   - 2: Only receive broadcast and scan response packets initiated by the whitelist list, except for the adv_direct_ind broadcast packet where the specified connection address is not its own.
   - 4: Use extended filtering strategy to filter devices

- The third parameter represents the scan gap. Its calculation method is 0.625ms * N. The range is between 2.5 ms and 10.24 s. It should be greater than or equal to the scan window.
- The fourth parameter represents the scan window. Its calculation method is 0.625ms * N. The range is between 2.5 ms and 10.24 s. It should be less than or equal to the scan gap.
- Example: Enter the command ``ble_start_scan 0 0 0x80 0x40``

    .. figure:: img/image11.png
       :alt:

ble_stop_scan
^^^^^^^^^^^^^^^^^^^^

- Command function: Stop scanning
- Parameters: None
- Example: After the system enters SCAN, enter the command ``ble_stop_scan``

    .. figure:: img/image14.png
       :alt:

ble_conn_update
^^^^^^^^^^^^^^^^^^^^

- Command function: indicates updating connection parameters
- The first parameter represents the minimum value of the connection gap, which is calculated as N * 1.25 ms and ranges from 7.5 ms to 4 s.
- The second parameter represents the maximum value of the connection gap, which is calculated as N * 1.25 ms and ranges from 7.5 ms to 4 s.
- The third parameter indicates how many connection events the slave device delays. The range is 0~499. For example, if this value is set to 1, it indicates that data interaction is delayed by one event. The effect is to reduce the frequency of interaction and save power.
- The fourth parameter represents the connection timeout time, the calculation method is N * 10 ms, the range is 100 ms to 32 s
- Example: After the connection is successful, enter the command ``ble_conn_update 0x28 0x28 0x0 0xf4``

    .. figure:: img/image7.png
       :alt:

ble_security
^^^^^^^^^^^^^^^^^^^^

- Command function: Set SMP encryption level
- The first parameter indicates the encryption level, there are 5 levels in total

   - 0: only used for BR/EDR, such as SDP service
   - 1: Indicates a process that does not require encryption or authentication
   - 2: Indicates a process that requires encryption and does not require authentication
   - 3: Indicates that encryption and authentication are required, for example, both parties need to enter a PIN code
   - 4: Indicates the need for encryption and authentication, through a 128-bit key
   - Example: After the connection is successful, enter the command ``ble_security 2``

    .. figure:: img/image8.png
       :alt:

ble_get_device_name
^^^^^^^^^^^^^^^^^^^^^^^^^^

- Command function: Get local device name
- Parameters: None
- Example: Enter the command ``ble_get_device_name``

ble_set_device_name
^^^^^^^^^^^^^^^^^^^^^^^^^^^

- Command function: Set local device name
- Parameter: the device name that needs to be set
- Parameters: None
- Example: Enter the command ``ble_set_device_name qcc74x``

ble_read_local_address
^^^^^^^^^^^^^^^^^^^^^^^^^^^

- Command function: read local device address
- Parameters: None
- Example: Enter the command ``ble_read_local_address``

    .. figure:: img/image15.png
       :alt:

ble_set_adv_channel
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- Command function: Set ADV channel
- Parameter: The number of ADV channels that need to be set, the value range is 1-7, the parameter size is 1byte, bit0 represents channel 37, bit1 represents channel 38, bit2 represents channel 39
- Example: Enter the command ``ble_set_adv_channel 4``

    .. figure:: img/image16.png
       :alt:

ble_connect
^^^^^^^^^^^^^^^^^^^^

- Command function: Connect to the device with the specified address
- The first parameter indicates the device address type

   - 0: The device represents the public address type
   - 1: Indicates that the device address is of random type
   - 2: Indicates that the device address is a resolvable address or a Public address
   - 3: Indicates that the device address is a resolvable address or a random address

- The second parameter represents the device address, with the high byte first and the low byte last
- Example: Enter the command ``ble_connect 0 18B905DE96E0``

    .. figure:: img/image18.png
       :alt:

ble_disconnect
^^^^^^^^^^^^^^^^^^^^

- Command function: Disconnect the device with the specified address
- The first parameter indicates the device address type

   - 0: The device represents the public address type
   - 1: Indicates that the device address is of random type
   - 2: Indicates that the device address is a resolvable address or a Public address
   - 3: Indicates that the device address is a resolvable address or a random address

- The second parameter represents the device address, with the high byte first and the low byte last.
- Example: After the connection is successful, enter the command ``ble_disconnect 0 18B905DE96E0``

    .. figure:: img/image19.png
       :alt:

ble_select_conn
^^^^^^^^^^^^^^^^^^^^

- Command function: Among multiple connections, set a certain connection object as the current connection object
- The first parameter indicates the device address type

   - 0: The device represents the public address type
   - 1: Indicates that the device address is of random type
   - 2: Indicates that the device address is a resolvable address or a Public address
   - 3: Indicates that the device address is a resolvable address or a random address

- The second parameter represents the device address, with the high byte first and the low byte last.
- Example: After multiple devices are successfully connected, enter the command ``ble_select_conn 1 5F10546C8D83`` to set the selected connection object as the current connection object. Subsequent operations such as ble_read will act on this connection

    .. figure:: img/image20.png
       :alt:

ble_auth_cancel
^^^^^^^^^^^^^^^^^^^^

- Command function: Cancel the encryption authentication process
- Parameters: None
- Example: When in the SMP process, enter the command ``ble_auth_cancel``

    .. figure:: img/image22.png
       :alt:

ble_auth_passkey_confirm
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- Command function: After receiving the passkey, reply to the remote end, and the peer device also displays the passkey during the pairing process; For example: Confirm passkey for 48:95:E6:73:1C:1A (random): 745491 is printed locally during the pairing process. ;You can send this function to reply
- Parameters: None
- Example: When in the SMP process, the corresponding security level is 3, you need to enter the command ``ble_auth_passkey_confirm``

    .. figure:: img/image9.png
       :alt:

ble_auth_pairing_confirm
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- Command function: After receiving the remote pairing request, use this function to reply to the remote pairing request, for example: the pairing process prints locally Confirm pairing for 00:1B:DC:F2:20:E9 (public); this function can be sent to reply
- Parameters: None
- Example: When in the SMP process, the corresponding security level is 2, enter the command ``ble_auth_pairing_confirm``

    .. figure:: img/image23.png
       :alt:

ble_auth_passkey
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- Command function: Request to enter passkey
- Parameter: passkey value, its range is 0-999999
- Example: When using the ble_security 3 command for pairing, and the SMP pairing method is PASSKEY_INPUT (implementation method in the code: when registering the smp interface function with ble_auth, fill in the function passkey_entry in the data structure bt_conn_auth_cb, passkey_display and passkey_confirm are not filled in, other interface functions just use the default). The serial port will print out Enter passkey for XX:XX:XX:XX:XX:XX (public). At this time, enter the command ``ble_auth_passkey 111111`` to complete the pairing.

    .. figure:: img/image24.png
       :alt:

ble_exchange_mtu
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- Command function: exchange mtu size
- Parameters: None
- Example: After the connection is successful, enter the command ``ble_exchange_mtu``

    .. figure:: img/image25.png
       :alt:

ble_discover
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- Command function: Query the specified service or feature
- The first parameter indicates the type to be queried

   - 0: primary
   - 1: secondary
   - 2:include
   - 3: Characteristic
   - 4: Descriptor

- The second parameter represents the uuid of 2BYTES
- The third parameter represents the starting handle, accounting for 2BYTES
- The fourth parameter represents the end handle, accounting for 2BYTES
- Example: After the connection is successful, enter the command ``ble_discover 0 0x1800 0x1 0xffff``

    .. figure:: img/image26.png
       :alt:

ble_read
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- Command function: Read the data of the specified handle
- The first parameter represents the handle
- The second parameter represents the offset
- Example: After the connection is successful, enter the command ``ble_read 0x5 0``

    .. figure:: img/image27.png
       :alt:

ble_write
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- Command function: Specify the handle to write the corresponding data
- The first parameter represents the handle, accounting for 2 bytes
- The second parameter represents the offset, accounting for 2 bytes
- The third parameter indicates the data length, accounting for 2 bytes, and the maximum does not exceed 512
- The fourth parameter represents the data that needs to be written

- Example: After the connection is successful, write 2 bytes of data. The command is ``ble_write 0xf 0 2 0102``, where 01 is a byte and 02 is a byte

    .. figure:: img/image28.png
       :alt:

ble_write_without_rsp
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- Command function: Specify the handle to write the corresponding data and no reply is required
- The first parameter indicates whether to start the sign write command

   - 0: disable sign write command
   - 1: Enable sign write command

- The second parameter represents the handle, accounting for 2 bytes
- The third parameter indicates the length of the data, accounting for 2 bytes, and the maximum does not exceed 512
- The fourth parameter represents the written data

- Example: After the connection is successful, write 2 bytes of data. The command is ``ble_write_without_rsp 0 0xf 2 0102``, where 01 is a byte and 02 is a byte

    .. figure:: img/image29.png
       :alt:

ble_subscribe
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- Command function: Subscribe to CCC
- The first parameter represents the CCC handle
- The second parameter represents the handle of the subscription value
- The third parameter indicates the subscription type

   - 1: indicates notification
   - 2: indicates indication

- Example: After the connection is successful, enter the command ``ble_subscribe 0xf 0xd 0x1`` to enable CCC notification

    .. figure:: img/image30.png
       :alt:

ble_unsubscribe
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- Command function: Unsubscribe from CCC
- Parameters: None
- Example: Enter the command ``ble_unsubscribe``

    .. figure:: img/image31.png
       :alt:

ble_set_data_len
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- Command function: Set pdu data length
- The first parameter indicates the maximum value of payload transmission, ranging from 0x001B - 0x00FB
- The second parameter indicates the maximum time for payload transmission, the range value is 0x0148 - 0x4290

- Example: When the connection is successful, send the command ``ble_set_data_len 0xfb 0x0848``

    .. figure:: img/image32.png
       :alt:

ble_conn_info
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- Command function: Get all connection information
- Parameters: None
- Example: When the connection is successful, send the command ``ble_conn_info`` to obtain the connected device

    .. figure:: img/image33.png
       :alt:

ble_disable
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- Command function: log out BLE
- Parameters: None
- Example: When there is no scan/adv/connect event, send the command ``ble_disable``

    .. figure:: img/image34.png
       :alt:

ble_set_tx_pwr
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
- Command function: Set transmit power
- The first parameter represents the set power value
- Example: Send command ``ble_set_tx_pwr 0xa``

    .. figure:: img/image35.png
       :alt:

Functions
----------------
ble stack uses zephyr ble stack. For API, please refer to `zephyr bluetooth api <https://docs.zephyrproject.org/2.1.0/>`_.
