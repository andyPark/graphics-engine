
/**
 *  Copyright 2013 Andrew Park 
 *
 *  COMP 590 -- Fall 2013
 *  Instructor: Mike Reed
 *
 */
#ifndef GContext3_DEFINED
#define GContext3_DEFINED

#include "GTypes.h"
#include "GContext.h"
#include "GColor.h"
#include "GBitmap.h"
#include "GIRect.h"
#include <cassert>
#include <iostream>
#include <iomanip>

class GBitmap;
class GColor;

class GContext3: public GContext {
protected:
    GBitmap bitmap;
private:
    static const float EIGHTBIT_MAX = 255.0f;
    friend class GContext;
    GPixel color2pix(const GColor &c);
    GPixel blend_pixels(GPixel src, GPixel dest);
    GPixel blend_alpha(GPixel src, float alpha);
    float pin_float(float x);
    bool isNotValidToFillRect(GIRect const &rect, GColor const &src_c);
    static inline float div255(int value);
public:
    GContext3(const GBitmap&);
    virtual ~GContext3(); 
    void getBitmap(GBitmap* b) const;
    void clear(const GColor&);
    static GContext* Create(const GBitmap& bm);
    static GContext* Create(int width, int height);
    void fillIRect(const GIRect&, const GColor&);
    void drawBitmap(const GBitmap&, int x, int y, float alpha = 1);
};

#endif
