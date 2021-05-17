# Watch Winder Software #

This is a program for controlling a single watch winder module.

### Features ###
Features of the program.

- Options for turning the winder ON/OFF or in night mode.
- Adjustable number of turns per day.
- Real Time Clock (RTC) module ensuring an accurate reference for setting watches.
- Built in option for setting the RTC.
- Night mode option. This makes the winder turn off during a configurable timeslot of the day/night. The winder automatically adjusts how often it must run to still reach the set number of turns per day.
- Options for adjusting the turn direction of the winder (Bi-directional, Clockwise, or Counter-Clockwise).

### Hardware ###
Below the necessary hardware is listed.

- **Arduino Nano**
  - The brains of the operation. Technically, other Arduinos can be used, but the code setting up the internal timers are configured for the ATmega328(P) found on the Nano.
- **TB6612 Stepper Driver**
- **Real Time Clock RTC DS3231 I2C**
- **Rotary encoder with clickbutton**
  - Any should do, just make sure it is for 5V.
- **OLED 128X32 Display Module IIC I2C**
  - Lots of different brands can be found online for cheap.
- **Stepper motor**
  - I personally used a NEMA14, but others can be used. Just make sure the TB6612 Stepper Driver can support the current/voltage of the selected stepper motor.
- **Buck converter (optional)**
  - I personally used a buck converter to step down the supplied voltage (12V) to the voltage of a NEMA14 stepper motor (7.4V). This in turn also powered the Arduino Nano. Make sure the buck converter can support the current drawn by the stepper motor.

### Wiring ###
The harware should be connected as described below.

#### Rotary Encoder ####
The encoder should be connected to the 5V and GND connections of the Arduino using the appropriate pins on the encoder (VCC/+ and GND/-).

The output from the button of the encoder should be connected to A2 of the Arduino. On my encoder, this pin was labled SW.

The remaining two pins should be connected to A0 and A1 pins of the Arduino. Note, that it does not matter which goes to which, it only affects which turn direction of the encoder moves the cursor up or down. Simply switch these, if the turn direction does not match the desired movement.

#### OLED Display  ####
The OLED display should be connected to the 5V and GND connections of the Arduino using the appropriate pins on the display (VCC/+ and GND/-).

The SCL pin of the display should be connected to the A5 pin of the Arduino.

The SDA pin of the display should be connected to the A4 pin of the Arduino.

#### RTC Module ####
The RTC module should be connected to the 5V and GND connections of the Arduino using the appropiate pins on the RTC module (VCC/+ and GND/-).

The SCL pin of the RTC module should be connected to the A5 pin of the Arduino.

The SDA pin of the RTC module should be connected to the A4 pin of the Arduino.

#### TB6612 Stepper Driver ####
The stepper driver should be connected to the 5V and GND connections of the Arduino using the appropiate pins on the stepper driver (VCC/+ and GND/-).

The VM pin of the stepper driver should be connected to the supply voltage of the stepper motor (in my case this was 7.4 V).

The PWMB and PWMA pins of the stepper driver should be connected to the 5V pin of the Arduino.

The BIN1 pin of the stepper driver should be connected to PIN6 of the Arduino.

The BIN2 pin of the stepper driver should be connected to PIN7 of the Arduino.

The AIN1 pin of the stepper driver should be connected to PIN5 of the Arduino.

The AIN2 pin of the stepper driver should be connected to PIN4 of the Arduino.

The MOTORA pins of the stepper driver should be connected to one coil of the stepper motor.

The MOTORB pins of the stepper driver should be connected to the other coil of the stepper motor.

**NOTE:** If the stepper motor spins the wrong direction when having selected either CLK or CCLK turn direction, switch which coil is connected to which MOTORA/MOTORB.


### How to use the software ###
The software will display the time and date after approximately 20s of no input. If the encoder is turned at any point while the display is showing the time and date, the software will switch to showing the menu. In this menu, the encoder is used to choose a specific item.

#### Selecting ON/OFF/NIGHT MODE ####
A single press of the encoder button while on the STATUS menu item will cycle through the three different settings of the winder (ON/OFF/NIGHT).

#### Adjusting Turns per Day ####
A single press of the encoder button while on the TPD menu item will allow for setting the number of turns per day using the encoder. A single press again will set the selected number of turns per day.

#### Adjusting Turn Direction ####
A single press of the encoder button while on the DIR menu item will cycle through the three different direction settings of the winder (BI/CLK/CCLK), corresponding to bi-directional, clockwise, and counter-clockwise.

#### Adjusting night mode time interval ####
A double press of the encoder button while in the menu will allow for setting the time interval in which the winder will run when in night mode using the encoder. A single or double press will set the selected interval.

#### Adjusting the time/date of the RTC module ####
A long press of the encoder button while in the menu will allow for setting the RTC module using the encoder. The lower left of the screen will show which item is currently being adjusted by the encoder. When the item has been adjusted, a single press of the encoder will move to the next item to be adjusted. When the year has been adjusted, a single press will set the RTC module to the selected time and date. A double press at any time while in this menu will cancel the operation.

**NOTE:** To allow a precise setting of the RTC, the screen will not timeout while in this menu. As the winder only runs while the winder is not beeing used, this will block the winder from running. It is therefore important to either set the time or cancel the time setting (double click) to allow the winder to run.
