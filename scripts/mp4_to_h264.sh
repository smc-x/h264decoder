#!/bin/bash
if (( $# != 2 )); then
    echo "Usage: ./mp4_to_h264.sh {input.mp4} {output.h264}"
    exit 1
fi

INPUT=$1
OUTPUT=$2

ffmpeg -i ${INPUT} -vcodec copy -an -bsf:v h264_mp4toannexb ${OUTPUT}
