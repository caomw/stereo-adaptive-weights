/**
 * @file image.cpp
 * @brief image class with shallow copy
 * @author Pascal Monasse <monasse@imagine.enpc.fr>
 *
 * Copyright (c) 2012-2014, Pascal Monasse
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Pulic License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "image.h"
#include "io_png.h"
#include <algorithm>
#include <cassert>

/// Constructor
///
/// The main interest of this one is to allow arrays of Image.
Image::Image()
: count(0), tab(0), w(0), h(0), c(0) {}

/// Constructor
Image::Image(int width, int height, int channels)
: count(new int(1)), tab(new float[width*height*channels]),
  w(width), h(height), c(channels) {}

/// Constructor with array of pixels.
///
/// Make sure it is not deleted during the lifetime of the image.
Image::Image(float* pix, int width, int height, int channels)
: count(0), tab(pix), w(width), h(height), c(channels) {}

/// Copy constructor (shallow copy)
Image::Image(const Image& I)
: count(I.count), tab(I.tab), w(I.w), h(I.h), c(I.c) {
    if(count)
        ++*count;
}

/// Assignment operator (shallow copy)
Image& Image::operator=(const Image& I) {
    if(count != I.count) {
        kill();
        if(I.count)
            ++*I.count;
    }
    count=I.count; tab=I.tab; w=I.w; h=I.h; c=I.c;
    return *this;
}

/// Deep copy
Image Image::clone() const {
    Image I(w,h,c);
    std::copy(tab, tab+w*h*c, I.tab);
    return I;
}

/// Free memory
void Image::kill() {
    if(count && --*count == 0) {
        delete count;
        delete [] tab;
    }
}

/// Save \a disparity image in 8-bit PNG image.
///
/// The disp->gray function is affine: gray=a*disp+b.
/// Pixels outside [0,255] are assumed invalid and written in cyan color.
bool save_disparity(const char* fileName, const Image& disparity,
                    int dMin, int dMax, int grayMin, int grayMax) {
    const float a=(grayMax-grayMin)/float(dMax-dMin);
    const float b=(grayMin*dMax-grayMax*dMin)/float(dMax-dMin);

    const int w=disparity.width(), h=disparity.height();
    const float* in=&(const_cast<Image&>(disparity))(0,0);
    unsigned char *out = new unsigned char[3*w*h];
    unsigned char *red=out, *green=out+w*h, *blue=out+2*w*h;
    for(size_t i=w*h; i>0; i--, in++, red++) {
        if((float)dMin<=*in && *in<=(float)dMax) {
            float v = a * *in + b +0.5f;
            if(v<0) v=0;
            if(v>255) v=255;
            *red = static_cast<unsigned char>(v);
            *green++ = *red;
            *blue++  = *red;
        } else { // Cyan for disparities out of range
            *red=0;
            *green++ = *blue++ = 255;
        }
    }
    bool ok = (io_png_write_u8(fileName, out, w, h, 3) == 0);
    delete [] out;
    return ok;
}
