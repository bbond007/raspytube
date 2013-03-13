# raspytube - an OpenVG YouTube client for Raspberry Pi.

binarybond007@gmail.com

## Installation

```sh
# Install youtube-dl
sudo wget https://github.com/downloads/rg3/youtube-dl/youtube-dl -O /usr/local/bin/youtube-dl
sudo chmod a+x /usr/local/bin/youtube-dl

# Install compilation tools and required libraries
sudo apt-get update
sudo apt-get install build-essential git-core libjpeg8-dev libx11-dev

# Compile ilclient
cd /opt/vc/src/hello_pi/libs/ilclient
make clean
make

# Compile raspytube
cd ~
git clone https://github.com/bbond007/raspytube
cd raspytube
cd OMXJPEG
make clean
make
cd ..
cd GFXDATA
make clean
make
cd ..
make clean
make

# Launch raspytube
./raspytube.bin
```

## Usage

Keys:
		
	Q - run 1/4 screen in X11 window

	M - main menu

	S - search (edit existing txt)

	N - new search

	J - (uppercase) test joystick (useful for configuring JS) 

	j - (lowercase) joystick/keyboard/mouse menu (input menu)

	cursor down - next record

	cursor up - prev record

	cursor right- get next # results

	cursor left- get prev # results

	ESC - exit screen/program
 
The following are also available from the menu:
		
	C - category menu

	F - set youtube-dl format
		
	G - GUI Menu 

	P - toggle between OMXPlayer and MPlayer 

	H - toggle between HDMI/Local audio output

	j - joystick/keyboard/mouse menu (input menu)

	I - more information on selected record

	X - toggle between software (libjpeg) and hardware (OMX) jpeg decoding.

## Notes

		* OMX-jpg & gfx data broken out into seperate static libs (see updated compile instructions)
		
		* you need 64MB VRAM ro run this program.

		* MPlayer if you switch to windowed mode.  

		* Added mouse support.

		* Added support to run in X11 window! 

This application can now run in a X11 window. The app is not really 

running in X11 window, but it is a clever illusion. How I accomplish 

this is I create a dummy X11 window, track the movement of that window 

on the desktop then uses "vc_dispmanx_element_change_attributes" of 

the VC output to match. This has the added benefit of being able to 

take input focus and capture X11 input such as mouse move/enter events 

and keypresses. Apparently it is very expensive to call XGetWindowAttributes 

and XGetGeometry (that is actually how MC Hammer went broke), so I only 

do so periodically which is why you'll notice a slight lag when you move 

the window (before the VC window catches up). This is also why the window 

is always front-most. To switch to XWindows mode, There is an invisible 

"button" on right-topmost position to toggle on the screen or the "q" 

key (for quarter-screen). Repeat to return.

## Acknowledgements

Thanks the the RPi community!

parts of this come from "shapes.c":

shapes: minimal program to explore OpenVG

Anthony Starks (ajstarks@gmail.com)

and "test_image.c"

ShivaVG / IvanLeben<ivan.leben@gmail.com>

ShivaVG-anopen-sourceLGPLANSICimplementationoftheOpenVGspecification

Also thanks to MattOwnby and jumble) for OMX jpeg decode 