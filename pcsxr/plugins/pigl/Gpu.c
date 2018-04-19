#include "PSEmu Plugin Defs.h"
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>

#include "window.h"

// TODO: replace these quick defs for undefined shit
static void * hInst;
static long lSelectedSlot;
static long iTransferMode;
static HWND GetActiveWindow(void){
    return (HWND) NULL;
};
#define DLGPROC void *
#define IDD_CFG 0
static int MAKEINTRESOURCE(int i) {
    return 0;
};
static int IDD_ABOUT;
static void BuildDispMenu(int i) {
    return;
};
static void DialogBox(void * a, int b, HWND c, DLGPROC d) {
    return;
};
static void * CfgDlgProc;
static void * AboutDlgProc;
// Vars below here are things I know
#define CALLBACK
static unsigned char * psxVub;
static int statusReg;
#define STATUSREG statusReg
static int dataReg;
#define DATAREG dataReg
typedef struct {
  int x;
  int y;
} vec2_t;
static vec2_t drawingOffset;
typedef struct {
  int x1;
  int y1;
  int x2;
  int y2;
} rect_t;
static rect_t displayArea;
static rect_t drawingArea;
typedef struct {
  short maskX;
  short maskY;
  short offsetX;
  short offsetY;
} textureWindowSetting_t;
static textureWindowSetting_t textureWindowSetting;
const static int extraWordsByCommand[] = {
  0, 0, 2, 0, 0, 0, 0, 0, // 00
  0, 0, 0, 0, 0, 0, 0, 0, // 08
  0, 0, 0, 0, 0, 0, 0, 0, // 10
  0, 0, 0, 0, 0, 0, 0, 0, // 18
  0, 0, 0, 0, 6, 6, 6, 6, // 20
  0, 0, 0, 0, 8, 8, 8, 8, // 28
  5, 0, 5, 0, 8, 0, 0, 0, // 30
  7, 0, 7, 0, 11, 0, 0, 0, // 38
  0, 0, 0, 0, 0, 0, 0, 0, // 40
  0, 0, 0, 0, 0, 0, 0, 0, // 48
  0, 0, 0, 0, 0, 0, 0, 0, // 50
  0, 0, 0, 0, 0, 0, 0, 0, // 58
  2, 0, 2, 0, 0, 3, 0, 3, // 60
  0, 0, 0, 0, 0, 0, 0, 0, // 68
  0, 0, 0, 0, 0, 0, 0, 0, // 70
  0, 0, 0, 0, 0, 2, 0, 0, // 78
  3, 0, 0, 0, 0, 0, 0, 0, // 80
  0, 0, 0, 0, 0, 0, 0, 0, // 88
  0, 0, 0, 0, 0, 0, 0, 0, // 90
  0, 0, 0, 0, 0, 0, 0, 0, // 98
  2, 0, 0, 0, 0, 0, 0, 0, // a0
  0, 0, 0, 0, 0, 0, 0, 0, // a8
  0, 0, 0, 0, 0, 0, 0, 0, // b0
  0, 0, 0, 0, 0, 0, 0, 0, // b8
  2, 0, 0, 0, 0, 0, 0, 0, // c0
  0, 0, 0, 0, 0, 0, 0, 0, // c8
  0, 0, 0, 0, 0, 0, 0, 0, // d0
  0, 0, 0, 0, 0, 0, 0, 0, // d8
  0, 0, 0, 0, 0, 0, 0, 0, // e0
  0, 0, 0, 0, 0, 0, 0, 0, // e8
  0, 0, 0, 0, 0, 0, 0, 0, // f0
  0, 0, 0, 0, 0, 0, 0, 0, // f8
};
static unsigned int commandWordsExpected;
static unsigned int commandWordsReceived;
static unsigned int commandWordsBuffer[256];
typedef struct {
  int x;
  int y;
  int width;
  int height;
  int currentX;
  int currentY;
} GPUWrite_t;
static GPUWrite_t GPUWrite;
static GPUWrite_t GPURead;
static GLuint vramTexture;
static bool treatWordsAsPixelData;

////////////////////////////////////////////////////////////////////////
// PPDK developer must change libraryName field and can change revision and build
////////////////////////////////////////////////////////////////////////

const  unsigned char version  = 1;    // do not touch - library for PSEmu 1.x
const  unsigned char revision = 1;
const  unsigned char build    = 46;
static char *libraryName      = "pi's opengl driver";
static char *PluginAuthor     = "pi";
#define VRAM_WIDTH 1024
#define VRAM_HEIGHT 512
static int VRAM_PIXEL_COUNT = VRAM_WIDTH * VRAM_HEIGHT;
static bool glInitialized = FALSE;

////////////////////////////////////////////////////////////////////////
// stuff to make this a true PDK module
////////////////////////////////////////////////////////////////////////

char * CALLBACK PSEgetLibName(void) {
 return libraryName;
}

unsigned long CALLBACK PSEgetLibType(void)
{
 return  PSE_LT_GPU;
}

unsigned long CALLBACK PSEgetLibVersion(void)
{
 return version<<16|revision<<8|build;
}

////////////////////////////////////////////////////////////////////////
// Init/shutdown, will be called just once on emu start/close
////////////////////////////////////////////////////////////////////////

long CALLBACK GPUinit() {
  return 0;
}

long CALLBACK GPUshutdown()                            // GPU SHUTDOWN
{
 return 0;
}

////////////////////////////////////////////////////////////////////////
// Snapshot func, save some snap into
////////////////////////////////////////////////////////////////////////

void CALLBACK GPUmakeSnapshot(void)                    // MAKE SNAPSHOT FILE
{
}

////////////////////////////////////////////////////////////////////////
// Open/close will be called when a games starts/stops
////////////////////////////////////////////////////////////////////////

GLuint makeVramTexture(void){
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                  GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_NEAREST);
  return tex;
}

