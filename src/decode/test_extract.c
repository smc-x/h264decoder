#include <stdio.h>
#include <stdlib.h>

#include "h264decoder.h"

const size_t BUFFER_SIZE = 1024;

int main(int argc, char* argv[]) {
    if (argc != 2 && argc != 3) {
        printf("usage: {program_name} {path_to_h264_file} [no_output]\n");
        return -1;
    }

    int no_output = 0;
    if (argc == 3) {
        no_output = 1;
    }

    // Allocate buffer
    char buffer[BUFFER_SIZE];

    // Open file
    const char* v_filename = argv[1];
    FILE* v_fp = fopen(v_filename, "rb");
    if (!v_fp) {
        printf("failed opening file %s\n", v_filename);
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
        fclose(v_fp);
        return -1;
    }

    // Workaround for decoding the tail frame
    char fake_bytes[5];
    int fake_initialized = 0;
    int fake_injected = 0;

    // Decode video
    size_t nread, nparsed;
    size_t nframes = 0;
    int is_keyframe = 0;
    const char* data;
    while (1) {
        nread = fread(buffer, 1, BUFFER_SIZE, v_fp);

        if (nread == 0) {
            if (fake_injected) {
                break;
            } else {
                data = &fake_bytes[0];
                nread = 5;
                fake_injected = 1;
            }
        } else {
            if (!fake_initialized) {
                memcpy(fake_bytes, buffer, 5);
                fake_initialized = 1;
            }
            data = &buffer[0];
        }

        while (nread > 0) {
            nparsed = h264decoder_parse(decoder_ptr, data, nread);
            nread -= nparsed;
            data += nparsed;
            printf("nparsed: %d\n", nparsed);

            if (h264decoder_available(decoder_ptr)) {
                h264decoder_decode(decoder_ptr, &is_keyframe);

                if (!no_output) {
                    uint8_t* image = NULL;
                    size_t image_size = 0;
                    h264decoder_frame_to_jpeg(decoder_ptr, &image, &image_size);
                    if (image_size) {
                        char i_filename[20];
                        sprintf(i_filename, "out%03d.jpg", nframes);
                        FILE* i_fp = fopen(i_filename, "wb");
                        fwrite(image, 1, image_size, i_fp);
                        fclose(i_fp);
                    }
                }

                printf("frame decoded: %d, keyframe: %d\n", nframes, is_keyframe);
                nframes++;
            }
        }
    };

    // Cleanup
    h264decoder_free(decoder_ptr);
    fclose(v_fp);
    return 0;
}
