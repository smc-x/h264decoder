smc-x/h264decoder
=================

**CAVEAT**

The reduced branch is not for general purposes, but instead, we have it crafted
for demonstrating the following functions with FFmpeg:
- convert a common mp4 clip to one w/ fixed-sized gops and w/o b-frames,
- unwrap the h264 part from a common mp4,
- split an h264 file to gops,
- extract frames (as jpegs) from a gop.

During the initial implementation of the above functions, we had referred to
quite a lot of online materials, without which we couldn't have realized the
functions (unfortunately, it is hard to list all of them here).

However, the above functions are not necessarily achieved in a "correct" way,
nor in the most efficient way.

We have just validated them against a few mp4 clips and compared the consumed
time with `ffprobe` when applicable.

## Convert a common mp4 clip to one w/ fixed-sized gops and w/o b-frames

```bash
# start from repo home
cd scripts
# usage: ./mp4_gop.sh {input.mp4} {output.mp4} {iframes}
./mp4_gop.sh ./sample.mp4 ./sample_i1.mp4 1
./mp4_gop.sh ./sample.mp4 ./sample_i24.mp4 24
```

## Unwrap the h264 part from a common mp4

```bash
# start from repo home
cd scripts
# usage: ./mp4_to_h264.sh {input.mp4} {output.h264}
./mp4_to_h264.sh ./sample_i1.mp4 ./sample_i1.h264
./mp4_to_h264.sh ./sample_i24.mp4 ./sample_i24.h264
```

## Split an h264 file to gops

```bash
# start from repo home
cd src/decode
gcc \
  -I../../include \
  -l avcodec -l avutil -l swscale \
  h264decoder*.c ./test_gop.c \
  -o test_gop
./test_gop ../../scripts/sample_i24.h264
```

## Extract frames (as jpegs) from a gop

```bash
# start from repo home
cd src/decode
gcc \
  -I../../include \
  -l avcodec -l avutil -l swscale \
  h264decoder*.c ./test_extract.c \
  -o test_extract
./test_extract ./gop000.h264
```

## Timing

| Command | Elapsed | Details |
|:--------|:--------|:--------|
| `./test_gop ./sample_i24.h264`              | 0.36s | user 0.02s system 99% cpu 0.385 total |
| `./test_extract ./sample_i24.h264`          | 0.84s | user 0.07s system 99% cpu 0.915 total |
| `./test_extract ./sample_i24.h264 nooutput` | 0.38s | user 0.02s system 99% cpu 0.398 total |
| `./probe.sh ./sample_i24.h264`              | 0.42s | user 0.04s system 99% cpu 0.460 total |
