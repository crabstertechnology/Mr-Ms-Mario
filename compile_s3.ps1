Write-Host "Compiling Mr.&Ms Luna ESP32-S3 Mini firmware..." -ForegroundColor Cyan
arduino-cli compile --fqbn esp32:esp32:esp32s3:PartitionScheme=huge_app,CDCOnBoot=cdc --output-dir mobile_app\bin_s3 luna_firmware_s3\luna_firmware_s3.ino
if ($LASTEXITCODE -eq 0) {
    Write-Host "Compilation successful! New binaries exported to mobile_app/bin_s3/" -ForegroundColor Green
} else {
    Write-Host "Compilation failed!" -ForegroundColor Red
}
