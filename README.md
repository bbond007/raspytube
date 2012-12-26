raspytube - an OpenVG YouTube client for Raspberry Pi.

keys:
		
		Q - run 1/4 screen in X11 window

		M - main menu
 
		S- search (edit existing txt)
 
		N - new search
 
		J - test joystick (useful for configuring JS)
 
		cursor down - next record
 
		cursor up - prev record
 
		cursor right- get next # results
 
		cursor left- get prev # results
 
		ESC - exit screen/proram
 
the following are also available from the menu:

		F - set youtube-dl format
 
		P- toggle between OMXPlayer and MPlayer 
 
		H- toggle between HDMI/Local audio output
 
		I- more information on selected record
 
		X- toggle between software (libjpeg) and hardware (OMX) jpeg decoding.
 
binarybond007@gmail.com

NOTES:
		
		* you need 64MB VRAM ro run this program.

		* MPlayer not working yet... mplayer still not working....

		* Added mouse support.

		* Added support to run in X11 window! 

This application can now run in a X11 window. The app is not really 

running in X11 window, but it is a clever illusion. How I accomplish 

this is I create a dummy X11 window, track the movement of that window 

on the desktop then uses "vc_dispmanx_element_change_attributes" of 

the VC output to match. This has the added benefit of being able to 

take input focus and capture X11 input such as mouse move/enter events 

and keypresses. Apparently it is very expensive to call 

XGetWindowAttributes and XGetGeometry (that is actually how MC Hammer 

went broke), so I only do so periodically which is why you'll notice a 

slight lag when you move the window (before the VC window catches up). 

This is also why the window is always front-most. To switch to XWindows 

mode, There is an invisible "button" on right-topmost position to toggle 

on the screen or the "q" key (for quarter-screen).   


Thanks the the RPi community!


		parts of this come from "shapes.c":

		shapes: minimal program to explore OpenVG

		Anthony Starks (ajstarks@gmail.com)

		and "test_image.c"
		
		ShivaVG / IvanLeben<ivan.leben@gmail.com>
		
		ShivaVG-anopen-sourceLGPLANSICimplementationoftheOpenVGspecification

		Also thanks to MattOwnby and jumble) for OMX jpeg decode 


Compile instructions:

		sudo wget https://github.com/downloads/rg3/youtube-dl/youtube-dl -O /usr/local/bin/youtube-dl

		sudo chmod a+x /usr/local/bin/youtube-dl

		sudo apt-get update

		sudo apt-get install build-essential git-core libjpeg8-dev

		cd /opt/vc/src/hello_pi/libs/ilclient

		make

		cd ~

		git clone https://github.com/bbond007/raspytube

		cd raspytube

		make

		./raspytube.bin

 
