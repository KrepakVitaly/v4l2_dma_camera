CC = gcc
CXX = g++
#CFLAGS        = -pipe -O2 -Wall -W -D_REENTRANT -lpthread -lLEPTON_SDK -L/usr/lib/arm-linux-gnueabihf -L../raspberrypi_libs/leptonSDKEmb32PUB/Debug
CXXFLAGS      = -pipe -O2 -Wall -W -D_REENTRANT -lpthread -L/usr/lib/gcc/aarch64-linux-gnu/7
INCPATH = -I. -I../raspberrypi_libs 

all: v4l2_dma_camera

#Palettes.o: Palettes.cpp Palettes.h
#	${CXX} -c ${CXXFLAGS} ${INCPATH} -o Palettes.o Palettes.cpp

v4l2_dma_camera: v4l2_dma_camera.o
	${CXX} -o v4l2_dma_camera v4l2_dma_camera.cpp ${CXXFLAGS}

refactored_camera: main.o
	${CXX} -o main main.cpp ${CXXFLAGS}

clean:
	rm -f v4l2_dma_camera.o v4l2_dma_camera
