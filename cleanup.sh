rm -rf *~
rm -rf DEADJOE
rm -rf ~.old
make clean
make 
strip raspytube.bin
zip raspytube.bin.zip raspytube.bin

