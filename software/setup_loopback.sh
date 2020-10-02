#!/bin/bash

echo "Enable V4L2 Loopback device"

modprobe v4l2loopback
v4l2-ctl -d /dev/video0 --all
#./v4l2lepton /dev/video0 &

