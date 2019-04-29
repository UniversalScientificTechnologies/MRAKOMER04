stty -F /dev/ttyUSB$1 2400
echo -n u > /dev/ttyUSB$1
echo u
sleep 2
echo -n uf > /dev/ttyUSB$1
echo uf
sleep 2
ascii-xfr -s -v -c1 ../SW/HEX/irmrak4.hex > /dev/ttyUSB$1
