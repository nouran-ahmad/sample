- #### Tesseract OCR Engine
installing tesseract command

	sudo apt-get install tesseract-ocr
installing development lib command

	sudo apt-get install tesseract-ocr-dev
installing Arabic packages

	sudo apt-get install tesseract-ocr-ara
extracting text from images for Arabic language

	tesseract -l ara input-image output-text-file

- ### eSpeak-ng Text to Speech 
Source code:	https://github.com/linuxscout/espeak-ng

    sudo apt-get intall mbrola

- #### PulseAudio CLI
start pulse Audio server to enable the sound driver in Raspberry Pi
        
    pulseaudio &
    
- ##### espeak CLI: 

        espeak-ng  hello --stdout | paplay -d 0

        espeak-ng --voices=mb
 
        espeak-ng --voices=ar

        espeak-ng -v Arabic -s 150 -f out1.txt  --stdout | paplay

        espeak-ng -v mb-ar1 -s 110 -f out1.txt  --stdout | paplay

- #### espeak code edits: 
remove new line at mb-ar1 at /usr/local/share/espeak-ng-data/voices/mb

- https://stackoverflow.com/questions/47027953/undifined-reference-to-espeak-ng-headers-in-ubuntu

- https://www.peterbeard.co/blog/post/making-your-raspberry-pi-talk-to-you-with-espeak/

- sudo modprobe snd_bcm2835 (on rbp3 only)

-  speaker-test -c2
