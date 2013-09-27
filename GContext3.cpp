/*
 * Copyright 2013 Andrew Park
 *
 * COMP 590 -- Fall 2013
 * Instructor: Mike Reed
 *
 */

#include "GContext3.h"

GContext3::GContext3(const GBitmap& b){
    this->bitmap = b;
}

GContext3::~GContext3() {
}

void GContext3::getBitmap(GBitmap* b) const {
    *b = bitmap;
}

void GContext3::clear(const GColor& color) {
    
    GPixel pix = color2pix(color);
    unsigned rowbytes = bitmap.fRowBytes/sizeof(GPixel);

    for (int y = 0; y < bitmap.fHeight; ++y) {
        for (int x = 0; x < bitmap.fWidth; ++x) {
            bitmap.fPixels[y*rowbytes + x] = pix;
        }
    }
}

GContext* GContext::Create(const GBitmap& bm) {
    return new GContext3(bm);
}

GContext* GContext::Create(int width, int height) {
    GBitmap* b = new GBitmap();
    b->fPixels = new GPixel[width*height];
    b->fWidth = width;
    b->fHeight = height;
    b->fRowBytes = width * sizeof(GPixel);
    return new GContext3(*b);
}

/************************************************************************
 *  Fill the specified rectangle with the specified color, blending using
 *  SRC_OVER mode. If the rectangle is inverted (e.g. width or height < 0)
 *  then nothing is drawn.
 ************************************************************************/
void GContext3::fillIRect(const GIRect& rect, const GColor& src_c) {

    if (isValidToFillRect(rect, src_c))
        return;

    unsigned rowbytes = bitmap.fRowBytes/sizeof(GPixel);

    int rect_left = rect.fLeft;
    int fill_width = rect.width();
    int rect_top = rect.fTop;
    int fill_height = rect.height(); 

    if (rect.fLeft < 0) {
        rect_left = 0;
        fill_width = rect.width() + rect.fLeft;
    }
    if (rect_left + fill_width > bitmap.width()) {
        fill_width = bitmap.width() - rect_left;
    }
    if (rect.fTop < 0) {
        rect_top = 0;
        fill_height = rect.height() + rect.fTop;
    }
    if (rect_top + fill_height > bitmap.height()) {
        fill_height = bitmap.height() - rect_top;
    }

    unsigned dest_start = rect_left + rect_top * rowbytes; //destination rectangle top left corner pixel
    GPixel src_c_pix = color2pix(src_c);
    
    for (int y = 0; y < fill_height; ++y) {
        for (int x = 0; x < fill_width; ++x) {
            int dest_pixel = dest_start + y*rowbytes + x;
            bitmap.fPixels[dest_pixel] = blend_pixels(src_c_pix, bitmap.fPixels[dest_pixel]);
        }
    }
}

GPixel GContext3::blend_pixels(GPixel src, GPixel dest) {
    unsigned out_a = (unsigned) (GPixel_GetA(src) + GPixel_GetA(dest)*((255 - GPixel_GetA(src))/255.0f) + 0.5f);
    unsigned out_r = (unsigned) ((GPixel_GetR(src) + GPixel_GetR(dest)*((255 - GPixel_GetA(src))/255.0f)) + 0.5f);
    unsigned out_g = (unsigned) ((GPixel_GetG(src) + GPixel_GetG(dest)*((255 - GPixel_GetA(src))/255.0f)) + 0.5f);
    unsigned out_b = (unsigned) ((GPixel_GetB(src) + GPixel_GetB(dest)*((255 - GPixel_GetA(src))/255.0f)) + 0.5f);
    return GPixel_PackARGB(out_a, out_r, out_g, out_b);
}


float GContext3::pin_float(float x) {
    x = (x < 0) ? 0 : x;
    x = (x > 1) ? 1 : x;
    return x;
}

int float2byte(float x) {
    assert(x >= 0 && x <= 1);
    return (int)(x*255+0.5f);
}

GPixel GContext3::color2pix(const GColor& color) {
    float float_a = pin_float(color.fA);
    int a = float2byte(float_a);
    int r = float2byte(pin_float(color.fR)*float_a);
    int g = float2byte(pin_float(color.fG)*float_a);
    int b = float2byte(pin_float(color.fB)*float_a);
    return GPixel_PackARGB(a, r, g, b);
}

bool GContext3::isValidToFillRect(GIRect const &rect, GColor const &src_c) {
    return rect.fTop > bitmap.height() || rect.fLeft > bitmap.width() || rect.width() == 0 || rect.height() == 0  || rect.width() < 0 ||  rect.height() < 0 || src_c.fA <= 0.0;
}
