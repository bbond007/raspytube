#!/bin/sh

# Install youtube-dl. Use github to make sure it's the latest version.
if [ ! -f /usr/local/bin/youtube-dl ];
then
    echo "> Installing youtube-dl."
    sudo wget https://github.com/downloads/rg3/youtube-dl/youtube-dl -O /usr/local/bin/youtube-dl
    sudo chmod a+x /usr/local/bin/youtube-dl
else
    echo "> Skipping installing youtube-dl, already installed."
fi

# Install dependancies
echo "> Installing dependancies."
sudo apt-get install omxplayer wget libjpeg8

# Download binary
echo "> Downloading raspytube binary."
sudo wget https://github.com/bbond007/raspytube/blob/master/raspytube.bin -O /usr/local/bin/raspytube.bin
chmod a+x /usr/local/bin/raspytube.bin

# Launch raspytube!
echo "> Launching raspytube!"
raspytube.bin