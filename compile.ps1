Write-Host "Compiling Mr.&Ms Luna firmware..." -ForegroundColor Cyan
arduino-cli compile --fqbn esp32:esp32:esp32c3:PartitionScheme=huge_app,CDCOnBoot=cdc --output-dir mobile_app\bin luna_firmware\luna_firmware.ino
if ($LASTEXITCODE -eq 0) {
    Write-Host "Compilation successful! New binaries exported to mobile_app/bin/" -ForegroundColor Green
} else {
    Write-Host "Compilation failed!" -ForegroundColor Red
}
