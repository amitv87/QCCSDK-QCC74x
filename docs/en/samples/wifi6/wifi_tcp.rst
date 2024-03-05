WIFI TCP
====================

This section mainly introduces the use of QCC743/QCC744 wifi and how to make TCP requests.

Preparation before Development
----------------------------------------
Same as :ref:`wifi_http`

Hardware Connection
-----------------------------

none

Software Implementation
-----------------------------

For more detailed code, please refer to examples/wifi/sta/wifi_tcp

Compile and Program
-----------------------------

- Compilation, the Makefile file has already configured other parameters ( CHIP and BOARD ), and does not need to be filled in by the user.

.. code-block:: bash

    $ cd examples/wifi/sta/wifi_tcp
    $ make

- Program

Reference :ref:`get_started`

Experimental phenomena
-----------------------------

- Open the serial port terminal software, press the reset key, and output log. Press the Enter key to display the words qcc74x />, similar to the Linux terminal.

- Enter wifi_sta_connect ssid pwd to connect to the network, where ssid is the connected ap name and pwd is the connected ap password

- After the wifi connection is successful, the assigned IP address will be printed out and CONNECTED will be prompted.

If you want to test TCP TX, please use the following instructions for testing.

- The TCP server needs to be initialized on the PC side of the same LAN using the iperf instruction

- Enter wifi_tcp_test Server IP port to connect to the tcp server

If you want to test TCP RX, please use the following instructions for testing.

- Enter wifi_tcp_echo_test port to start a TCP server

- The client can connect to the TCP server of the module through the iperf command
