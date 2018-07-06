
echo installing tesseract:

sudo apt-get install tesseract-ocr
sudo apt-get install tesseract-ocr-dev
sudo apt-get install tesseract-ocr-ara

echo installing pulseAudio cli and libs:
sudo apt-get install pulseaudio
sudo apt-get install libasound2-dev
sudo apt-get install libpulse-dev

echo installing autotools:
sudo apt-get install make autoconf automake libtool pkg-config

echo cloning and installing espeak-ng:

git clone https://github.com/espeak-ng/espeak-ng.git
cd ./espeak-ng
./autogen.sh
./configure 
make
sudo make install

echo downloading mbrola binaries:
curl -O http://tcts.fpms.ac.be/synthesis/mbrola/bin/raspberri_pi/mbrola.tgz
tar -xvzf ./mbrola.tgz -C /usr/local/bin


