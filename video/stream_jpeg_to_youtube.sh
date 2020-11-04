#! /bin/bash
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

ffmpeg  -re \
    -loglevel debug -threads:v 4 -threads:a 8 -filter_threads 2 \
    -thread_queue_size 512 \
    -f lavfi -i anullsrc=channel_layout=stereo:sample_rate=44100 \
    -f v4l2 -i /dev/video0  -framerate $FPS \
    -tune zerolatency  \
    -vcodec libx264 -bf 2 -preset $QUAL -r $FPS -g $(($FPS * 2)) -b:v $VBR \
    -f flv "$YOUTUBE_URL/$KEY"


#ffmpeg -re \
#    -loglevel debug -threads:v 4 -threads:a 8 -filter_threads 2 \
#    -thread_queue_size 512 \
#    -f lavfi -i anullsrc=channel_layout=stereo:sample_rate=44100 \
#    -loop 1 -y -i test.jpg  -framerate $FPS \
#   -tune stillimage -tune zerolatency -s 1280x720 \
#    -vcodec libx264 -bf 2 -preset $QUAL -r $FPS -g $(($FPS * 2)) -b:v $VBR \
#    -acodec libmp3lame -ar 44100 -threads 6 -qscale 3 -b:a 712000 -bufsize 512k \
#    -f flv "$YOUTUBE_URL/$KEY"

