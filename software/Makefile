CC = gcc
CXX = g++
#CFLAGS        = -pipe -O2 -Wall -W -D_REENTRANT -lpthread -lLEPTON_SDK -L/usr/lib/arm-linux-gnueabihf -L../raspberrypi_libs/leptonSDKEmb32PUB/Debug
CXXFLAGS      = -pipe -O2 -Wall -W -D_REENTRANT -lpthread -L/usr/lib/arm-linux-gnueabihf 
#INCPATH = -I. -I../raspberrypi_libs 

all: v4l2_dma_camera

#Palettes.o: Palettes.cpp Palettes.h
#	${CXX} -c ${CXXFLAGS} ${INCPATH} -o Palettes.o Palettes.cpp

v4l2_dma_camera: v4l2_dma_camera.o leptsci.o Palettes.o SPI.o
	${CXX} -o v4l2_dma_camera v4l2_dma_camera.c ${CXXFLAGS}

clean:
	rm -f v4l2_dma_camera.o v4l2_dma_camera