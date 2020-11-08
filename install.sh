#! /bin/bash
#
# Install v4l2-loopback xdma Xilinx driver

if [ -n "$1" ]
then
        echo "Cloning sources....."
        (cd ~/ && git clone https://github.com/umlaeute/v4l2loopback.git)
        (cd ~/v4l2loopback && git checkout 99cc2968f2f3125ff16f67ff7087d55223433a02)


        echo "Apply 12-bit patch....."
        cp ./v4l2loopback.patch ~/v4l2loopback/
        (cd ~/v4l2loopback && git apply ./v4l2loopback.patch)

        echo "Building v4l2loopback....."
        (cd ~/v4l2loopback && make && sudo make install)

        echo "Installing packages...."
        (sudo apt-get update)
        (sudo apt-get install v4l2loopback-utils)
fi
        echo "Modprobe.....v4l2loopback"
        (cd ~/v4l2loopback && sudo make modprobe)

        echo "Modprobe.....obsolete_xdma"
        (cd ./obsolete_xdma/Xilinx_Answer_65444_Linux_Files/tests && sudo bash ./load_driver.sh)

        echo "Make .....v4l2_dma_camera"
        (cd ./software && make clean && make )

exit
