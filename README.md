A LITTLE GAME
=============

![IMG_20210407_180206](https://user-images.githubusercontent.com/14978209/114098914-50b44180-98c2-11eb-9791-515de6bbeb0b.jpg)


Rules
-----

It's a game for 2 players. There are 2 blue LEDs on top. Each player has 2 buttons - one for each LED. 
If you press the correct button when a LED lights up, you earn a point. If you press the wrong one, 
or do it before any LED lights up, you get 1s cooldown when you can't do anything. 
The LEDs in the middle from green to red are point tracks for both players. When you get max points + 1 you win.
When a point track goes blank, it means that the cooldown is in progress.


Schematic
---------

![Schematic_SimpleArduinoGame1_2021-04-10](https://user-images.githubusercontent.com/14978209/114248066-0c46a580-9997-11eb-9a1f-b0bc28f16905.png)
[Schematic_SimpleArduinoGame1_2021-04-10.pdf](https://github.com/kotbehemot53/simple-arduino-game1/files/6288817/Schematic_SimpleArduinoGame1_2021-04-10.pdf)


Hardware platform
-----------------

Atmega328P with Arduino bootloader (Arduino UNO platform).


Setup
-----

### Platformio + any IDE ###

You can use the project as is - it contains platformio.ini, so it should build and upload out of the box.

### Arduino IDE ###

You may copy the contents of `main.cpp` (except `#include <Arduino.h>`) and paste it to an empty sketch (`.ino` file) and it should work. Set platform to Arduino UNO.


Notes about construction & operation
------------------------------------

Atmega328P doesn't have enough pins to drive each LED in the score tracks separately, so 2 simple serial-to-parallel shift registers are used to
multiplex the LEDs in the score tracks. This means sending a pulse over each LED from bottom to top, one step per program
loop cycle ("frame") until a desired number of LEDs has been lit, and then resetting the register for the remainder of the cycle.
Due to the nature of this solution, it's crucial to connect the LEDs to the shift registers in the correct order.
It's impossible to light up the score track LEDs in any other order (would be possible if a buffered register was used).

Even though the 2 top LEDs are connected to separate pins, they are still multiplexed to conserve energy from the batteries
(the whole thing still draws around 100 mA at 3V when all LEDs are lit).

The green-red LEDs in the score tracks use only 120 ohm resistors, because I wanted them to be as bright as possible (reaching ~20-25 mA of current)
due to their short duty cycles caused by multiplexing over 8 LEDs in each track.
The blue LEDs, on the other hand, use 1,5k ohm resistors, because they were much brighter than others in my case (even when there was no multiplexing). 
This has a benefit of conserving battery power.


Disclaimer
----------

There's no nice OOP design here and no tests. There are a lot of global variables too. I'm aware it all could
be nicer but I did in my spare time and the program is short. I hope the comments make up for the lackings.


Pin mapping
-----------

```
LEFT SIDE       DIP pin  Arduino pin
top led l       19       13
shift reg clk   6        4
shift reg /r    5        3
shift reg data  4        2
btn up/left     15       9
btn down/right  16       10

RIGHT SIDE      DIP pin  Arduino pin
top led r       14       8
shift reg clk   13       7
shift reg /r    12       6
shift reg data  11       5
btn up/right    17       11
btn down/left   18       12
```
