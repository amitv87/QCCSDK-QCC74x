.. _flash_prog_cfg:

Usage of qcc74x_flash_cube
=================================

qcc74x SDK uses the new flash tool (**qcc74x_flash_cube**), and programming depends on the **flash_prog_cfg.ini** file.

Compared with the cumbersome functions of qcc74x DevCube, qcc74x_flash_cube is only used to download code. When the user executes ``make flash``, the executable file under **qcc74x_flash_cube** will be called and burned according to **flash_prog_cfg.ini**.
This section mainly introduces the syntax of **flash_prog_cfg.ini**. For a more detailed introduction, please refer to ``tools/qcc74x_tools/qcc74x_flash_cube/docs/FlashCube_User_Guide.pdf``

Overview
----------------------------------

For normal use of ``flash_prog_cfg.ini``, you only need to create a KEY, such as [FW], and fill in the filedir and address.

There are several ways to fill in filedir:

- Absolute path of bin file + bin file name
- bin file relative path + bin file name
- Adding the _$(CHIPNAME) suffix to the bin file name can automatically identify different chips (only used when the bin file name prefixes are different)
- Add the * wildcard character to the bin file name to automatically complete the bin file name (only used when the bin file name prefixes are different)

General MCU Usage (without wireless functionality)
----------------------------------------------------------

.. code-block:: ini
   :linenos:

    [cfg]
    # 0: no erase, 1:programmed section erase, 2: chip erase
    erase = 1
    # skip mode set first para is skip addr, second para is skip len, multi-segment region with ; separated
    skip_mode = 0x0, 0x0
    # 0: not use isp mode, #1: isp mode
    boot2_isp_mode = 0

    [FW]
    filedir = ./build/build_out/xxx*_$(CHIPNAME).bin
    address = 0x0000


- **cfg** represents some configurations during programming, which normally do not need to be changed.
- **FW**: The application firmware to be burned must use the **FW** name.

   - **filedir** represents the relative path where the application firmware is located. Normally, it is placed in the ``build/build_out`` directory after compilation. ``_$(CHIPNAME).bin`` is used to automatically distinguish between different chips. ``xxx`` represents the application firmware name, which is consistent with the name in ``project(xxx)`` in ``CMakeLists.txt``. ``*`` indicates regular matching, which can be used or not.
   - **address** must use 0 address

General IoT Usage (using wireless functionality)
---------------------------------------------------------

.. code-block:: ini
   :linenos:

    [cfg]
    # 0: no erase, 1:programmed section erase, 2: chip erase
    erase = 1
    # skip mode set first para is skip addr, second para is skip len, multi-segment region with ; separated
    skip_mode = 0x0, 0x0
    # 0: not use isp mode, #1: isp mode
    boot2_isp_mode = 0

    [boot2]
    filedir = ./build/build_out/boot2_*.bin
    address = 0x000000

    [partition]
    filedir = ./build/build_out/partition*.bin
    address = 0xE000

    [FW]
    filedir = ./build/build_out/xxx*_$(CHIPNAME).bin
    address = 0x10000

    [mfg]
    filedir = ./build/build_out/mfg*.bin
    address = 0x210000

- **cfg**: Indicates some configurations during programming, which normally do not need to be changed.
- **FW**: The application firmware to be burned must use the **FW** name.

     - **filedir**: Indicates the relative path where the application firmware is located. Normally, it is placed in the ``build/build_out`` directory after compilation. ``_$(CHIPNAME).bin`` is used to distinguish different chips. ``xxx`` represents the application firmware name, which is consistent with the name in ``project(xxx)`` in ``CMakeLists.txt``.
     - **address** specified by ``partition_xxx.toml``

- **boot2**: The boot2 firmware to be burned must use the boot2 name.

     - **filedir**: Indicates the relative path where the boot2 firmware is located. Normally, it is placed in the ``build/build_out`` directory after compilation. Automatically copy from the ``bsp/board/board_name/config`` directory.
     - **address**: must use address 0

- **partition**: The partition firmware to be burned must use the partition name.

     - **filedir**: Indicates the relative path where the partition firmware is located. Normally, it is placed in the ``build/build_out`` directory after compilation. Automatically convert ``partition_xxx.toml`` from the ``bsp/board/board_name/config`` directory into a bin file and copy it.
     - **address** specified by ``partition_xxx.toml``

- **mfg**: The mfg firmware to be burned must use the mfg name. mfg is optional and does not need to be burned.

     - **filedir**: Indicates the relative path where the mfg firmware is located. Normally, it is placed in the ``build/build_out`` directory after compilation. Automatically copy from the bsp/board/board_name/config directory.
     - **address** specified by ``partition_xxx.toml``

.. note:: If partition is used, **address** can use @partition instead of absolute address. @partition will automatically find the corresponding address from ``partition_xxx.toml``

.. code-block:: ini
   :linenos:

    [cfg]
    # 0: no erase, 1:programmed section erase, 2: chip erase
    erase = 1
    # skip mode set first para is skip addr, second para is skip len, multi-segment region with ; separated
    skip_mode = 0x0, 0x0
    # 0: not use isp mode, #1: isp mode
    boot2_isp_mode = 0

    [boot2]
    filedir = ./build/build_out/boot2_*.bin
    address = 0x000000

    [partition]
    filedir = ./build/build_out/partition*.bin
    address = 0xE000

    [FW]
    filedir = ./build/build_out/xxx*_$(CHIPNAME).bin
    address = 0x10000

    [mfg]
    filedir = ./build/build_out/mfg*.bin
    address = 0x210000

Multiple Firmwares Programming
----------------------------------

The wildcard character * and the ``_$(CHIPNAME)`` prefix are prohibited because the bin file names have the same prefix.

.. code-block:: ini
   :linenos:

    [cfg]
    # 0: no erase, 1:programmed section erase, 2: chip erase
    erase = 1
    # skip mode set first para is skip addr, second para is skip len, multi-segment region with ; separated
    skip_mode = 0x0, 0x0
    # 0: not use isp mode, #1: isp mode
    boot2_isp_mode = 0

    [FW1]
    filedir = ./build/build_out/xxx0.bin
    address = 0x00000

    [FW2]
    filedir = ./build/build_out/xxx1.bin
    address = 0x10000

    [FW3]
    filedir = ./build/build_out/xxx2.bin
    address = 0x20000

