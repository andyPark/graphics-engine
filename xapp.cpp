/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#include "GXWindow.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GContext.h"
#include "GIRect.h"
#include "GRandom.h"
#include "GTime.h"
#include <string>
#include <math.h>

static GPixel* next_row(const GBitmap& bm, GPixel* row) {
    return (GPixel*)((const char*)row + bm.fRowBytes);
}

static void fill_circle(const GBitmap& bm) {
    int w = bm.width();
    int h = bm.height();
    int r = (w < h ? w : h) >> 1;
    float cx = w/2;
    float cy = h/2;
    float rr = (float)r * r;

    GPixel* row = bm.fPixels;
    for (int y = 0; y < h; ++y) {
        float dy2 = (y - cy) * (y - cy);
        for (int x = 0; x < w; ++x) {
            float dx2 = (x - cx) * (x - cx);
            if (dx2 + dy2 > rr) {
                row[x] = 0;
            }
        }
        row = next_row(bm, row);
    }
}

class Shape {
public:
    Shape(float x, float y) : fX(x), fY(y), fA(1) {
        fPrevTime = GTime::GetMSec();
    }
    virtual ~Shape() {}

    void setup(float dx, float dy, float da) {
        fDx = dx; fDy = dy; fDa = da;
    }
    
    void bounce(int w, int h, GMSec now);

    int getX() const { return (int)fX; }
    int getY() const { return (int)fY; }
    float getA() const { return fA; }

    virtual void draw(GContext*) = 0;

private:
    float fX, fY, fA;
    float fDx, fDy, fDa;
    GMSec  fPrevTime;
};

static void bounce(float& x, float& dx, float scale, float limit) {
    x += dx * scale;
    if (dx > 0 && x > limit) {
        x = limit;
        dx = -dx;
    } else if (dx < 0 && x < 0) {
        x = 0;
        dx = -dx;
    }
}

void Shape::bounce(int w, int h, GMSec now) {
    float dur = (now - fPrevTime) / 1000.0f;
    fPrevTime = now;

    ::bounce(fX, fDx, dur, w);
    ::bounce(fY, fDy, dur, h);
    ::bounce(fA, fDa, dur, 1);
}

///////////////////////////////////////////////////////////////////////////////

class BitmapShape : public Shape {
public:
    BitmapShape(const GBitmap& bm, int x, int y)
    : Shape(x, y), fBM(bm) {}

    virtual void draw(GContext* ctx) {
        ctx->drawBitmap(fBM,
                        this->getX() - fBM.width()/2,
                        this->getY() - fBM.height()/2, this->getA());
    }
    
private:
    GBitmap fBM;
};

class TestWindow : public GXWindow {
    Shape** fShapes;
    int     fShapeCount;
    GBitmap* fBitmaps;
    int     fBitmapCount;
public:
    TestWindow(int w, int h,
               char const* const* files, int fileCount,
               bool doCircles, bool doFade, int repeat) : GXWindow(w, h) {
        fDoOpaque = true;
        fStartTime = GTime::GetMSec();
        fCounter = 0;

        GRandom rand;

        fBitmapCount = 0;
        fBitmaps = new GBitmap[fileCount];
        for (int i = 0; i < fileCount; ++i) {
            if (GReadBitmapFromFile(files[i], &fBitmaps[i])) {
                fBitmapCount += 1;
                
                if (doCircles) {
                    fill_circle(fBitmaps[i]);
                }
            }
        }
        fShapeCount = fBitmapCount * repeat;
        fShapes = new Shape*[fShapeCount];

        float speed = 300;
        for (int i = 0; i < fShapeCount; ++i) {
            fShapes[i] = new BitmapShape(fBitmaps[i % fBitmapCount], w/2, h/2);
            fShapes[i]->setup(rand.nextF() * speed,
                              rand.nextF() * speed,
                              doFade ? rand.nextF() : 0);
        }
    }

    virtual ~TestWindow() {
        for (int i = 0; i < fShapeCount; ++i) {
            delete fShapes[i];
        }
        delete[] fShapes;

        for (int i = 0; i < fBitmapCount; ++i) {
            free(fBitmaps[i].fPixels);
        }
        delete[] fBitmaps;
    }
    
protected:
    virtual void onDraw(GContext* ctx) {
        ctx->clear(GColor::Make(1, 1, 1, 1));
        
        GMSec now = GTime::GetMSec();
        for (int i = 0; i < fShapeCount; ++i) {
            fShapes[i]->bounce(this->width(), this->height(), now);
            fShapes[i]->draw(ctx);
        }
        this->requestDraw();

        if (++fCounter > 100) {
            char buffer[100];
            int dur = GTime::GetMSec() - fStartTime;
            sprintf(buffer, "FPS %8.1f", fCounter * 1000.0 / dur);
            this->setTitle(buffer);

            fStartTime = GTime::GetMSec();
            fCounter = 0;
        }
    }
    
    virtual bool onKeyPress(const XEvent& evt) {
        fDoOpaque = !fDoOpaque;
        return this->INHERITED::onKeyPress(evt);
    }
    
private:
    GRandom fRand;
    GMSec   fStartTime;
    int     fCounter;
    bool    fDoOpaque;

    typedef GXWindow INHERITED;
};

int main(int argc, char const* const* argv) {
    if (1 == argc) {
        fprintf(stderr, "usage: [--circles] [--fade] [--repeat N] file1.png file2.png ...\n");
        return -1;
    }
    
    bool docircles = false;
    bool dofade = false;
    int repeat = 1;
    int firstFile = argc;

    for (int i = 1; i < argc; ++i) {
        if (strstr(argv[i], "--") == argv[i]) {
            if (!strcmp(argv[i], "--circles")) {
                docircles = true;
            } else if (!strcmp(argv[i], "--fade")) {
                dofade = true;
            } else if (!strcmp(argv[i], "--repeat") && i < argc - 1) {
                repeat = atol(argv[++i]);
                if (repeat < 1) {
                    repeat = 1;
                }
            } else {
                fprintf(stderr, "unrecognized option %s\n", argv[i]);
                return -1;
            }
        } else {
            firstFile = i;
            break;
        }
    }
    int count = argc - firstFile;
    if (count <= 0) {
        fprintf(stderr, "need file1.png ... args\n");
        return -1;
    }

    return TestWindow(640, 480,
                      &argv[firstFile], count,
                      docircles, dofade, repeat).run();
}

