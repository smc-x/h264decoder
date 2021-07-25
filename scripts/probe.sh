#!/bin/bash
if (( $# != 1 )); then
    echo "Usage: ./probe.sh {input.h264}"
    exit 1
fi

INPUT=$1

ffprobe -i ${INPUT} -show_frames
