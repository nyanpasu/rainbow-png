#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <png.h>

#define DEFAULT_NAME "a.png"
#define SZ 4096 * 4096 * 3

union {
    int i;
    char rgb[3];
} pixel = {0};

void
gen_a(char *buf)
{
    pixel.i = 0;

    for (unsigned int i = 0; i < 4096 * 4096; i++) {
        buf[i * 3 + 0] = pixel.rgb[0];
        buf[i * 3 + 1] = pixel.rgb[1];
        buf[i * 3 + 2] = pixel.rgb[2];
        pixel.i++;
    }
}

void
gen_b(char *buf)
{
    gen_a(buf);

    char *buf2 = malloc(SZ);
    if (!buf2) {
        printf("Could not allocated memory.\n");
        exit(1);
    }
    memcpy(buf2, buf, SZ);

    for (unsigned int i = 0; i < 4096 * 4096; i++) {
        int y = i / 4096;
        int x = i % 4096;

        size_t column = x % 16 * 256 + x / 16;
        size_t target = y * 4096 + column;
        buf[i * 3 + 0] = buf2[target * 3 + 0];
        buf[i * 3 + 1] = buf2[target * 3 + 1];
        buf[i * 3 + 2] = buf2[target * 3 + 2];
    }

    free(buf2);
}

int
main(int argc, char **argv)
{
    const char *fn = DEFAULT_NAME;
    if (argc >= 2)
        fn = argv[1];

    FILE *f = fopen(fn, "w");
    if (!f) {
        printf("Could not open file %s for writing\n", fn);
        exit(1);
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING,
            NULL, NULL, NULL);
    if (!png)
        exit(1);
    png_infop png_info = png_create_info_struct(png);
    if (!png_info)
        exit(1);

    if (setjmp(png_jmpbuf(png))) {
        fclose(f);
        printf("libpng has encountered an error.\n");
        exit(1);
    }

    png_init_io(png, f);
    png_set_IHDR(png, png_info,
            4096, 4096, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_set_filter(png, 0, PNG_FILTER_NONE);

    png_write_info(png, png_info);

    // Generate the image!
    char *buf = malloc(SZ);
    if (!buf) {
        printf("Could not allocated memory.\n");
        exit(1);
    }

    gen_b(buf);

    png_byte *rows[4096];
    for (unsigned int i = 0; i < 4096; i++) {
        rows[i] = buf + i * 4096 * 3;
    }


    png_write_image(png, rows);
    png_write_end(png, png_info);
    png_destroy_write_struct(&png, &png_info);
 
    return 0;
}
