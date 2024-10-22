# Matter App Documentation

## Support for CHIP Platforms

Supported platforms: `qcc743`, `qcc744`

## Environment Setup

**Steps**:
   
   Download and Prepare the ZAP Package

   Required Version: Matter v1.3

   Download Link: https://github.com/project-chip/zap/releases

   Extract the downloaded ZAP package to a preferred location.

   Note: Avoid using the .deb package for compatibility reasons.


### Sync the connectedhomeip source code

**Steps**:
   Navigate to the example/matter/wifi directory.

   Run script "checkout_matter_code.py" to sync the connectedhomeip source code
   and apply qcc74x related changes on top of it.

   ```bash
   cd examples/matter/wifi
   python checkout_matter_code.py <matter_directory_path>

   ```
   
   The command above will clone the connectedhomeip source code into the
   directory specified by <matter_directory_path>.
   The source code will be located at <matter_directory_path>\connectedhomeip.



### Compilation process

**Steps**:
Run the preparation script:

```bash
./matter_prepare_build.sh <MATTER_REPO_PATH> <QCC74X_SDK_TOOLCHAIN> <ZAP_INSTALL_PATH> <QCC74X_SDK_SOURCE_ROOT>
```

MATTER_REPO_PATH: Directory path where the connectedhomeip source code is
                  synced (e.g., <matter_directory_path>\connectedhomeip).

QCC74X_SDK_TOOLCHAIN: Directory path to the QCC74x toolchain binaries.

ZAP_INSTALL_PATH: Directory path where ZAP is installed.

QCC74X_SDK_SOURCE_ROOT: Directory path where the QCC74x source code is synced.

The command above will compile the connectedhomeip source with QCC74x support
and copy the necessary libraries to the QCC74x SDK. These libraries will then
be used to generate QCC74x images with Matter support.

Compile the Matter Lighting Application Example:

```bash
make
```

Use the following command to flash the image onto the QCC74x device

Execute the following command to flash the firmware onto the device:
```bash

make flash COMX=xxx  # Replace 'xxx' with your COM port name
```


### Working with the CHIP Tool

The CHIP Tool (`chip-tool`) is a Matter controller implementation that allows to
commission a Matter device into the network and to communicate with it using
Matter messages, which may encode Data Model actions, such as cluster commands.

The tool also provides other utilities specific to Matter, such as parsing of
the setup payload or performing discovery actions.


### Source files

You can find source files of the CHIP Tool in the 
`<MATTER_REPO_PATH>/connectedhomeip/examples/chip-tool` directory.


### Building the CHIP Tool

To build and run the CHIP Tool:

1. Open a command prompt in the `connectedhomeip` directory.

2. Run the following command:

    ```
    cd <MATTER_REPO_PATH>\connectedhomeip
    ./scripts/examples/gn_build_example.sh examples/chip-tool BUILD_PATH
    ```

    In this command, `BUILD_PATH` specifies where the target binaries are to be
    placed.



### Commissioning and Testing OnOff Function

Use the following commands on a linux machine on which chiptool is compiled
for commissioning and testing the OnOff capability:

```bash
./chip-tool pairing ble-wifi <node_id> <wifi_ssid> <wifi_passwd> 20202021 3840
./chip-tool onoff toggle <node_id> 1
```
