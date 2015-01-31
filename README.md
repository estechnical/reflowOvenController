reflowOvenController
====================

ESTechnical Reflow Oven Controller source code

Copyright Ed Simmons 2013-2015 - ESTechnical
ed@estechnical.co.uk
http://www.estechnical.co.uk


Introduction
====================

The ESTechnical reflow oven controller was designed to operate the T962, T962A and T962C ovens.

It operates using PID control of the heater and fan output to improve the control compared to the original manufacturer's controller.

ESTechnical sell the controller hardware ready to install on the ESTechnical website: http://www.estechnical.co.uk

Thanks go to Toby Wilkinson for his excellent menu codebase - http://tobestool.net/t962-t962a-reflow-oven-controller/
Thanks are also due to Brett Beauregard for his excellent work on the arduino PID library - http://playground.arduino.cc/Code/PIDLibrary

Overview
====================

To load the latest firmware to your ESTechnical Reflow Controller you will require:
 * The arduino IDE
 * A slight modification to a config file (adds a menu item for the ESTechnica Reflow Controller to the arduino IDE)
 * The code downloaded from github
 * USB to serial connection to ESTechnical Reflow Controller (the USB connection is installed in ESTechnical Reflow Ovens and the necessary parts are also provided in the full upgrade kits for DIY installation)


Obtaining the source code
====================

Use one of the following methods to get the source code:

Using GIT:
If you are familiar with git and want to use git to check out the source code, use the following command in a terminal:
(This will check out the source code from git into the current directory. I will cover no further use of git here.)

	git clone https://github.com/estechnical/reflowOvenController.git


Downloading a zip file:
	To download a zip file of the latest source code, visit https://github.com/estechnical/reflowOvenController and click on the
	'Download ZIP' button (in the right hand column of the page). Once the file has dowloaded, extract the zip archive to a location of your choice.


Installation
====================

To edit/compile the source code for the reflow controller, first install the arduino IDE - http://arduino.cc/en/Main/Software

Because the ESTechnical reflow controller uses a 20MHz crystal instead of a 16MHz crystal, this requires special build settings for the arduino IDE. 

Browse to your arduino installation directory, then browse to hardware/arduino/avr/boards.txt. Edit this file (using, for example Notepad or Programmer's Notepad http://www.pnotepad.org) and add the following block to the top of the file, taking care to not mess up the layout of the file:


	##############################################################

	estechnical20mhz.name=ESTechnical Reflow Controller
	
	estechnical20mhz.upload.tool=avrdude
	estechnical20mhz.upload.protocol=arduino

	estechnical20mhz.bootloader.tool=avrdude
	estechnical20mhz.bootloader.low_fuses=0xFF
	estechnical20mhz.bootloader.unlock_bits=0x3F
	estechnical20mhz.bootloader.lock_bits=0x0F

	estechnical20mhz.build.f_cpu=20000000L
	estechnical20mhz.build.board=AVR_DUEMILANOVE
	estechnical20mhz.build.core=arduino
	estechnical20mhz.build.variant=standard

	## Arduino Duemilanove or Diecimila w/ ATmega328
	## ---------------------------------------------
	estechnical20mhz.menu.cpu.atmega328=ATmega328

	estechnical20mhz.menu.cpu.atmega328.upload.maximum_size=30720
	estechnical20mhz.menu.cpu.atmega328.upload.maximum_data_size=2048
	estechnical20mhz.menu.cpu.atmega328.upload.speed=57600

	estechnical20mhz.menu.cpu.atmega328.bootloader.high_fuses=0xDA
	estechnical20mhz.menu.cpu.atmega328.bootloader.extended_fuses=0x05
	estechnical20mhz.menu.cpu.atmega328.bootloader.file=atmega/ATmegaBOOT_168_atmega328.hex

	estechnical20mhz.menu.cpu.atmega328.build.mcu=atmega328p

	##############################################################


Save the file and quit.

Setting up the source code to build firmware
====================

**Loading new firmware will erase prevously saved profile settings, be sure you make a note of important profile settings**

Locate the sketchbook directory that the arduino IDE created when installing, you can see the path to your sketchbook folder in the arduino preferences menu (File-> Preferences).

From the files downloaded and unzipped from github:

 * Copy the ReflowController directory from the zip file into the Sketchbook directory. 
 * Copy all the directories in the Libraries directory to the libraries directory in the arduino sketchbook directory.

Open the arduino IDE, open the ReflowController.ino sketch (using File->Open). 

Select the 'ESTechnical Reflow Controller' from the Tools->Board menu. 

Compile the firmware with Ctrl+R to test everything is installed correctly.

Connect the computer to the oven's USB port, choose the right serial port from Tools->Serial Port and then upload the code using Ctrl+U.

Once the uploading of code has completed, perform a factory reset of the controller using the menu on the oven controller.





