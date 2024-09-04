# ESP32 MIDI WiFi Server

This project implements a MIDI file server on an ESP32 device. The server allows users to upload, download, list, and delete MIDI files over a Wi-Fi connection. The files are stored on the ESP32 using the LittleFS filesystem.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Requirements](#requirements)
- [Setup](#setup)
- [API Documentation](https://nobodygitsmypuns.github.io/wifi_midi_server_esp32/#/paths/~1upload/post)
- [Contributing](#contributing)
- [License](#license)

## Overview

The ESP32 MIDI WiFi Server is designed to handle MIDI files, allowing you to manage them wirelessly. The server exposes a REST API that can be accessed via HTTP to perform various operations on the MIDI files stored on the ESP32.

## Config
The config_template.h shows the two config variable need.
Create a config.h file in the same folder as config_templage.h and copy the contents.
Then set the SSID to your network and password accordingly.

## Features

- **File Upload**: Upload MIDI files to the ESP32.
- **File Download**: Download MIDI files from the ESP32.
- **File Listing**: List all MIDI files stored on the ESP32.
- **File Deletion**: Delete MIDI files from the ESP32.
- **Wi-Fi Connectivity**: Easily connect the ESP32 to a local Wi-Fi network.

## Requirements

- **ESP32**: Any ESP32 development board.
- **Arduino IDE**: Used for uploading the code to the ESP32.
- **LittleFS Library**: File system library for ESP32.
- **Wi-Fi Network**: The ESP32 must be connected to a Wi-Fi network.

## Setup

### 1. Clone the Repository

```bash
git clone https://github.com/nobodygitsmypuns/wifi_midi_server_esp
