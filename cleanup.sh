rm -rf *~
rm -rf DEADJOE
rm -rf ~.old
rm -rf *log
rm -rf *orig
make clean
make 
strip raspytube.bin
rm -rf raspytube.bin.zip
zip raspytube.bin.zip raspytube.bin
git add cleanup.sh README.md Makefile *.c *.h *.cpp *.inc raspytube.bin raspytube.bin.zip
git add screenshots/*.jpg 
git commit 
git push origin master

