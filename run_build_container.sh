#!/bin/bash
# Run Build Verification inside a Docker Container (for Linux/macOS)

# Check if docker is installed
if ! command -v docker &> /dev/null; then
    echo "Error: docker command not found."
    echo "Please install Docker and ensure the daemon is running."
    exit 1
fi

echo "Building Docker Image 'luna-build-env'..."
docker build -t luna-build-env .
if [ $? -ne 0 ]; then
    echo "Failed to build the Docker image."
    exit 1
fi

echo "Running firmware compilation verification in the container..."
docker run --rm -v "$(pwd):/workspace" luna-build-env
if [ $? -eq 0 ]; then
    echo "Containerized build verification successful!"
else
    echo "Containerized build verification failed!"
fi
