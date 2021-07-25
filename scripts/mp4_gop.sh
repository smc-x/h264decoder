#!/bin/bash
if (( $# != 3 )); then
    echo "Usage: ./mp4_gop.sh {input.mp4} {output.mp4} {iframes}"
    exit 1
fi

INPUT=$1
OUTPUT=$2
IF=$3

set -x
ffmpeg \
    -i ${INPUT} \
    -c:v libx264 \
    -g ${IF} -keyint_min ${IF} -bf 0 \
    ${OUTPUT}
