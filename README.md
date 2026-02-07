# BLE Server Project

This project implements a Bluetooth Low Energy (BLE) server, targetting the Raspberry Pi Pico 2 (rp2350). It includes components for a BLE server (`ble_server.c`, `ble_server.h`), GATT service definition (`foo.gatt`), and BTstack configuration (`btstack_config.h`).

## Compiling the Project

This project uses CMake for its build system. Follow these steps to compile:

1.  **Create a build directory:**
    ```bash
    mkdir build
    cd build
    ```
2.  **Configure the project with CMake:**
    ```bash
    cmake ..
    ```
    *(Ensure you have the Pico SDK properly configured and discoverable by CMake, or provide its path if necessary.)*
3.  **Build the project:**
    ```bash
    make
    ```

After successful compilation, the executables or firmware images will be located in the `build` directory.
