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

    // Get the specific export codec
    decoder->exp_codec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
    if (!decoder->exp_codec) {
        return "cannot find encoder";
    }

    // Allocate export context
    decoder->exp_context = avcodec_alloc_context3(decoder->exp_codec);
    if (!decoder->exp_context) {
        return "cannot allocate export context";
    }

    // Mark un-initialized export context
    decoder->exp_init = 0;

    // Everything is good
    *code = 0;
    return "";
}

void h264decoder_free(h264decoder* decoder) {
    if (decoder->exp_init) {
        avcodec_close(decoder->exp_context);
    }
    avcodec_free_context(&decoder->exp_context);

    av_parser_close(decoder->parser);

    avcodec_close(decoder->context);
    avcodec_free_context(&decoder->context);

    av_frame_free(&decoder->frame);
    av_packet_free(&decoder->pkt);
}

int h264decoder_parse(h264decoder* decoder, const uint8_t* in_data, size_t in_size) {
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

int h264decoder_decode(h264decoder* decoder, int* is_keyframe) {
    int err;

    err = avcodec_send_packet(decoder->context, decoder->pkt);
    if (err < 0) {
        return err;
    }
    err = avcodec_receive_frame(decoder->context, decoder->frame);
    if (err < 0) {
        return err;
    }

    *is_keyframe = decoder->frame->key_frame;
    return 0;
}

int h264decoder_frame_to_jpeg(h264decoder* decoder, uint8_t** data, size_t* size) {
    int err;

    if (!decoder->exp_init) {
        decoder->exp_context->time_base.den = decoder->context->time_base.den;
        decoder->exp_context->time_base.num = decoder->context->time_base.num;
        decoder->exp_context->pix_fmt = AV_PIX_FMT_YUVJ420P;
        decoder->exp_context->height = decoder->frame->height;
        decoder->exp_context->width = decoder->frame->width;
        err = avcodec_open2(decoder->exp_context, decoder->exp_codec, NULL);
        if (err < 0) {
            return err;
        }
        decoder->exp_init = 1;
    }

    // Allocate a packet for storing the encoded frame
    AVPacket* packet = av_packet_alloc();
    if (!packet) {
        return -1;
    }

    // Encode the frame
    err = avcodec_send_frame(decoder->exp_context, decoder->frame);
    if (err < 0) {
        return err;
    }
    err = avcodec_receive_packet(decoder->exp_context, packet);
    if (err < 0) {
        return err;
    }

    // Copy the data if got frame
    if (packet->size) {
        *size = packet->size;
        *data = malloc(packet->size);
        memcpy(*data, packet->data, packet->size);
    } else {
        *size = 0;
        *data = NULL;
    }

    // Free the packet
    av_packet_unref(packet);
    return 0;
}
