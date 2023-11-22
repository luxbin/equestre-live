# Equestre Live

## Overview
This application is a bridge between Equestre Pro app and Equestre live web application.

### eco-system
* Equestre Pro
* Equestre Live Web Application
* VTChrono

## Getting Started

### Requirements
* Visual Studio 2019 or above (recommended 2019)
* Additional libraries.
    [POCO library](https://pocoproject.org/download.html#github)
    [OpenSSL win64](https://slproweb.com/download/Win64OpenSSL-1_1_1h.exe)
  You can install these libraries using [vcpkg](https://github.com/microsoft/vcpkg).
  ```
  vcpkg install openssl:x64-windows
  vcpkg install POCO[sqlite3,netssl]:x64-windows --recurse
  ```


### How to build?
1. Download and build poco library. (use cmake)
   This will generate a directory called `cmake-build`. It will be linked to the `SocketIO-poco` project as library dependency.
2. Download and install OpenSSL win64. It will be installed to `C:\Program Files\OpenSSL-Win64` as default.
3. Configure project configuration.
   Make sure the OpenSSL include and lib directory is correctly set in `SocketIO-poco` project configuration.<br/>
   In `Live-Equestre-Pro` project, you will see `F:\work\equestra\3rdparty\Poco\cmake-build\lib\release` directory is set to additional library dependencies. Please change this to indicate your poco build directory. (mentioned above step)
4. Press `Build/Build Solution` or simply hit `ctrl+shift+b`. It will build release files.
