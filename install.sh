#!/bin/sh

# Install youtube-dl. Use github to make sure it's the latest version.
# Check for /usr/bin/youtube-dl (from package manager install) and
# /usr/local/bin/youtube-dl (from previous repo based install)
if [ ! -f /usr/bin/youtube-dl -a ! -f /usr/local/bin/youtube-dl ];
then
    echo "> Installing youtube-dl."
    sudo wget https://github.com/downloads/rg3/youtube-dl/youtube-dl -O /usr/local/bin/youtube-dl
    sudo chmod a+x /usr/local/bin/youtube-dl
else
    echo "> Updating youtube-dl."
    sudo youtube-dl -U
fi

# Install dependancies
echo "> Installing dependancies."
sudo apt-get install omxplayer wget libjpeg8

# Download binary
echo "> Downloading raspytube binary."
sudo wget https://github.com/bbond007/raspytube/raw/master/raspytube.bin -O /usr/local/bin/raspytube.bin
sudo chmod a+x /usr/local/bin/raspytube.bin

# Launch raspytube!
echo "> Launching raspytube!"
raspytube.bin