// TODO: move it
void display(void) {
  glClear(GL_COLOR_BUFFER_BIT);

  glEnable(GL_TEXTURE_2D);
  if (!vramTexture)
    vramTexture = makeVramTexture();
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, VRAM_WIDTH, VRAM_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, (unsigned short *)psxVub);

  glBegin(GL_POLYGON);
  glTexCoord2f(0.0, 0.0);
  glVertex3i(-1, 1, 0);
  glTexCoord2f(1.0, 0.0);
  glVertex3i(1, 1, 0);
  glTexCoord2f(1.0, 1.0);
  glVertex3i(1, -1, 0);
  glTexCoord2f(0.0, 1.0);
  glVertex3i(-1, -1, 0);
  glEnd();
  glFlush();
}

long CALLBACK GPUopen(HWND hwndGPU) {
  // printf("GPUopen entered\n");
  psxVub = calloc(VRAM_PIXEL_COUNT * 2, sizeof(unsigned char));

  if (!glInitialized) {
    // printf("initGLWindow()...\n");
    initGLWindow(VRAM_WIDTH, VRAM_HEIGHT);
    // printf("returned from initGLWindow()\n");
    glInitialized = TRUE;
    makeCurrentContext();
    // printf("making GL calls now...\n");
    glViewport(0, 0, VRAM_WIDTH, VRAM_HEIGHT);
    glClearColor(0.0, 0.0, 0.0, 0.0);
  }

  return 0;
}

long CALLBACK GPUclose() {
  free(psxVub);
  return 0;
}

////////////////////////////////////////////////////////////////////////
// UpdateLace will be called on every vsync
////////////////////////////////////////////////////////////////////////

void CALLBACK GPUupdateLace(void) {
  makeCurrentContext();
  display();
}

////////////////////////////////////////////////////////////////////////
// process read request from GPU status register
////////////////////////////////////////////////////////////////////////

unsigned long CALLBACK GPUreadStatus(void) {
  // printf("GPUreadStatus\n");
  return STATUSREG;
}

////////////////////////////////////////////////////////////////////////
// processes data send to GPU status register
////////////////////////////////////////////////////////////////////////

// new: for freezing stuff... memset it to 0 at GPUinit !
unsigned long ulStatusControl[256];

void CALLBACK GPUwriteStatus(unsigned long gdata) {
  unsigned long lCommand = (gdata >> 24) & 0xff;
  // printf("GPUwriteStatus %08x\n", gdata);
  ulStatusControl[lCommand] = gdata;                      // store command for freezing

  switch (lCommand) {
    case 0x00: // Reset GPU
      STATUSREG = 0x14802000; // TODO: make this less magic-numbery
      break;
    case 0x01: // Reset command buffer / clear fifo (TODO: do it)
      break;
    case 0x02: // Acknowledge GPU interrupt (IRQ1)
      STATUSREG &= 0xfeffffff;  // "Resets the IRQ flag in GPUSTAT.24"
      break;
    case 0x03: // Enable/disable display (0=enable; 1=disable)
      STATUSREG &= 0xff7fffff;
      STATUSREG |= (gdata & 0b1) << 23;
      break;
    case 0x04: // Set DMA direction (0=off, 1=fifo, 2=cpu->gp0, 3=GPUREAD->CPU)
      STATUSREG &= 0x9fffffff;
      STATUSREG |= (gdata & 0b11) << 29;
      break;
    case 0x05: // Set display area location (upper-left) in vram
      displayArea.x1 = gdata & 0x03ff;
      displayArea.y1 = (gdata >> 10) & 0x01ff;
      break;
    case 0x06: // Set display x-range on screen TODO: do it
      break;
    case 0x07: // Set display y-range on screen TODO: do it
      break;
    case 0x08: // Set display mode TODO: do it
      break;
    case 0x10: // Get GPU Info
      switch (gdata & 0xf) { // param
        case 0x07: // read GPU type (usually 2) see "gpu versions"
          DATAREG = 2;
          break;
      }
      break;
    default:
      printf("Unknown GP1 cmd received: %08x\n", gdata);
      break;
  }
}

unsigned short * getPixel(int x, int y) {
  return (unsigned short *)psxVub + (y * VRAM_WIDTH + x);
}

////////////////////////////////////////////////////////////////////////
// core read from vram
////////////////////////////////////////////////////////////////////////

unsigned long CALLBACK GPUreadData(void) {
  return DATAREG;
}

// new function, used by ePSXe, for example, to read a whole chunk of data

void CALLBACK GPUreadDataMem(unsigned long * pMem, int iSize) {
  // iSize is the number of WORDS (i.e. pixel PAIRS) to read
  int pixelNum = 0;
  unsigned short * usMem = (unsigned short *) pMem;
  
  while ((GPURead.currentX < GPURead.x + GPURead.width) &&
         (GPURead.currentY < GPURead.y + GPURead.height) &&
         (pixelNum < iSize * 2)) {
    usMem[pixelNum] = *(getPixel(GPURead.currentX, GPURead.currentY));
    pixelNum += 1;
    GPURead.currentX += 1;
    if (GPURead.currentX >= GPURead.x + GPURead.width) {
      GPURead.currentX = GPURead.x;
      GPURead.currentY += 1;
    }
  }
  // did we just finish reading the last line?
  if (GPURead.currentY > GPURead.y + GPURead.height) {
    statusReg &= 0xf7ffffff;
  }
}

void copyVramToVram(unsigned int * buffer) {
  vec2_t fromLoc = {.x = buffer[1] & 0xffff, .y = (buffer[1] >> 32) & 0xffff};
  vec2_t toLoc = {.x = buffer[2] & 0xffff, .y = (buffer[2] >> 32) & 0xffff};
  vec2_t size = {.x = buffer[3] & 0xffff, .y = (buffer[3] >> 32) & 0xffff};
  unsigned short *psxVus = (unsigned short *)psxVub;
  for (int i = 0; i < size.y; i++) {
    memcpy(
      psxVus + (VRAM_WIDTH * toLoc.y + toLoc.x),
      psxVus + (VRAM_WIDTH * fromLoc.y + fromLoc.x),
      2 * size.x
    );
  }
}

