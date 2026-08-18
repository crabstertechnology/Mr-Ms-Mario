Write-Host 'Compiling 1.3 Luna Firmware (ST7789 1.3")...' -ForegroundColor Cyan
arduino-cli compile --fqbn esp32:esp32:esp32c3:PartitionScheme=huge_app,CDCOnBoot=cdc --output-dir mobile_app\bin "1.3 Luna Firmware\1.3 Luna Firmware.ino"
if ($LASTEXITCODE -ne 0) {
    Write-Host "1.3 Luna Firmware Compilation failed!" -ForegroundColor Red
    exit 1
}

Write-Host 'Compiling 1.8 Luna Firmware (ST7735 1.8")...' -ForegroundColor Cyan
arduino-cli compile --fqbn esp32:esp32:esp32c3:PartitionScheme=huge_app,CDCOnBoot=cdc --output-dir mobile_app\bin_v1_8 "1.8 Luna Firmware\1.8 Luna Firmware.ino"
if ($LASTEXITCODE -eq 0) {
    Write-Host "All Compilations successful!" -ForegroundColor Green
} else {
    Write-Host "1.8 Luna Firmware Compilation failed!" -ForegroundColor Red
}
