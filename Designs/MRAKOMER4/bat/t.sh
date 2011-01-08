while [ 1 ]; do
  tail -n 1 /mnt/USBdisk/log/$(date +%F)_M40.log 
  tail -n 1 /mnt/USBdisk/log/$(date +%F)_M41.log
  sleep 2
done