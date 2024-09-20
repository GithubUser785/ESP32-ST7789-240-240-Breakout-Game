# Breakout Game for Arduino

## Description
A breakout-style game for Arduino using an ST7789 display, featuring joystick control, scoring, and power-ups. The game includes sound effects with a buzzer and offers level progression with increasing difficulty.

## Features
- Joystick control for paddle movement
- Collision detection for ball and bricks
- Scoring and level progression
- Sound effects using a buzzer
- Power-ups that increase paddle size

## Getting Started

### Prerequisites
- ESP32 (e.g ESP32-WROVER)
- ST7789 display
- Joystick module
- Buzzer
- Wires and breadboard

### Installation
1. Install the [TFT_eSPI library](https://github.com/Bodmer/TFT_eSPI) via the Arduino Library Manager.
2. Configure the `User_Setup.h` file in the TFT_eSPI library to match your hardware setup.
3. Connect the hardware as described in the setup guide. Here, you can use any pins for SPI but hardware SPI pins are recommended.

## Usage
1. Upload the code to your Arduino board using the Arduino IDE.
2. Use the joystick to move the paddle and interact with the game.
3. Press the joystick button to restart the game if needed.

## Configuration
You can adjust the game speed, level settings, and other parameters directly in the code. Modify the relevant constants and variables to fit your preferences.

## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Contributing
Feel free to open issues or submit pull requests if you have suggestions or improvements.

## Acknowledgements
- TFT_eSPI library by Bodmer
- Joystick module and buzzer components

