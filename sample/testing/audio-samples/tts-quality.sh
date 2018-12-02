for i in 1 2 3 4 5 6 7 8 9
do
  echo "processing ... number $i"
  espeak-ng -v mb-ar2 -s 120 -f $i.txt  -w $i.wav
done

