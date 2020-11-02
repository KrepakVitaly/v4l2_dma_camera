CC = gcc
CXX = g++
#CFLAGS        = -pipe -O2 -Wall -W -D_REENTRANT -lpthread -lLEPTON_SDK -L/usr/lib/arm-linux-gnueabihf -L../raspberrypi_libs/leptonSDKEmb32PUB/Debug
CXXFLAGS      = -pipe -O2 -Wall -W -D_REENTRANT -lpthread -L/usr/lib/gcc/aarch64-linux-gnu/7
INCPATH = -I. -I../raspberrypi_libs 

all: v4l2_dma_camera

#Palettes.o: Palettes.cpp Palettes.h
#	${CXX} -c ${CXXFLAGS} ${INCPATH} -o Palettes.o Palettes.cpp

tools.o: tools.h tools.cpp
	${CXX} -c ${CXXFLAGS} ${INCPATH} -o tools.o tools.cpp

v4l2_camera.o: v4l2_camera.h v4l2_camera.cpp
	${CXX} -c ${CXXFLAGS} ${INCPATH} -o v4l2_camera.o v4l2_camera.cpp

xdma_camera.o: xdma_camera.h xdma_camera.cpp
	${CXX} -c ${CXXFLAGS} ${INCPATH} -o xdma_camera.o xdma_camera.cpp

v4l2_dma_camera: xdma_camera.o v4l2_camera.o tools.o
	${CXX} -o v4l2_dma_camera main.o xdma_camera.o v4l2_camera.o tools.o ${CXXFLAGS}

clean:
	rm -f v4l2_dma_camera.o v4l2_dma_camera main main.o xdma_camera xdma_camera.o v4l2_camera v4l2_camera.o tools tools.o