void setupWriteTextureToVram(unsigned int * buffer, unsigned int count) {
  GPUWrite.x = GPUWrite.currentX = buffer[1] & 0xffff;
  GPUWrite.y = GPUWrite.currentY = (buffer[1] >> 16) & 0xffff;
  GPUWrite.width = buffer[2] & 0xffff;
  GPUWrite.height = (buffer[2] >> 16) & 0xffff;
  treatWordsAsPixelData = true;
  // printf("Prepared CPU->VRAM: %d * %d @ (%d, %d)\n",
  //       GPUWrite.width, GPUWrite.height, GPUWrite.x, GPUWrite.y);
}

unsigned short get15from24(unsigned int color) {
  return ((color >> 9) & 0x7c00) |
         ((color >> 6) & 0x03e0) |
         ((color >> 3) & 0x001f);
}

unsigned short blend15bit(unsigned short src, unsigned short dest, unsigned char alphaMode) {
  unsigned short srcR = (src >> 10) & 0x1f;
  unsigned short destR = (dest >> 10) & 0x1f;
  unsigned short srcG = (src >> 5) & 0x1f;
  unsigned short destG = (dest >> 5) & 0x1f;
  unsigned short srcB = src & 0x1f;
  unsigned short destB = dest & 0x1f;
  unsigned short outR;
  unsigned short outG;
  unsigned short outB;
  if (src == 0) {
    return dest;
  }
  if (!(src & 0x8000)) { // alpha bit = 0?
    return src;
  }
  // TODO: use blend func indicated by status reg bits
  // dest - src:
  switch (alphaMode) {
    case 0:
      outR = destR / 2 + srcR / 2;
      outG = destG / 2 + srcG / 2;
      outB = destB / 2 + srcB / 2;
      break;
    case 1:
      outR = destR + srcR;
      outG = destG + srcG;
      outB = destB + srcB;
      if (outR > 31) outR = 31;
      if (outG > 31) outG = 31;
      if (outB > 31) outB = 31;
      break;
    case 2:
      outR = (destR < srcR ? 0 : destR - srcR);
      outG = (destG < srcG ? 0 : destG - srcG);
      outB = (destB < srcB ? 0 : destB - srcB);
      break;
    case 3:
      outR = (destR + srcR / 4);
      outG = (destG + srcG / 4);
      outB = (destB + srcB / 4);
      if (outR > 31) outR = 31;
      if (outG > 31) outG = 31;
      if (outB > 31) outB = 31;
      break;
    default:
      printf("alpha mode %d not implemented\n", alphaMode);
  }
  return ((outR << 10) & 0x7c00) |
         ((outG << 5) & 0x03e0) |
         (outB & 0x001f);
}

vec2_t vertFromWord(unsigned int word) {
  vec2_t vert = {.y = (short)(word >> 16 & 0xffff), .x = (short)(word & 0xffff)};
  return vert;
}

// TODO: speedup e.g. with memset
// TODO: DRY with drawSingleColorRectVarSizeSemiTrans
void drawSingleColorRectVarSizeOpaque(unsigned int * buffer, unsigned int count) {
  int color = buffer[0] & 0xffffff; // 24-bit
  int color15 = get15from24(color);
  int originY = (buffer[1] >> 16) & 0xffff;
  int originX = buffer[1] & 0xffff;
  int height = (buffer[2] >> 16) & 0xffff;
  int width = buffer[2] & 0xffff;
  unsigned short * pixel;

  for (int y = originY; y < originY + height; y++) {
    if (y < drawingArea.y1 || y > drawingArea.y2) {
      continue;
    }
    for (int x = originX; x < originX + width; x++) {
      if (x < drawingArea.x1 || x > drawingArea.x2) {
        continue;
      }
      pixel = getPixel(x, y);
      *pixel = color15;
    }
  }
}

void drawSingleColorRectVarSizeSemiTrans(unsigned int * buffer, unsigned int count) {
  int color = buffer[0] & 0xffffff; // 24-bit
  int color15 = get15from24(color) | 0x8000;
  int originY = (buffer[1] >> 16) & 0xffff;
  int originX = buffer[1] & 0xffff;
  int height = (buffer[2] >> 16) & 0xffff;
  int width = buffer[2] & 0xffff;
  unsigned short * pixel;

  for (int y = originY; y < originY + height; y++) {
    for (int x = originX; x < originX + width; x++) {
      pixel = getPixel(x, y);
      *pixel = blend15bit(color15, *pixel, (statusReg >> 5) & 0x3);
    }
  }
}

unsigned short sampleTexpage(short texpageX, short texpageY, vec2_t uv, unsigned short colorDepth, unsigned short clut) {
  unsigned short u = uv.x & (~(textureWindowSetting.maskX * 8)) |
    ((textureWindowSetting.offsetX & textureWindowSetting.maskX) * 8);
  unsigned short v = uv.y & (~(textureWindowSetting.maskY * 8)) |
    ((textureWindowSetting.offsetY & textureWindowSetting.maskY) * 8);
  // Clamp uv coords, because on the first encounter in FF7, there was a rounding
  // error in the uv mapping: uv.x = round(-0.7...) = -1, causing segfault
  // TODO: Decide if this should be necessary, and either improve or remove.
  if (u < 0) u = 0;
  if (v < 0) v = 0;
  if (v > 255) v = 255;
  // set pixel to the halfword beginning the line
  unsigned short * pixel = getPixel(texpageX * 64, texpageY * 256 + v);
  unsigned short sample;
  unsigned short clutX = 16 * (clut & 0x3f);
  unsigned short clutY = (clut >> 6) & 0x1ff;
  
  switch (colorDepth) {
    case 0: // 4bit
      if (u > 255) u = 255; // clamp
      pixel += u / 4;
      sample = (*pixel >> (4 * (u % 4))) & 0xf;
      sample = *getPixel(clutX + sample, clutY);
      break;
    case 1: // 8bit
      if (u > 127) u = 127; // clamp
      pixel += u / 2;
      sample = (*pixel >> (8 * (u % 2))) & 0xff;
      sample = *getPixel(clutX + sample, clutY);
      break;
    case 2: // 16bit
      if (u > 63) u = 63; // clamp
      pixel += u;
      sample = (*pixel) & 0xffff;
      break;
  }
  return sample;
}

