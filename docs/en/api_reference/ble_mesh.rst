.. _blemesh-index:

BLE_MESH
==================

CLI commands
-------------

blemesh_init
^^^^^^^^^^^^^^^^^^^^

- Command function: blemesh general initialization. This command needs to be entered before all blemesh cli operations.
- Parameters: None
- Example: Enter the command ``blemesh_init``

blemesh_pb
^^^^^^^^^^^^^^^^^^^^

- Command function: Set Provisioning bearer
- The first parameter indicates the Provisioning carrying mode

   - 1: Bearing mode PB-ADV
   - 2: Bearing mode PB-GATT

- The second parameter indicates enable
- Example: Enter the command ``blemesh_pb 2 1``

Functions
----------------

ble stack uses zephyr ble mesh. For API, please refer to `zephyr bluetooth mesh api <https://docs.zephyrproject.org/apidoc/latest/group__bt__mesh.html>`_.
