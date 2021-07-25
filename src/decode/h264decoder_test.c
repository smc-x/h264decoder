#include <stdio.h>
#include <stdlib.h>

#include <libavcodec/avcodec.h>

#include "h264decoder.h"



const size_t buffer_size = 1024;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("usage: {program_name} {path_to_h264_file}\n");
        return -1;
    }

    // Allocate buffer
    char buffer[buffer_size];

    // Open file
    const char* filename = argv[1];
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        printf("failed opening file %s\n", filename);
        return -1;
    }

    // Create decoder
    h264decoder decoder;
    h264decoder* decoder_ptr = &decoder;

    // Initialize decoder
    int code;
    const char* hint = h264decoder_init(decoder_ptr, &code);
    if (code != 0) {
        printf("failed initializing the decoder: %s\n", hint);
        fclose(fp);
        return -1;
    }

    // Decode video
    size_t nread, nparsed;
    size_t nframes = 0;
    while (1) {
        nread = fread(buffer, 1, buffer_size, fp);
        if (nread == 0) {
            break;
        }

        const char* data = &buffer[0];
        while (nread > 0) {
            nparsed = h264decoder_parse(decoder_ptr, data, nread);
            nread -= nparsed;
            data += nparsed;
            if (h264decoder_available(decoder_ptr)) {
                h264decoder_decode(decoder_ptr);
                nframes++;

                uint8_t* image = NULL;
                size_t image_size = 0;
                h264decoder_frame_to_jpeg(decoder_ptr, &image, &image_size);
                if (image_size) {
                    char filename[20];
                    sprintf(filename, "out%03d.jpg", nframes);
                    FILE* fp_write = fopen(filename, "wb");
                    fwrite(image, 1, image_size, fp_write);
                    fclose(fp_write);
                }

                printf("frame decoded: %d, keyframe: %d\n", nframes, decoder_ptr->frame->key_frame);
            }
        }
    };

    // Cleanup
    h264decoder_free(decoder_ptr);
    fclose(fp);
    return 0;
}
