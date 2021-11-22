#! /bin/bash
# This script launches linux_ccsds_serial_driver demo

echo "This script launches linux_ccsds_serial_driver demo"

socat -x pty,link=/tmp/ttyVCOM0,raw pty,link=/tmp/ttyVCOM1,raw &

sleep 1

../../build/build/bin/LinuxApp

echo -e "\n\rDemo finished\n"

trap "pkill cat" EXIT INT TERM