// this is NOT semi-trans blending--this is "draw blended poly" blending
unsigned short blend24bit(unsigned short color, unsigned int blender) {
  unsigned short b = (color >> 7 & 0xf8) * (blender >> 16 & 0xff) / 0x80;
  unsigned short g = (color >> 2 & 0xf8) * (blender >> 8 & 0xff) / 0x80;
  unsigned short r = (color << 3 & 0xf8) * (blender & 0xff) / 0x80;
  return (
    0x8000 | // alpha bit
    (b << 7 & 0x7c00) |
    (g << 2 & 0x03e0) |
    (r >> 3 & 0x001f)
  );
}

void drawTexturedTri(vec2_t verts[], vec2_t texcoords[], unsigned int colors[], unsigned short texpage, unsigned short clut, bool semiTrans) {
  short vertCount = 3;
  short yMin;
  short yMax;
  short vertIndexL;
  short vertIndexR;
  short vertIndexNextL;
  short vertIndexNextR;
  float xLeft;
  float xRight;
  float doubleTriArea = (verts[0].y - verts[2].y) * (verts[1].x - verts[2].x) +
    (verts[1].y - verts[2].y) * (verts[2].x - verts[0].x);
  float b[] = {0, 0, 0};
  vec2_t uv;
  
  short texpageX = texpage & 0x000f; // *64
  short texpageY = (texpage & 0x0010) >> 4; // *256
  short texpageAlphaMode = (texpage & 0x0060) >> 5;
  short texpageColorDepth = (texpage & 0x0180) >> 7;
  short texpageTextureDisable = (texpage & 0x0800) >> 11;
  unsigned short blendR;
  unsigned short blendG;
  unsigned short blendB;
  unsigned int blendColor;
  unsigned short outColor;
  unsigned short * psxVus = (unsigned short *)psxVub;
  unsigned short * psxVusTarget;
  
  // find top and bottom scanlines (yMin, yMax)
  // set left and right verts to leftmost and rightmost of top scanline (may be same)
  for (int i = 0; i < vertCount; i++) {
    if ((i == 0) || (verts[i].y < yMin)) {
      yMin = verts[i].y;
      vertIndexL = i;
      vertIndexR = i;
    } else if (verts[i].y == yMin) {
      vertIndexR = i;
    }
    if ((i == 0) || (verts[i].y > yMax)) {
      yMax = verts[i].y;
    }
  }
  // set initial values before iteration
  vertIndexNextL = (vertIndexL - 1 + vertCount) % vertCount;
  vertIndexNextR = (vertIndexR + 1) % vertCount;
  // iterate through scanlines
  // TODO: speedup
  // TODO: expect circular linked list to prevent all this modulo and index crap
  for (int y = yMin; y < yMax; y++) {
    if (y + drawingOffset.y < drawingArea.y1 || y + drawingOffset.y > drawingArea.y2) {
      continue;
    }
    if (verts[vertIndexNextL].y == y) {
      vertIndexL = vertIndexNextL;
      vertIndexNextL = (vertIndexNextL - 1 + vertCount) % vertCount;
    }
    if (verts[vertIndexNextR].y == y) {
      vertIndexR = vertIndexNextR;
      vertIndexNextR = (vertIndexNextR + 1) % vertCount;
    }
    // ((y - y0)/(y1 - y0))(x1 - x0) + x0
    xLeft = verts[vertIndexL].x + (
      (float)(y - verts[vertIndexL].y) /
        (verts[vertIndexNextL].y - verts[vertIndexL].y) *
        (verts[vertIndexNextL].x - verts[vertIndexL].x)
    );
    xRight = verts[vertIndexR].x + (
      (float)(y - verts[vertIndexR].y) /
        (verts[vertIndexNextR].y - verts[vertIndexR].y) *
        (verts[vertIndexNextR].x - verts[vertIndexR].x)
    );
    // draw scanline
    for (int x = (short)xLeft; x < (short)xRight; x++) {
      if (x + drawingOffset.x < drawingArea.x1 || x + drawingOffset.x > drawingArea.x2) {
        continue;
      }
      b[0] = ((y - verts[2].y) * (verts[1].x - verts[2].x) +
        (verts[1].y - verts[2].y) * (verts[2].x - x)) / doubleTriArea;
      b[1] = ((y - verts[0].y) * (verts[2].x - verts[0].x) +
        (verts[2].y - verts[0].y) * (verts[0].x - x)) / doubleTriArea;
//      b[2] = ((y - verts[1].y) * (verts[0].x - verts[1].x) +
//        (verts[0].y - verts[1].y) * (verts[1].x - x)) / doubleTriArea;
      b[2] = 1 - b[0] - b[1];
      if (texcoords) {
        uv.x = round(b[0] * texcoords[0].x + b[1] * texcoords[1].x + b[2] * texcoords[2].x);
        uv.y = round(b[0] * texcoords[0].y + b[1] * texcoords[1].y + b[2] * texcoords[2].y);
        outColor = sampleTexpage(texpageX, texpageY, uv, texpageColorDepth, clut);
      } else {
        outColor = 0x0000;
      }
      if (colors) {
        blendB = (unsigned short)(
          (colors[0] >> 16 & 0xff) * b[0] +
          (colors[1] >> 16 & 0xff) * b[1] +
          (colors[2] >> 16 & 0xff) * b[2]
        );
        blendG = (unsigned short)(
          (colors[0] >> 8 & 0xff) * b[0] +
          (colors[1] >> 8 & 0xff) * b[1] +
          (colors[2] >> 8 & 0xff) * b[2]
        );
        blendR = (unsigned short)(
          (colors[0] & 0xff) * b[0] +
          (colors[1] & 0xff) * b[1] +
          (colors[2] & 0xff) * b[2]
        );
        if (texcoords) {
          blendColor = (blendB << 16) | (blendG << 8) | (blendR);
          outColor = blend24bit(outColor, blendColor);
        } else {
          outColor = 0x8000 | // alpha bit
            (blendB << 7 & 0x7c00) |
            (blendG << 2 & 0x03e0) |
            (blendR >> 3 & 0x001f);
        }
      }
      psxVusTarget = psxVus + (VRAM_WIDTH * (y + drawingOffset.y) + (x + drawingOffset.x));
      if (semiTrans) {
        outColor = blend15bit(outColor, *psxVusTarget, texpageAlphaMode);
      }
      if (outColor != 0x0000) { // remember, "full-black" is actually "full transparent"
        *psxVusTarget = outColor;
      }
    }
  }
}

