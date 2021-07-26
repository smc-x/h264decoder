#include <stdio.h>
#include <stdlib.h>

#include "h264decoder.h"

#define BUFFER_SIZE 1024
#define GOP_MAX_SIZE  2 * 1024 * 1024

char gop_buffer[GOP_MAX_SIZE];

void save_gop(size_t gop_len, int ngops) {
    char gop_filename[20];
    sprintf(gop_filename, "gop%03d.h264", ngops);

    FILE* fp = fopen(gop_filename, "wb");
    if (!fp) {
        printf("failed opening file %s\n", gop_filename);
        return;
    }

    fwrite(gop_buffer, 1, gop_len, fp);
    fclose(fp);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("usage: {program_name} {path_to_h264_file}\n");
        return -1;
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

    // Decode video
    size_t nread, nparsed;

    size_t ngops = 0;
    size_t gop_len = 0;
    // Note that gop_buffer might contain the frame bytes of the next gop
    size_t gop_buffer_len = 0;

    int is_keyframe = 0;
    int first_keyframe = 1;

    while (1) {
        nread = fread(buffer, 1, BUFFER_SIZE, v_fp);
        if (nread == 0) {
            break;
        }

        const char* data = &buffer[0];
        while (nread > 0) {
            nparsed = h264decoder_parse(decoder_ptr, data, nread);
            memcpy(gop_buffer + gop_buffer_len, data, nparsed);
            gop_buffer_len += nparsed;
            nread -= nparsed;
            data += nparsed;

            // Frame is ready
            if (h264decoder_available(decoder_ptr)) {
                h264decoder_decode(decoder_ptr, &is_keyframe);
                if (is_keyframe) {
                    if (first_keyframe) {
                        first_keyframe = 0;
                    } else {
                        save_gop(gop_len, ngops);
                        ngops++;
                        gop_buffer_len -= gop_len;
                        memcpy(gop_buffer, gop_buffer + gop_len, gop_buffer_len);
                    }
                }
                gop_len = gop_buffer_len;
            }
        }
    };

    // Save the last gop
    if (gop_buffer_len != 0) {
        save_gop(gop_buffer_len, ngops);
    }

    // Cleanup
    h264decoder_free(decoder_ptr);
    fclose(v_fp);
    return 0;
}
