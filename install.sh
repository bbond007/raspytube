#!/bin/sh

# Install dependancies
echo "> Installing dependancies."
sudo apt-get install omxplayer wget libjpeg8 youtube-dl

# Updating youtube-dl to make sure it's the latest version.
echo "> Updating youtube-dl."
sudo youtube-dl -U

# Download binary
echo "> Downloading raspytube binary."
sudo wget https://github.com/bbond007/raspytube/raw/master/raspytube.bin -O /usr/local/bin/raspytube.bin
sudo chmod a+x /usr/local/bin/raspytube.bin

# Launch raspytube!
echo "> Launching raspytube!"
raspytube.bin