* TESSERACT

	sudo apt-get install tesseract-ocr
	sudo apt-get install tesseract-ocr-dev
	sudo apt-get install tesseract-ocr-ara
	
	tesseract -l ara input-image output-text-file

* ESPEAK-NG
	https://github.com/linuxscout/espeak-ng

- sudo apt-get intall mbrola

- pulseaudio &

- espeak-ng  hello --stdout | paplay -d 0
- espeak-ng --voices=mb
- espeak-ng --voices=ar
- 

- espeak-ng -v Arabic -s 150 -f out1.txt  --stdout | paplay
- espeak-ng -v mb-ar1 -s 110 -f out1.txt  --stdout | paplay


remove new line at mb-ar1 at /usr/local/share/espeak-ng-data/voices/mb

- https://stackoverflow.com/questions/47027953/undifined-reference-to-espeak-ng-headers-in-ubuntu

- https://www.peterbeard.co/blog/post/making-your-raspberry-pi-talk-to-you-with-espeak/

- sudo modprobe snd_bcm2835 (on rbp3 only)

-  speaker-test -c2
