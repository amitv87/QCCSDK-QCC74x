.. _wifi_http:

WIFI HTTP
====================

This section mainly introduces the use of QCC743/QCC744 wifi and how to make http requests.

Preparation before Development
-------------------------------------

Hardware Preparation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Prepare a MQM622/MQM623 development board.

The development board has a USB interface. After connecting to the computer using a Type-C cable, the USB can enumerate a serial port while powering the development board.

The development board also has a Boot button, which is used to select the operating mode after the chip is reset. With the Reset button, it can realize the Flash startup and download mode entry of the chip.

- operating mode

Directly press the RESET button and release it. After the button is released, the chip starts from Flash and enters the running mode.

- Download mode

First press the BOOT button, then press the RESET button, then release the RESET button, and finally release the BOOT button. At this time, the chip enters the programming mode.

PC Preparation
^^^^^^^^^^^^^^^^^^^^^

Prepare PC terminal tools: xshell or mobaxterm. Under Linux, you can choose picocom or putty.

Hardware Connection
-----------------------------

none

Software Implementation
-----------------------------

For more detailed code, please refer to **examples/wifi/sta/wifi_http**

Compile and Program
-----------------------------

- Compilation, the Makefile file has already configured other parameters ( **CHIP** and **BOARD** ), and does not need to be filled in by the user.

.. code-block:: bash
   :linenos:

   $ cd examples/wifi/sta/wifi_http
   $ make

- Program

Reference :ref:`get_started`

Experimental phenomena
-----------------------------

- Open the serial port terminal software, press the reset key, and output the following log. Press the Enter key to display the words **qcc74x />**, similar to the Linux terminal.

- Enter ``wifi_sta_connect ssid pwd`` to connect to the network, where ssid is the connected ap name and pwd is the connected ap password

- After the wifi connection is successful, the assigned IP address will be printed out and CONNECTED will be prompted.

- Enter ``ping url`` to ping a website, url is the website IP address or domain name

- Enter ``wifi_http_test url`` to get html data from the website, url is the website IP address or domain name

- Reference for using other commands :ref:`wifi6_api`

