#ifndef _INCL_H264DECODER
#define _INCL_H264DECODER

#include <libavcodec/avcodec.h>

/**
 * h264decoder encapsulates variables required to decode an h264 video.
 */
typedef struct h264decoder {
    /* for video decoding */
    AVCodec* codec;
    AVCodecContext* context;
    AVCodecParserContext* parser;

    AVFrame* frame;
    AVPacket* pkt;

    /* for image exporting */
    AVCodec* exp_codec;
    AVCodecContext* exp_context;
    int exp_init;
} h264decoder;

/**
 * h264decoder_init initializes an h264decoder struct.
 * @param[in] decoder the h264decoder to initialize
 * @param[out] code status code
 * @return empty string if code is 0, the error string otherwise
 */
const char* h264decoder_init(h264decoder* decoder, int* code);

/**
 * h264decoder_free frees an h264decoder struct.
 * @param[in] decoder the h264decoder to free
 */
void h264decoder_free(h264decoder* decoder);

/**
 * h264decoder_parse parses the given data.
 * @param[in] decoder the h264decoder to use
 * @param[in] in_data the data to parse
 * @param[in] in_size the data size to parse
 * @return the number of parsed bytes
 */
int h264decoder_parse(h264decoder* decoder, const uint8_t* in_data, int in_size);

/**
 * h264decoder_available tests if there is a ready frame.
 * @param[in] decoder the h264decoder to use
 * @return 0 if there is no ready frame, a positive integer otherwise
 */
int h264decoder_available(h264decoder* decoder);

/**
 * h264decoder_decode decodes a frame. On success, the frame can be accessed via the decoder.
 * @param[in] decoder the h264decoder to use
 * @return 0 on success, a negative integer otherwise
 */
int h264decoder_decode(h264decoder* decoder);

/**
 * h264decoder_frame_to_jpeg encodes a frame to jpeg for exporting.
 * @param[in] decoder the h264decoder to use
 * @param[out] data the encoded image
 * @param[out] size the encoded image size
 * @return 0 on success, a negative integer otherwise
 */
int h264decoder_frame_to_jpeg(h264decoder* decoder, uint8_t** data, size_t* size);

#endif
