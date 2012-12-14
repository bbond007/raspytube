raspytube

an OpenVG YouTube client for Raspberry Pi.

keys:

 M - main menu
 S - search (edit existing txt)
 N - new search
 cursor down - next record
 cursor up - prev record
 cursor right- get next 10 results
 cursor left- get prev 10 results
 ESC - exit screen/proram
 
the following are also available from the menu:
 F - set youtube-dl format
 P- toggle between OMXPlayer and MPlayer 
 H- toggle between HDMI/Local audio output
 I- more information on selected record
 X- toggle between software (libjpeg) and hardware (OMX) jpeg decoding.
 
binarybond007@gmail.com

NOTES:

 MPlayer not working yet... 
 
Thanks the the RPi community!

// parts of this come from "shapes.c":
// shapes: minimal program to explore OpenVG
// Anthony Starks (ajstarks@gmail.com)
// and "test_image.c"
// ShivaVG / IvanLeben<ivan.leben@gmail.com>
// ShivaVG-anopen-sourceLGPLANSICimplementationoftheOpenVGspecification
// Also thanks to MattOwnby and jumble) for jpeg decode 


Compile instructions:

sudo wget https://github.com/downloads/rg3/youtube-dl/youtube-dl -O /usr/local/bin/youtube-dl
sudo chmod a+x /usr/local/bin/youtube-dl
sudo apt-get update
sudo apt-get install build-essential git-core libjpeg8-dev
git clone https://github.com/bbond007/raspytube
cd raspytube
make
./raspytube.bin

 
