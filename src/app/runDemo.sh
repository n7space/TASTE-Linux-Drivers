#! /bin/bash
# This script launches linux_ccsds_serial_driver demo

echo "This script launches linux_ccsds_serial_driver demo"

sudo socat pty,link=/dev/ttyVCOM0,raw pty,link=/dev/ttyVCOM1,raw &

sleep 1

cat /dev/ttyVCOM0 > logVCOM0.txt &
cat /dev/ttyVCOM1 > logVCOM1.txt &
sudo ../../build/build/bin/LinuxApp

echo -e "\n\rDemo finished\n"

echo -e "\n\r/dev/ttyVCOM0 contains:"
cat logVCOM0.txt

echo -e "\n\r/dev/ttyVCOM1 contains:"
cat logVCOM1.txt

trap "pkill cat" EXIT INT TERM
