echo /dev/ttyUSB$1
echo uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu > /dev/ttyUSB$1
echo u
sleep 5
echo uf > /dev/ttyUSB$1
echo uf
sleep 5
ascii-xfr -s -v -c1 ../SW/HEX/irmrak4.hex > /dev/ttyUSB$1
