while [ 1 ]; do
  read line
  echo $(date +%s%t%F%t%X) $line >> /mnt/USBdisk/log/$(date +%F)_$1.log
done