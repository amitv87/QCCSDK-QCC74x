# Matter App Documentation

## Support for CHIP Platforms

Supported platforms: `qcc743`, `qcc744`

## Environment Setup

### Pull and Prepare the Matter Repository

- **Repository URL**: [matter_qcc74x.git](matter_qcc74x.git)
- **Branch**: `qcc74x_matter_module`

**Steps**:
1. Clone the repository and switch to the specific branch:
   ```bash
   git clone -b qcc74x_matter_module <your_url>/matter_qcc74x.git
   cd matter_qcc74x
   git submodule update --init
   ```

2. Download and Prepare the ZAP Package

    Required Version: Matter v1.3
    Download Link: zap-linux-x64.zip
        Note: Avoid using the .deb package for compatibility reasons.

### Setup Instructions:

Extract the downloaded ZAP package to a preferred location.

Compilation Process

Navigate to the example/matter directory.

Open the matter_build_prepare.sh script and make the following modifications:
```bash
MATTER_REPO_PATH: Path to the cloned Matter repository.
QCC74X_SDK_TOOLCHAIN: Path to the SDK toolchain.
ZAP_INSTALL_PATH: Path to the extracted ZAP package.
```

Run the preparation script:

```bash
./matter_build_prepare.sh
```

Compile the Matter app:
```bash
make
```

Flashing the Firmware

Execute the following command to flash the firmware onto the device:
```bash

make flash COMX=xxx  # Replace 'xxx' with your COM port name
```

Commissioning and Testing OnOff Function

Use the following commands for commissioning and testing the OnOff capability:

```bash
./chip-tool pairing ble-wifi <node_id> <wifi_ssid> <wifi_passwd> 20202021 3840
./chip-tool onoff toggle <node_id> 1
```
