del /a /f /s /q "*.log"
del /a /f /s /q "*.ini"
del /a /f /s /q "*.db"
del /a /f /s /q "iot.toml"
del /a /f /s /q "iot_linux.toml"
del /a /f /s /q "mcu.toml"
del /a /f /s /q "storage.toml"
del /a /f /s /q "partition.bin"
del /a /f /s /q "ro_params.dtb"
del /a /f /s /q .\chips\qcc743\img_create_iot\*.binn
del /a /f /s /q .\chips\qcc743\img_create_mcu\*.bin
del /a /f /s /q .\chips\qcc743\efuse_bootheader\*.bin
del /a /f /s /q .\chips\*.pack
rd /s /Q .\chips\qcc743\ota
rd /s /Q .\chips\qcc743\builtin_imgs

for /l %%a in (1,1,16) do (
  rd /s /Q task%%a
)

pause