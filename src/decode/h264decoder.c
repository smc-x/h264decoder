#include "h264decoder.h"

const char* h264decoder_init(h264decoder* decoder, int* code) {
    // Assume there will be some error
    *code = -1;

    // Get the specific codec
    decoder->codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!decoder->codec) {
        return "cannot find decoder";
    }

    // Allocate and initialize context
    decoder->context = avcodec_alloc_context3(decoder->codec);
    if (!decoder->context) {
        return "cannot allocate context";
    }
    if (decoder->codec->capabilities & AV_CODEC_CAP_TRUNCATED) {
        decoder->context->flags |= AV_CODEC_FLAG_TRUNCATED;
    }
    int err = avcodec_open2(decoder->context, decoder->codec, NULL);
    if (err < 0) {
        return "cannot open context";
    }

    // Initialize the specific parser context
    decoder->parser = av_parser_init(AV_CODEC_ID_H264);
    if (!decoder->parser) {
        return "cannot init parser";
    }

    // Allocate a frame
    decoder->frame = av_frame_alloc();
    if (!decoder->frame) {
        return "cannot allocate frame";
    }

    // Allocate a packet
    decoder->pkt = av_packet_alloc();
    if (!decoder->pkt) {
        return "cannot allocate packet";
    }

    // Everything is good
    *code = 0;
    return "";
}

void h264decoder_free(h264decoder* decoder) {
    av_parser_close(decoder->parser);
    avcodec_close(decoder->context);
    av_free(decoder->context);
    av_frame_free(&decoder->frame);
    av_packet_free(&decoder->pkt);
}

int h264decoder_parse(h264decoder* decoder, const uint8_t* in_data, int in_size) {
    return av_parser_parse2(
        decoder->parser, decoder->context,
        &decoder->pkt->data, &decoder->pkt->size,
        in_data, in_size,
        0, 0, AV_NOPTS_VALUE
    );
}

int h264decoder_available(h264decoder* decoder) {
    return decoder->pkt->size > 0;
}

int h264decoder_decode(h264decoder* decoder) {
    int ret = avcodec_send_packet(decoder->context, decoder->pkt);
    if (ret == 0) {
        ret = avcodec_receive_frame(decoder->context, decoder->frame);
        if (ret == 0) {
            return 0;
        }
    }
    return -1;
}
