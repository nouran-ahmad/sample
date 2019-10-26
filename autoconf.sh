

sudo apt-get install make autoconf automake libtool pkg-config cmake
sudo apt-get install libpng-dev libjpeg8-dev install libtiff5-dev zlib1g-dev
sudo apt-get install libicu-dev libpango1.0-dev libcairo2-dev libleptonica-dev

cd /home/pi/Desktop/project/sample/dependencies

echo installing leptonica lib:
echo ==============================
#curl -O http://leptonica.org/source/leptonica-1.76.0.tar.gz
#tar -xvzf ./leptonica-1.76.0.tar.gz
./leptonica-1.76.0/configure
make
sudo make install

echo installing tesseract:
echo ==============================

git clone https://github.com/tesseract-ocr/tesseract.git
cd ./tesseract
git reset --hard 18787ea12b2ea9368c8e1c0128d1f8aef2beebc8 
./autogen.sh
./configure
make
sudo make install
sudo ldconfig
make training
sudo make install training-install
cd ..

echo installing pulseAudio cli and libs:
echo ==============================

sudo apt-get install pulseaudio libasound2-dev libpulse-dev

git clone https://github.com/rhdunn/pcaudiolib.git
cd ./pcaudiolib
./autogen.sh
./configure --prefix=/usr
make
sudo make install
cd ..

echo installing autotools:
echo ==============================

echo cloning and installing espeak-ng:
echo ==============================
git clone https://github.com/espeak-ng/espeak-ng.git
cd ./espeak-ng
git reset --hard 3ed34d3a74eb69cedcdfa98dbedef4359279d352
./autogen.sh
./configure 
make
sudo make install
cd ..

echo downloading mbrola binaries:
echo ==============================
curl -O http://tcts.fpms.ac.be/synthesis/mbrola/bin/raspberri_pi/mbrola.tgz
sudo tar -xvzf ./mbrola.tgz -C /usr/local/bin

sudo mkdir /usr/share/mbrola
 
cp ./mbrola/ar1 -r /usr/share/mbrola
cp ./mbrola/ar2 -r /usr/share/mbrola

echo installing opencv:
================
git clone https://github.com/opencv/opencv.git
cd ./opencv
git reset --hard 68942affdbc4677aa845bc4307d4752182324a0e
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local ..
make
sudo make install
sudo ldconfig
cd ../..

echo installing raspicam lib:
echo ==============================
cd ./raspicam-0.1.6
#sudo apt-get install unzip
#unzip raspicam-0.1.6.zip

mkdir build
cd build
cmake ..
make
sudo make install
sudo ldconfig
