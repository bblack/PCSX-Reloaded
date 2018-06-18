//
//  Gpu.h
//  pigl
//
//  Created by Brian Black on 6/13/18.
//

#ifndef Gpu_h
#define Gpu_h

#include "draw.h"

extern unsigned char * psxVub;
extern rect_t drawingArea;
extern int statusReg;
extern vec2_t drawingOffset;
typedef struct {
  short maskX;
  short maskY;
  short offsetX;
  short offsetY;
} textureWindowSetting_t;
extern textureWindowSetting_t textureWindowSetting;

#endif /* Gpu_h */
