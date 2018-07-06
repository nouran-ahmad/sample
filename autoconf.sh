
echo installing tesseract:
echo ==============================

sudo apt-get install tesseract-ocr
sudo apt-get install tesseract-ocr-dev
sudo apt-get install tesseract-ocr-ara

echo installing leptonica lib:
echo ==============================
curl -O http://leptonica.org/source/leptonica-1.76.0.tar.gz
tar -xvzf ./leptonica-1.76.0.tar.gz
cd ./leptonica-1.76.0
./configure
make
sudo make install

echo installing pulseAudio cli and libs:
echo ==============================

sudo apt-get install pulseaudio
sudo apt-get install libasound2-dev
sudo apt-get install libpulse-dev

echo installing autotools:
echo ==============================

sudo apt-get install make autoconf automake libtool pkg-config

echo cloning and installing espeak-ng:
echo ==============================
git clone https://github.com/espeak-ng/espeak-ng.git
cd ./espeak-ng
./autogen.sh
./configure 
make
sudo make install

echo downloading mbrola binaries:
curl -O http://tcts.fpms.ac.be/synthesis/mbrola/bin/raspberri_pi/mbrola.tgz
sudo tar -xvzf ./mbrola.tgz -C /usr/local/bin

sudo mkdir /usr/share/mbrola
 
cp ./mbrola/ar1 -r /usr/share/mbrola
cp ./mbrola/ar2 -r /usr/share/mbrola
