WIFI UDP
====================

This section mainly introduces the use of QCC743/QCC744 wifi and how to make UDP requests.

Preparation before Development
----------------------------------
Same as :ref:`wifi_http`

Hardware Connection
-----------------------------

none

Software Implementation
-----------------------------

For more detailed code, please refer to examples/wifi/sta/wifi_udp

Compile and Program
-----------------------------

- Compilation, the Makefile file has already configured other parameters ( CHIP and BOARD ), and does not need to be filled in by the user.

.. code-block:: bash

    $ cd examples/wifi/sta/wifi_udp
    $ make

- Program

Reference :ref:`get_started`

Experimental phenomena
-----------------------------

- Open the serial port terminal software, press the reset key, and output the following log. Press the Enter key to display the words qcc74x />, similar to the Linux terminal.

- Enter wifi_sta_connect ssid pwd to connect to the network, where ssid is the connected ap name and pwd is the connected ap password

- After the wifi connection is successful, the assigned IP address will be printed out and CONNECTED will be prompted.

If you want to test UDP RX, please use the following instructions for testing.

- Enter wifi_udp_echo port to start a UDP server

- The client can connect to the UDP server of the module through the iperf command