void drawTexturedRect(unsigned int * buffer, unsigned int count) {
  bool semiTrans = (buffer[0] >> 25) & 0x1;
  signed short y = (buffer[1] >> 16) & 0xffff;
  signed short x = buffer[1] & 0xffff;
  unsigned short clut = (buffer[2] >> 16) & 0xffff;
  unsigned short v = (buffer[2] >> 8) & 0xff;
  unsigned short u = buffer[2] & 0xff;
  unsigned short height;
  unsigned short width;
  unsigned char command = buffer[0] >> 24 & 0xff;
  if (command >= 0x7c) { // TODO: succinctify by using relevant bits
    height = width = 16;
  } else if (command >= 0x74) {
    height = width = 8;
  } else if (command >= 0x6c) {
    height = width = 1;
  } else {
    height = buffer[3] >> 16 & 0xffff;
    width = buffer[3] & 0xffff;
  }
  vec2_t v0 = {.y = y, .x = x};
  vec2_t v1 = {.y = y, .x = x + width};
  vec2_t v2 = {.y = y + height, .x = x + width};
  vec2_t v3 = {.y = y + height, .x = x};
  vec2_t uv0 = {.y = v, .x = u};
  vec2_t uv1 = {.y = v, .x = u + width};
  vec2_t uv2 = {.y = v + height, .x = u + width};
  vec2_t uv3 = {.y = v + height, .x = u};
  vec2_t tri0[] = {v0, v1, v2};
  vec2_t tri1[] = {v2, v3, v0};
  vec2_t texcoords0[] = {uv0, uv1, uv2};
  vec2_t texcoords1[] = {uv2, uv3, uv0};
  unsigned short texpage = statusReg & 0x7ff; // TODO double-check this
  
  drawTexturedTri(tri0, texcoords0, NULL, texpage, clut, semiTrans);
  drawTexturedTri(tri1, texcoords1, NULL, texpage, clut, semiTrans);
}

void drawTriTexturedTextureBlend(unsigned int * buffer, unsigned int count) {
  unsigned int color = buffer[0] & 0x00ffffff;
  bool semiTrans = buffer[0] & 0x02000000;
  vec2_t v0 = vertFromWord(buffer[1]);
  vec2_t v1 = vertFromWord(buffer[3]);
  vec2_t v2 = vertFromWord(buffer[5]);
  vec2_t uv0 = {.y = (buffer[2] >> 8) & 0xff, .x = (buffer[2] & 0xff)};
  vec2_t uv1 = {.y = (buffer[4] >> 8) & 0xff, .x = (buffer[4] & 0xff)};
  vec2_t uv2 = {.y = (buffer[6] >> 8) & 0xff, .x = (buffer[6] & 0xff)};
  vec2_t tri[] = {v0, v1, v2};
  vec2_t texcoords[] = {uv0, uv1, uv2};
  unsigned int colors[] = {color, color, color};
  unsigned short texpage = (buffer[4] >> 16) & 0xffff;
  unsigned short clut = (buffer[2] >> 16) & 0xffff;
  
  drawTexturedTri(tri, texcoords, colors, texpage, clut, semiTrans);
}

void drawQuadTexturedTextureBlend(unsigned int * buffer, unsigned int count) {
  unsigned int color = buffer[0] & 0x00ffffff;
  bool semiTrans = buffer[0] & 0x02000000;
  vec2_t v0 = vertFromWord(buffer[1]);
  vec2_t v1 = vertFromWord(buffer[3]);
  vec2_t v2 = vertFromWord(buffer[5]);
  vec2_t v3 = vertFromWord(buffer[7]);
  vec2_t uv0 = {.y = (buffer[2] >> 8) & 0xff, .x = (buffer[2] & 0xff)};
  vec2_t uv1 = {.y = (buffer[4] >> 8) & 0xff, .x = (buffer[4] & 0xff)};
  vec2_t uv2 = {.y = (buffer[6] >> 8) & 0xff, .x = (buffer[6] & 0xff)};
  vec2_t uv3 = {.y = (buffer[8] >> 8) & 0xff, .x = (buffer[8] & 0xff)};
  vec2_t tri0[] = {v0, v1, v2};
  vec2_t tri1[] = {v2, v1, v3}; // make it clockwise TODO: remove stupid hack
  vec2_t texcoords0[] = {uv0, uv1, uv2};
  vec2_t texcoords1[] = {uv2, uv1, uv3};
  unsigned int colors[] = {color, color, color};
  unsigned short texpage = (buffer[4] >> 16) & 0xffff;
  unsigned short clut = (buffer[2] >> 16) & 0xffff;
  
  drawTexturedTri(tri0, texcoords0, colors, texpage, clut, semiTrans);
  drawTexturedTri(tri1, texcoords1, colors, texpage, clut, semiTrans);
}

