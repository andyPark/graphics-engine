#include "GContext.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GIRect.h"
#include <iostream>

int main(int argc, const char* argv[]) {
    GContext* src = GContext::Create(100, 100);
    GContext* dest = GContext::Create(100, 100);
    src.clear(GColor::Make(1, 1, 0, 0));//red
    dst.clear(GColor::Make(1, 0, 0, 1));//blue
}
