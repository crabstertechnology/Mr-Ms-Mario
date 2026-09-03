# Run Build Verification inside a Docker Container
$DockerPath = "C:\Users\sasit\AppData\Local\Programs\DockerDesktop\resources\bin\docker.exe"

# Resolve docker command
if (Test-Path $DockerPath) {
    Set-Alias docker $DockerPath -Scope Script
} else {
    # Check if docker is already in path
    if (!(Get-Command docker -ErrorAction SilentlyContinue)) {
        Write-Host "Error: docker.exe not found on the host system." -ForegroundColor Red
        Write-Host "Please install Docker Desktop or add docker to your environment PATH." -ForegroundColor Yellow
        exit 1
    }
}

Write-Host "Building Docker Image 'luna-build-env'..." -ForegroundColor Cyan
docker build -t luna-build-env .
if ($LASTEXITCODE -ne 0) {
    Write-Host "Failed to build the Docker image." -ForegroundColor Red
    exit 1
}

Write-Host "Running firmware compilation verification in the container..." -ForegroundColor Cyan
docker run --rm -v "${PWD}:/workspace" luna-build-env
if ($LASTEXITCODE -eq 0) {
    Write-Host "Containerized build verification successful!" -ForegroundColor Green
} else {
    Write-Host "Containerized build verification failed!" -ForegroundColor Red
}
