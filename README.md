
![Brugmania Arcade Cabinet](https://lh6.googleusercontent.com/-lsMdu-8U-FM/Tq1ae26BHmI/AAAAAAAAADI/QX8KwsC1pIU/h301/brugmania.jpg)

### What is this?

This is a complete Frogger game which resembles the original Arcade version from Konami (c) released in 1981, but completely rewritten
from the ground up in the C programming language, with some nice extra elements like "achievements" en 2-player deathmatch mode!

To be able to play you need an Atmel AVR Arduino compatible board, or any other custom board with a (SPI-capable) microcontroller (minimum 16 Mhz and 8K RAM)
and of course the fantastic graphics/sound shield [Gameduino](http://excamera.com/sphinx/gameduino/).

(C) Bas Brugman 2011, freelance internet web application engineer and embedded hardware programmer, [www.visionnaire.nl](http://www.visionnaire.nl).

If you appreciate my work, **please** [DONATE and SUPPORT](http://pledgie.com/campaigns/16203) my game creation projects for the Gameduino. With your support I will be able to quickly
program/port more fantastic arcade games like Pac-man, Donkey Kong and many others.

Anyway, feel free to play around with the code, fork, optimize, bug hunt or just give tips/comments.

### Frogger "Brugmania" version for the Gameduino

- No Arduino framework dependency: rewrote Arduino's SPI and Gameduino's GD library to avr-libc plain C
- Besides the single-player mode, there is also a two-player "deathmatch" mode. (They start simultaneously!)
- Maximum of 255 levels (Although reaching level 10+ is already insane)
- Increased difficulty per level: faster/more cars and faster/less river objects
- Introscreen with high scores, latest achievements, points table and ways-to-die explanation
- "Konami Code" godmode in single-player mode: press joystick up/up down/down left/left right/right and then the middle button (button 2) within 5 seconds at the intro screen.
- Diving turtle groups which face another threat to frogger
- Fine-tuned collision detection on river: frogger has to cover most of the river object edges while jumping on them, else he slips and drowns.
- 2 different dying animations: drowning in water and road splat
- Bonus insects appear in the home slots
- Swimming crocodiles appear at level 2+, their heads are deadly
- Deadly crocodiles appear in the home slots at level 3+
- Flanking otters appear at level 3+
- Crawling snakes on logs and the riverbank appear at level 3+, their heads are deadly
- Full scoring functionality (high scores with 8 character player names are saved and kept in EEPROM): 
- Maximum score is 999999 now, so players can try to beat the world record :)
    * 10 points per forward leap
    * 50 points per home save
    * 200 points per bonus insect
    * 200 points per female frog brought home save
    * 10 points x remaining timer seconds
    * 1000 points per completed level
    * 5000 points per achievement
- Every 20k points an extra life 
- Implemented 6 challenging achievements:
    * The Right Way: On any level fill the 5 frog home slots in order from right to left.
    * Speedy Gonzales: Get all 5 frogs home in any level in less than 60 seconds.
    * Larry Laffer Style: Save 5 lady frogs in one level.
    * The Hungry Pig: Eat 5 insects in one level.
    * Calling Me Chicken?: Stay foot at the bottom berm until the timer turns red. Then manage to bring frogger home safely before the timer runs out.
    * Too much Seinfeld: Get hit by a truck 2 times in one level.
- Ways to die:
    * Running into road vehicles
    * Jumping into the river's water (This frog can't swim :S)
    * Running into snakes' heads, flanking otters or into a crocodile's jaws in the river
    * Jumping into a home invaded by a crocodile
    * Staying on top of a diving turtle too long
    * Drifting off the screen by sitting on a log or turtle too long
    * Jumping into a home already occupied by a frog
    * Jumping into the side of a home or the bushes
    * Jumping against the side and bottom edges of the screen
    * Running out of time before getting a frog home

### Background

Inspired by the clear and simple Gameduino Frogger tutorial (...and of course thinking back at the fun times I had playing the original
when I was like 7/8 years old, begging for more coins from my parents :)), I decided to make a fully functional game out of it...
...3 weeks spare-time programming later, I think it's ready for prime time.

This game is being used with my home-built "Brugmania" Arcade Cabinet. In my opinion an Arcade Cabinet should run retro games only on
comparable low-powered mcu boards (like the 8-bit AVR/Gameduino combo). Throwing in some PS3/Xbox 360 or a complete PC with MAME emulator is
not retro for me, better use those with your 50 inch 3D LCD TV and Dolby DTS 8 speaker setup :)

### Installation

I developed and tested the game on 2 microcontrollers:

Teensy++ 2.0 AVR AT90USB1286: 16MHZ/RAM:8K/Flash:128K/EEPROM:4K 

AVR ATMega1284P: 16MHZ/RAM:16K/Flash:128K/EEPROM:4K 

If you have another type of microcontroller (make sure you have at least 8K RAM and 32K flash), you only need to change some basic preprocessor settings and vars:

In Makefile, change the MCU type variable to match your microcontroller.
In SPI.h (communication from AVR to the Gameduino), add another "if defined(youravrmcuversion) and change the SS/MOSI/MISO/SCK pins/ports to match your AVR mcu.
In control.h (defines) / control.c (init function) (2x 4-way joystick and 2x 5 buttons), change the pins/ports to match your AVR mcu control connections.

(P.S. Only both joysticks and both CONTROL_BTN2 and CONTROL_BTN3 buttons are used in the game.)

My development platform is Xubuntu. It's very to install the avr-libc tool chain:

sudo aptitude install avr-libc gcc-avr gdb-avr simulavr avrdude avrdude-doc binutils-avr binutils avra libusb-1.0-0-dev libusb-dev

To transfer the game to your microcontroller, just run **make** and transfer the *frogger.hex* via *avrdude* or any other tool you're used to.

### License

Please note that I'm making these games purely for personal and educational use. Since all these remakes are based on original copyrighted games, it's not allowed to do anything
commercially with them, like selling or renting out. However my source code is freely available under the 3-clause BSD License. 

Have fun!

Bas Brugman
[E-Mail](mailto:bas.brugman@visionnaire.nl)
