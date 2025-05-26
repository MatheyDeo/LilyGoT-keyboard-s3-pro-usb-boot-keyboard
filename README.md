# T-Kyboard-s3-PRO Video Game Controller

This code is for The "Lilygo T-keyboard-s3-pro".
and once uploaded, it will turn the keyboard to a fully functioning video game controller. 

## Bindings

The bindings in this project are set as follows
1. Button                 -  Left Arrow Key
2. Button                 -  Up Arrow Key
3. Button                 -  Down Arrow Key
4. Button                 -  Right Arrow Key
5. Rotary encoder         -  Up / Down arrow keys
6. Rotary encoder button  -  Enter(Return) key

All of the bindings can be changed from code via the **KEY_SET** macros.

## Compile & Upload

This code is for the **arduinoIDE>** version **2.0** & newer.


### IMPORTANT ###

For this code to work correctly, Espressif boards version **2.0.1x** must be installe in Arduino IDE.



The USB Wrapper library used, dosent let users change the Interface protocol, although some devices require it.
The "Keyboard" interface protocol can be forced by directly editing the TinyUSB library.

To do this, open the **Hid.h** at <br>`/arduino15/packages/esp32/hardware/esp32/2.0.15/tools/sdk/esp32s3/include/arduino_tinyusb/tinyusb/src/class/hid/hid.h`
and change the line 70 from `HID_ITF_PROTOCOL_NONE     = 0, ///< None` to:
`HID_ITF_PROTOCOL_NONE     = 1, ///< None`.

After the change, every compiled HID device will be forced into "keyboard" interface mode.
to revert the change, one can simply reinstall the Esp32 board package.
