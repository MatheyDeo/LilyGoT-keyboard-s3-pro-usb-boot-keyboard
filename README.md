# T-Kyboard-s3-PRO Video Game Controller

This code is for the "Lilygo T-keyboard-s3-pro".
Once uploaded, it will turn the keyboard into a fully functioning video game controller. 

## Bindings

The bindings in this project are set as follows
1. Button                 -  Left Arrow Key
2. Button                 -  Up Arrow Key
3. Button                 -  Down Arrow Key
4. Button                 -  Right Arrow Key
5. Rotary encoder         -  Up / Down arrow keys
6. Rotary encoder button  -  Enter(Return) key

All bindings can be changed from code via the **KEY_SET** macros.

## Compile & Upload

This code is for the **ArduinoIDE>** version **2.0** & newer.


### IMPORTANT ###

For this code to work correctly, Espressif boards version **2.0.14** must be installed in Arduino IDE.



The USB Wrapper library doesn't let users change the Interface protocol, although some devices require it.
The "Keyboard" interface protocol can be forced by directly editing the TinyUSB library.

To do this, open the **Hid.h** at <br>
`C:\Users\user\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.14\tools\sdk\esp32s3\include\arduino_tinyusb\tinyusb\src\class\hid\hid.h`(windows) or
`/arduino15/packages/esp32/hardware/esp32/2.0.1x/tools/sdk/esp32s3/include/arduino_tinyusb/tinyusb/src/class/hid/hid.h`(linux)
and change the line 70 from `HID_ITF_PROTOCOL_NONE     = 0, ///< None` to:
`HID_ITF_PROTOCOL_NONE     = 1, ///< None`.

After the change, every compiled HID device will be forced into "keyboard" interface mode.
To revert the change, one can reinstall the ESP32 board package.
