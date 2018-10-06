This project is part of a larger monitoring system for air handlers, temperature
and humidity sensors, air conditioners, and a steam boiler.

This specific project is for monitoring the temperatures in various parts of
the building by using DS18B20s scattered through the hallways.  A STM32F103C8T6
is used for the MCU, as that has the larger memory needed for the one-wire
operations, especially the code that scans for multiple DS18B20s on the same wire.
RS485 is used to communicate through the UART to the master program that is
running on a Raspberry Pi (Python3, TkInter for an interactive display).

This device only responds to its id being received via RS485.  It will only
talk when asked by the main program, by responding to its id.

Because of the lengths of cable that have to be used, only 4 of the DS18B20s
seem to work on one line.  Adding the fifth tends to shut them all down, at
least with this setup.  Altogether, about 3,000 feet of CAT-3 cable is used
to connect to the DS18B20s, with 5 volts supplied to each device (not using
parasite power).

This is a cheap project, using one STM32F103C8T6, a RS485 module, a voltage
regulator, and a variable number of DS18B20s (along with a 4.7k resistor).

Eclipse is used on Linux as an IDE and the programs are compiled with the
ARM Cross GCC toolchain.  Specifically it is arm-none-eabi-gcc from Linaro.
Flashing is done using st-flash and a ST Link/V2 (4 wire connection to the board).

I'm sure the ST Micro supplied programs would work, but I don't use Windows unless
I have no choice.

Anyone can use these programs however they wish.  These are designs that are
being used every day in a nursing home.  Much of the code came from other
examples that people have on GitHub or in Arduino.