void drawTriShaded(unsigned int * buffer, unsigned int count) {
  bool semiTrans = buffer[0] & 0x02000000;
  vec2_t v0 = vertFromWord(buffer[1]);
  vec2_t v1 = vertFromWord(buffer[3]);
  vec2_t v2 = vertFromWord(buffer[5]);
  unsigned int c0 = buffer[0] & 0x00ffffff;
  unsigned int c1 = buffer[2] & 0x00ffffff;
  unsigned int c2 = buffer[4] & 0x00ffffff;
  vec2_t tri[] = {v0, v1, v2};
  unsigned int colors[] = {c0, c1, c2};
  drawTexturedTri(tri, NULL, colors, NULL, NULL, semiTrans);
}

void drawTriTexturedShaded(unsigned int * buffer, unsigned int count) {
  vec2_t v0 = vertFromWord(buffer[1]);
  vec2_t v1 = vertFromWord(buffer[4]);
  vec2_t v2 = vertFromWord(buffer[7]);
  unsigned int c0 = buffer[0] & 0x00ffffff;
  unsigned int c1 = buffer[3] & 0x00ffffff;
  unsigned int c2 = buffer[6] & 0x00ffffff;
  vec2_t uv0 = {.y = (buffer[2] >> 8) & 0xff, .x = (buffer[2] & 0xff)};
  vec2_t uv1 = {.y = (buffer[5] >> 8) & 0xff, .x = (buffer[5] & 0xff)};
  vec2_t uv2 = {.y = (buffer[8] >> 8) & 0xff, .x = (buffer[8] & 0xff)};
  vec2_t tri[] = {v0, v1, v2};
  unsigned int colors[] = {c0, c1, c2};
  vec2_t texcoords[] = {uv0, uv1, uv2};
  unsigned short texpage = (buffer[5] >> 16) & 0xffff;
  unsigned short clut = (buffer[2] >> 16) & 0xffff;
  drawTexturedTri(tri, texcoords, colors, texpage, clut, false);
}

void drawQuadShaded(unsigned int * buffer, unsigned int count) {
  bool semiTrans = buffer[0] & 0x02000000;
  vec2_t v0 = vertFromWord(buffer[1]);
  vec2_t v1 = vertFromWord(buffer[3]);
  vec2_t v2 = vertFromWord(buffer[5]);
  vec2_t v3 = vertFromWord(buffer[7]);
  unsigned int c0 = buffer[0] & 0x00ffffff;
  unsigned int c1 = buffer[2] & 0x00ffffff;
  unsigned int c2 = buffer[4] & 0x00ffffff;
  unsigned int c3 = buffer[6] & 0x00ffffff;
  vec2_t tri0[] = {v0, v1, v2};
  vec2_t tri1[] = {v2, v1, v3};
  unsigned int colors0[] = {c0, c1, c2};
  unsigned int colors1[] = {c2, c1, c3};
  drawTexturedTri(tri0, NULL, colors0, NULL, NULL, semiTrans);
  drawTexturedTri(tri1, NULL, colors1, NULL, NULL, semiTrans);
}

void drawQuadTexturedShaded(unsigned int * buffer, unsigned int count) {
  vec2_t v0 = vertFromWord(buffer[1]);
  vec2_t v1 = vertFromWord(buffer[4]);
  vec2_t v2 = vertFromWord(buffer[7]);
  vec2_t v3 = vertFromWord(buffer[10]);
  unsigned int c0 = buffer[0] & 0x00ffffff;
  unsigned int c1 = buffer[3] & 0x00ffffff;
  unsigned int c2 = buffer[6] & 0x00ffffff;
  unsigned int c3 = buffer[9] & 0x00ffffff;
  vec2_t uv0 = {.y = (buffer[2] >> 8) & 0xff, .x = (buffer[2] & 0xff)};
  vec2_t uv1 = {.y = (buffer[5] >> 8) & 0xff, .x = (buffer[5] & 0xff)};
  vec2_t uv2 = {.y = (buffer[8] >> 8) & 0xff, .x = (buffer[8] & 0xff)};
  vec2_t uv3 = {.y = (buffer[11] >> 8) & 0xff, .x = (buffer[11] & 0xff)};
  vec2_t tri0[] = {v0, v1, v2};
  vec2_t tri1[] = {v2, v1, v3};
  unsigned int colors0[] = {c0, c1, c2};
  unsigned int colors1[] = {c2, c1, c3};
  vec2_t texcoords0[] = {uv0, uv1, uv2};
  vec2_t texcoords1[] = {uv2, uv1, uv3};
  unsigned short texpage = (buffer[5] >> 16) & 0xffff;
  unsigned short clut = (buffer[2] >> 16) & 0xffff;
  drawTexturedTri(tri0, texcoords0, colors0, texpage, clut, false);
  drawTexturedTri(tri1, texcoords1, colors1, texpage, clut, false);
}

void setupReadFromVram(unsigned int * buffer, unsigned int count) {
  unsigned short x = buffer[1] & 0xffff;
  unsigned short y = (buffer[1] >> 16) & 0xffff;
  unsigned short width = buffer[2] & 0xffff;
  unsigned short height = (buffer[2] >> 16) & 0xffff;
  GPURead.currentX = GPURead.x = x;
  GPURead.currentY = GPURead.y = y;
  GPURead.width = width;
  GPURead.height = height;
  statusReg |= 0x08000000;
}

