//
//  draw.h
//  pigl
//
//  Created by Brian Black on 6/13/18.
//

#ifndef draw_h
#define draw_h

#define VRAM_WIDTH 1024

typedef struct {
  int x;
  int y;
} vec2_t;

typedef struct {
  int x1;
  int y1;
  int x2;
  int y2;
} rect_t;

unsigned short * getPixel(int x, int y);
vec2_t vertFromWord(unsigned int word);

void drawSingleColorRectVarSizeOpaque(unsigned int * buffer, unsigned int count);
void drawSingleColorTri(unsigned int * buffer, unsigned int count);
void drawTriTexturedTextureBlend(unsigned int * buffer, unsigned int count);
void drawQuad(unsigned int * buffer, unsigned int count);
void drawQuadTexturedTextureBlend(unsigned int * buffer, unsigned int count);
void drawTriShaded(unsigned int * buffer, unsigned int count);
void drawTriTexturedShaded(unsigned int * buffer, unsigned int count);
void drawQuadShaded(unsigned int * buffer, unsigned int count);
void drawQuadTexturedShaded(unsigned int * buffer, unsigned int count);
void drawLine(unsigned int * buffer, unsigned int count);
void drawShadedLine(unsigned int * buffer, unsigned int count);
void drawSingleColorRectVarSizeSemiTrans(unsigned int * buffer, unsigned int count);
void drawSingleColorRect16Opaque(unsigned int * buffer, unsigned int count);
void drawTexturedRect(unsigned int * buffer, unsigned int count);

#endif /* draw_h */
