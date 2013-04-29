reflowOvenController
====================

ESTechnical Reflow Oven Controller source code

Copyright Ed Simmons 2013 - ESTechnical
ed@estechnical.co.uk
http://www.estechnical.co.uk


The ESTechnical treflow oven controller was designed to operate the T962, T962A and T962C ovens.

It operates using PID control of the heater and fan output to improve the control compared to the original manufacturer's controller.

ESTechnical sell the controller hardware ready to install, currently on eBay and very soon on the ESTechnical website http://www.estechnical.co.uk

To edit/compile/program the controller, first install the arduino IDE. 

The ESTechnical reflow controller uses a 20MHz crystal instead of a 16MHz crystal, this requires special build settings for the arduino IDE. Browse to your arduino installation directory, then browse to hardware/arduino/boards.txt. Edit this file and add the following block to the file:


##############################################################

atmega328_20MHz.name=ESTechnical Reflow controller

atmega328_20MHz.upload.protocol=stk500
atmega328_20MHz.upload.maximum_size=30720
atmega328_20MHz.upload.speed=57600

atmega328_20MHz.bootloader.low_fuses=0xFF
atmega328_20MHz.bootloader.high_fuses=0xDA
atmega328_20MHz.bootloader.extended_fuses=0x05
atmega328_20MHz.bootloader.path=atmega
atmega328_20MHz.bootloader.file=ATmegaBOOT_168_atmega328.hex
atmega328_20MHz.bootloader.unlock_bits=0x3F
atmega328_20MHz.bootloader.lock_bits=0x0F

atmega328_20MHz.build.mcu=atmega328p
atmega328_20MHz.build.f_cpu=20000000L
atmega328_20MHz.build.core=arduino
atmega328_20MHz.build.variant=standard


##############################################################


Save the file and quit. 


Copy the ReflowController directory to your arduino sketchbook directory.
Copy all the folders in Libraries to the Libraries directory in your sketchbook.

Open the arduino ide, open the ReflowController.pde sketch. Select the right kind of hardware from the Tools->Board menu. Compile this with Ctrl+R to test everything is installed correctly. Choose the right serial port from Tools->Serial Port and then upload the code using Ctrl+U.





