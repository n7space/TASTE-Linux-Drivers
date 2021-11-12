#!/bin/bash

PREFIX=/home/taste/tool-inst
SOURCES=$(dirname $0)

mkdir -p "${PREFIX}/include/TASTE-Linux-Drivers/src"
rm -rf "${PREFIX}/include/TASTE-Linux-Drivers/src/*"
cp -r "${SOURCES}/src/linux_ip_socket" "${PREFIX}/include/TASTE-Linux-Drivers/src"
cp -r "${SOURCES}/src/linux_serial_ccsds" "${PREFIX}/include/TASTE-Linux-Drivers/src"
cp -r "${SOURCES}/configurations" "${PREFIX}/include/TASTE-Linux-Drivers/configurations"
