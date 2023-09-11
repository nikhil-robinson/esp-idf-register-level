# ESP-IDF GPIO High-Low Example using Register-Level Programming

This is a simple example demonstrating how to control the state of a GPIO pin on the ESP32-S3 microcontroller using register-level programming in the ESP-IDF (Espressif IoT Development Framework). The code sets the GPIO pin high and low alternately in a loop with a 1-second delay between each state change.

## Table of Contents

- [Introduction](#introduction)
- [Getting Started](#getting-started)
- [Code Explanation](#code-explanation)
- [License](#license)

## Introduction

This example showcases how to manipulate the GPIO pins at a low level using ESP-IDF. While the ESP-IDF provides a high-level API for GPIO control, this code directly accesses the hardware registers to achieve the desired functionality.

## Getting Started

To run this example, follow these steps:

1. Set up the ESP-IDF development environment by following the official [ESP-IDF documentation](https://docs.espressif.com/projects/esp-idf/en/latest/).

2. Clone this repository to your local development environment:

   ```bash
   git clone https://github.com/nikhil-robinson/esp-idf-register-level.git
   ```

3. Navigate to the project directory:

   ```bash
   cd esp-idf-register-level/examples/gpio
   ```

4. Build and flash the code to your ESP32-S3 development board using the ESP-IDF build tools.

5. Connect a led to the GPIO 2 to see the output.


## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.

---

Feel free to experiment with this example and modify it for your specific needs. If you have any questions or encounter issues, please don't hesitate to open an issue in this repository.