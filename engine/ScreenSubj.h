#pragma once
#include "linmath.h"
#include "geom/Gabarites.h"

class ScreenSubj
{
public:

    char className[32] = "";
    char name64[64] = ""; //64 because of GLTF node names
    char type16[16] = "";
    char subtype16[16] = "";
    unsigned int birthFrameN;
    Gabarites gabaritesOnScreen;

public:
    virtual bool isClickable() { return false; };
    virtual bool isDraggable() { return false; };
    virtual bool isResponsive() { return (isClickable() || isDraggable()); };

    virtual int onDrag() { return 1; };
    virtual int onClick() { return onFocusOut(); };
    virtual int onFocusOut() { return 1; };
    virtual int onFocus() { return 1; };
    virtual int onLeftButtonUp() { return onFocusOut(); };
    virtual int onLeftButtonDown() { return 1; };
    virtual ScreenSubj* getResponsiveSubj(){return NULL;};
};
