# IMD

This is a snapshot of the code for a portable mp3 player I made in High School. It was run on a Gumstix with a 128x128 OLLED screen for output, and a 3-axis accelerometer for input. Shortly after becoming fully functional, it short-circuited :(

This was done before I knew about a lot of things, and I haven't touched the code since. Please don't hate on the obvious, such as:

 - The Makefile and #includes are a mess
 - Using named pipes for IPC is messier than compiling the C as a Python module ("original" branch)
 - I could have used C++ or Java instead of doing psuedo-OO in C
 - Gumstix is overkill (I did try cheaper options, and they were significantly more difficult back then)
 - Should've used Doxygen-style comments

#### Details/Pics for v1.5: http://www.jbotelho.com/?p=hw_ovw


## Files

* adc driver - Contains source code for modified UCB1400 kernel module that enables all the ADC lines.
* splash - Splash screen that shows IMD logo on gumstix's OLED screen at bootup.
* src v0.5 - Early source code for OLED screen.
* src v1 - First functioning version with music playback. Input from the ADXL330 accelerometer is taken by an AVR and fed into the gumstix serial port for processing. Everything is controlled by gui.py, which uses fifos (named pipes) to communicate with oled (handles drawing to the screen), and audeng (audio playback with MAD mp3 decoder), both written in C. Uses buzhug for music database.
* src v1.5 - Main difference is the replacement of buzhug with a custom database engine (imd_dbe.py), for better performance and to overcome some seg fault issues.
* src v2 - Due to seg fault issues with python, everything was ported to C. The audeng and oled sources were merged with the new imd and gui sources, eliminating the need for pipes and enabling a single, unified app (imd). It is almost feature complete, with mp3 playback, a custon database engine, input from the buttons and acceleroemeter (now through the UCB1400 ADC), primitive battery level readins (rom ADC), and some playlist management features (automatic A-Z sorting, and searching with a context menu).
* test - Simple file to test the gumstix toolchain.
* utils - Various utils, including font converter, and http server (allows OLED's framebuffer to be redirected to a webbrowser).
* imd - Latest binary build of v2.
* Info.txt - Open it and read about Info.txt.
* Todo.txt - Todo list for v2.
