# Use an official lightweight Ubuntu base image
FROM ubuntu:22.04

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install system dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    curl \
    git \
    python3 \
    python3-pip \
    python3-serial \
    unzip \
    wget \
    xz-utils \
    ca-certificates \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Install arduino-cli
RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh

# Configure arduino-cli and install the ESP32 core
RUN arduino-cli config init --overwrite && \
    arduino-cli config set board_manager.additional_urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json && \
    arduino-cli core update-index && \
    arduino-cli core install esp32:esp32@3.0.2

# Pre-install common libraries via Library Manager to speed up subsequent builds
RUN arduino-cli lib install "Adafruit GFX Library" && \
    arduino-cli lib install "Adafruit ST7735 and ST7789 Library" && \
    arduino-cli lib install "QRCode"

# Set up the workspace directory
WORKDIR /workspace

# By default, copy the workspace files
# (This can be overridden by mounting the workspace directory at /workspace)
COPY . /workspace

# Run the verify_builds script by default
CMD ["python3", "tools/verify_builds.py"]