void executeCommandWordBuffer(unsigned int buffer[256], unsigned int count) {
  unsigned int command = (buffer[0] >> 24) & 0xff;

  printf("Executing command %02x\n", command);
  switch (command) {
    case 0x00: // noop
      break;
    case 0x01: // docs unclear - clear fifo, like GP1(01h)? clear texture cache? ??
      break;
    case 0x02: // single-color rect, var size, opaque
      drawSingleColorRectVarSizeOpaque(buffer, count);
      break;
    case 0x24:
      drawTriTexturedTextureBlend(buffer, count);
      break;
    case 0x26:
      drawTriTexturedTextureBlend(buffer, count);
      break;
    case 0x2c:
    case 0x2e:
      drawQuadTexturedTextureBlend(buffer, count);
      break;
    case 0x30: // shaded tri, opaque
      drawTriShaded(buffer, count);
      break;
    case 0x32: // shaded tri, semi-trans
      drawTriShaded(buffer, count);
      break;
    case 0x34: // shaded tri, opaque, texblend
      drawTriTexturedShaded(buffer, count);
      break;
    case 0x38: // shaded quad, opaque
      drawQuadShaded(buffer, count);
      break;
    case 0x3c: // shaded quad, opaque, texblend
      drawQuadTexturedShaded(buffer, count);
      break;
    case 0x3a: // shaded quad, semi-trans
      drawQuadShaded(buffer, count);
      break;
    case 0x60: // single-color rect, var size, opaque
      drawSingleColorRectVarSizeOpaque(buffer, count);
      break;
    case 0x62: // single-color rect, var size, semi-trans
      drawSingleColorRectVarSizeSemiTrans(buffer, count);
      break;
    case 0x65:
    case 0x67:
      drawTexturedRect(buffer, count);
      break;
    case 0x7d:
      drawTexturedRect(buffer, count);
      break;
    case 0x80:
      copyVramToVram(buffer);
      break;
    case 0xa0: // write texture to vram
      setupWriteTextureToVram(buffer, count);
      break;
    case 0xc0:
      setupReadFromVram(buffer, count);
      break;
    case 0xe1: // draw mode setting
      STATUSREG &= 0xfffffc00; // copy least-sig 10 bits to status reg...
      STATUSREG |= (buffer[0] & 0x3ff);
      STATUSREG &= 0xffff7fff; // copy bit 11 to bit 15 of status reg...
      STATUSREG |= ((buffer[0] << 4) & 0x800);
      // TODO: handle bits 12, 13 (textured rectangle x, y flip)
      break;
    case 0xe2: // texture window setting
      textureWindowSetting.maskX = buffer[0] & 0x1f;
      textureWindowSetting.maskY = (buffer[0] >> 5) & 0x1f;
      textureWindowSetting.offsetX = (buffer[0] >> 10) & 0x1f;
      textureWindowSetting.offsetY = (buffer[0] >> 15) & 0x1f;
      break;
    case 0xe3: // set drawing area x1, y1
      drawingArea.x1 = buffer[0] & 0x3ff;
      drawingArea.y1 = (buffer[0] >> 10) & 0x1ff;
      break;
    case 0xe4: // set drawing area x2, y2
      drawingArea.x2 = buffer[0] & 0x3ff;
      drawingArea.y2 = (buffer[0] >> 10) & 0x1ff;
      break;
    case 0xe5: // set drawing offset (typically within drawing area)
      // HERE: 11 bits, not 10--and they're 11-bit signed values.
      drawingOffset.x = buffer[0] & 0x7ff;
      if (drawingOffset.x & 0x400) {
        drawingOffset.x = drawingOffset.x & 0xfffff800;
      }
      drawingOffset.y = (buffer[0] >> 11) & 0x7ff;
      if (drawingOffset.y & 0x400) {
        drawingOffset.y = drawingOffset.y & 0xfffff800;
      }
      break;
    case 0xe6: // mask bit setting
      STATUSREG &= 0xffffe7ff;
      STATUSREG |= ((buffer[0] << 11) & 0x1800);
      break;
    default:
      printf("Command not yet implemented: %02x\n", command);
      break;
  }
}

// new function, used by ePSXe, for example, to write a whole chunk of data
// This represents a number (>= 1) of words written one-by-one to GPU0--
// For example, texture data via DMA2 following GP0(A0h)
void CALLBACK GPUwriteDataMem(unsigned int * pMem, int iSize) {
  unsigned short * psxVus = (unsigned short *) psxVub;
  unsigned short * usMem = (unsigned short *) pMem;
  int pixelNum = 0;
  unsigned char command;

  if (treatWordsAsPixelData) {
    // printf("GPU0 receiving %d things (words/pixelpairs?) to write to VRAM\n", iSize);
    while ((GPUWrite.currentX < GPUWrite.x + GPUWrite.width) &&
           (GPUWrite.currentY < GPUWrite.y + GPUWrite.height) &&
           (pixelNum < iSize * 2)) {
      // TODO: handle case when width not divisible by word width and might wrap mid-word
      psxVus[GPUWrite.currentY * VRAM_WIDTH + GPUWrite.currentX] =
        usMem[pixelNum];
      pixelNum += 1;
      GPUWrite.currentX += 1;
      if (GPUWrite.currentX >= GPUWrite.x + GPUWrite.width) {
        GPUWrite.currentX = GPUWrite.x;
        GPUWrite.currentY += 1;
      }
    }
    // did we just finish the final line?
    if (GPUWrite.currentY >= GPUWrite.y + GPUWrite.height) {
      treatWordsAsPixelData = false;
    }
    // printf("wrote %d words to vram\n", wordNum);
  } else {
    for (int i = 0; i < iSize; i++) {
      printf("GP0(%08xh)\n", pMem[i]);
      commandWordsBuffer[commandWordsReceived] = pMem[i];
      if (commandWordsReceived == 0) {
        command = (pMem[i] >> 24) & 0xff;
        commandWordsExpected = 1 + extraWordsByCommand[command];
      }
      commandWordsReceived += 1;
      if (commandWordsReceived == commandWordsExpected) {
        // we've collected all params for the command; let it take effect
        executeCommandWordBuffer(commandWordsBuffer, commandWordsReceived);
        commandWordsReceived = 0;
        commandWordsExpected = 0;
      }
    }
  }
}

void CALLBACK GPUwriteData(unsigned long gdata) {
  // printf("GPUwriteData %08x\n", gdata);
  GPUwriteDataMem(&gdata, 1);
}

