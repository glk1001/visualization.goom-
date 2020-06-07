#ifndef _GFONTRLE_H
#define _GFONTRLE_H

struct TheFont {
    unsigned int width;
    unsigned int height;
    unsigned int bytes_per_pixel;
    unsigned int rle_size;
    unsigned char rle_pixel[49725];
};
extern const TheFont the_font;

#endif
