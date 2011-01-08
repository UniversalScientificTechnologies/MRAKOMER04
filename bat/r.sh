stty -F /dev/ttyUSB0 igncr -echo onocr icrnl speed 2400
stty -F /dev/ttyUSB1 igncr -echo onocr icrnl speed 2400
echo aaaaaaaaaaa > /dev/ttyUSB0
echo aaaaaaaaaaa > /dev/ttyUSB1
cat /dev/ttyUSB0 | /mnt/USBdisk/logger.sh M40 &
cat /dev/ttyUSB1 | /mnt/USBdisk/logger.sh M41 &