////////////////////////////////////////////////////////////////////////
// setting/getting the transfer mode (this functions are obsolte)
////////////////////////////////////////////////////////////////////////

void CALLBACK GPUsetMode(unsigned long gdata)
{
 return;
}

// this function will be removed soon
long CALLBACK GPUgetMode(void)
{
 return iTransferMode;
}

////////////////////////////////////////////////////////////////////////
// dma chain, process gpu commands
////////////////////////////////////////////////////////////////////////

long CALLBACK GPUdmaChain(unsigned long * baseAddrL, unsigned long addr) {
  // printf(">> receiving dma chain...\n");
  unsigned char *psxMem = (unsigned char *)baseAddrL;
  unsigned int *memBlockWords;
  unsigned char memBlockWordCount;

  do {
    memBlockWords = (unsigned int *)(psxMem + addr);
    memBlockWordCount = (memBlockWords[0] >> 24) & 0xff;
    GPUwriteDataMem(memBlockWords + 1, memBlockWordCount);
    addr = memBlockWords[0] & 0x00ffffff;
  } while (addr != 0x00ffffff);

  // printf("<< dma chain received\n");
  return 0;
}

////////////////////////////////////////////////////////////////////////
// call config dlg
////////////////////////////////////////////////////////////////////////

long CALLBACK GPUconfigure(void)
{
 HWND hWP=GetActiveWindow();

 DialogBox(hInst,MAKEINTRESOURCE(IDD_CFG),
           hWP,(DLGPROC)CfgDlgProc);

 return 0;
}

////////////////////////////////////////////////////////////////////////
// show about dlg
////////////////////////////////////////////////////////////////////////
__attribute__((visibility("default")))
void CALLBACK GPUabout(void)                           // ABOUT?
{
 HWND hWP=GetActiveWindow();                           // to be sure
 DialogBox(hInst,MAKEINTRESOURCE(IDD_ABOUT),
           hWP,(DLGPROC)AboutDlgProc);
 return;
}

////////////////////////////////////////////////////////////////////////
// test... well, we are ever fine ;)
////////////////////////////////////////////////////////////////////////

long CALLBACK GPUtest(void)
{
 // if test fails this function should return negative value for error (unable to continue)
 // and positive value for warning (can continue but output might be crappy)
 return 0;
}


////////////////////////////////////////////////////////////////////////
// special debug function, only available in Pete's Soft GPU
////////////////////////////////////////////////////////////////////////

void CALLBACK GPUdisplayText(char * pText)
{
}

////////////////////////////////////////////////////////////////////////
// special info display function, only available in Pete's GPUs right now
////////////////////////////////////////////////////////////////////////

void CALLBACK GPUdisplayFlags(unsigned long dwFlags)
{
 // currently supported flags:
 // bit 0 -> Analog pad mode activated
 // bit 1 -> PSX mouse mode activated
 // display this info in the gpu menu/fps display
}

////////////////////////////////////////////////////////////////////////
// Freeze
////////////////////////////////////////////////////////////////////////

typedef struct
{
 unsigned long ulFreezeVersion;      // should be always 1 for now (set by main emu)
 unsigned long ulStatus;             // current gpu status
 unsigned long ulControl[256];       // latest control register values
 unsigned char psxVRam[1024*512*2];  // current VRam image
} GPUFreeze_t;

////////////////////////////////////////////////////////////////////////

long CALLBACK GPUfreeze(unsigned long ulGetFreezeData,GPUFreeze_t * pF)
{
 //----------------------------------------------------//
 if(ulGetFreezeData==2)                                // 2: info, which save slot is selected? (just for display)
  {
   long lSlotNum=*((long *)pF);                        // -> a bit dirty, I know... ehehe
   lSelectedSlot=lSlotNum+1;
   BuildDispMenu(0);                                   // -> that's one of my funcs to display the "lSelectedSlot" number
   return 1;
  }
 //----------------------------------------------------//
 if(!pF)                    return 0;                  // some checks
 if(pF->ulFreezeVersion!=1) return 0;

 if(ulGetFreezeData==1)                                // 1: get data
  {
   pF->ulStatus=STATUSREG;
   memcpy(pF->ulControl,ulStatusControl,256*sizeof(unsigned long));
   memcpy(pF->psxVRam,  psxVub,         1024*512*2);

   return 1;
  }

 if(ulGetFreezeData!=0) return 0;                      // 0: set data

 STATUSREG=pF->ulStatus;
 memcpy(ulStatusControl,pF->ulControl,256*sizeof(unsigned long));
 memcpy(psxVub,         pF->psxVRam,  1024*512*2);

// RESET TEXTURE STORE HERE, IF YOU USE SOMETHING LIKE THAT

 GPUwriteStatus(ulStatusControl[0]);
 GPUwriteStatus(ulStatusControl[1]);
 GPUwriteStatus(ulStatusControl[2]);
 GPUwriteStatus(ulStatusControl[3]);
 GPUwriteStatus(ulStatusControl[8]);                   // try to repair things
 GPUwriteStatus(ulStatusControl[6]);
 GPUwriteStatus(ulStatusControl[7]);
 GPUwriteStatus(ulStatusControl[5]);
 GPUwriteStatus(ulStatusControl[4]);

 return 1;
}

void dumpVram(void) {
  unsigned short * psxVuw = (unsigned short *)psxVub;
  int w = VRAM_WIDTH;
  int h = VRAM_HEIGHT;
  FILE *file;
  file = fopen("/Users/bblack/Desktop/psx.vram.ppm", "w");
  fprintf(file, "P6\n");
  fprintf(file, "%d %d\n", w, h);
  fprintf(file, "255\n"); // pixel depth
  for (int i = 0; i < w * h; i++) {
    fputc((psxVuw[i] << 3) & 0b11111000, file); // r
    fputc((psxVuw[i] >> 2) & 0b11111000, file); // g
    fputc((psxVuw[i] >> 7) & 0b11111000, file); // b
  };
  fclose(file);
}
