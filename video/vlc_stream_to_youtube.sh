#!/bin/bash
#
# Diffusion youtube avec ffmpeg
# https://gist.github.com/olasd/9841772
# Configurer youtube avec une résolution 720p. La vidéo n'est pas scalée.


VBR="2500k"                                    # Bitrate de la vidéo en sortie
FPS="30"                                       # FPS de la vidéo en sortie
QUAL="medium"                                  # Preset de qualité FFMPEG
YOUTUBE_URL="rtmp://a.rtmp.youtube.com/live2"  # URL de base RTMP youtube

SOURCE="udp://239.255.139.0:1234"              # Source UDP (voir les annonces SAP)
KEY="qqws-psbp-3abp-6v5t-29e8"                 # Clé à récupérer sur l'event youtube



PATHTOFILE=./small.mp4
STREAMID=qqws-psbp-3abp-6v5t-29e8
VLC=cvlc

cvlc ./small.mp4 --sout '#transcode{vcodec=FLV1,acodec=mp3,samplerate=44100}:std{access=rtmp,mux=ffmpeg{mux=flv},dst=rtmp://a.rtmp.youtube.com/live2/qqws-psbp-3abp-6v5t-29e8'


