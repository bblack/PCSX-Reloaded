#include "PSEmu Plugin Defs.h"
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>

#include "window.h"

// TODO: replace these quick defs for undefined shit
#define CALLBACK
static unsigned char * psxVub;
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
static int statusReg;
#define STATUSREG statusReg
static int dataReg;
#define DATAREG dataReg
typedef struct {
  int x;
  int y;
  int width;
  int height;
} displayArea_t;
static displayArea_t displayArea;
const static int extraWordsByCommand[] = {
  0, 0, 0, 0, 0, 0, 0, 0, // 00
  0, 0, 0, 0, 0, 0, 0, 0, // 08
  0, 0, 0, 0, 0, 0, 0, 0, // 10
  0, 0, 0, 0, 0, 0, 0, 0, // 18
  0, 0, 0, 0, 0, 0, 0, 0, // 20
  0, 0, 0, 0, 0, 0, 0, 0, // 28
  0, 0, 0, 0, 0, 0, 0, 0, // 30
  0, 0, 0, 0, 0, 0, 0, 0, // 38
  0, 0, 0, 0, 0, 0, 0, 0, // 40
  0, 0, 0, 0, 0, 0, 0, 0, // 48
  0, 0, 0, 0, 0, 0, 0, 0, // 50
  0, 0, 0, 0, 0, 0, 0, 0, // 58
  0, 0, 0, 0, 0, 0, 0, 0, // 60
  0, 0, 0, 0, 0, 0, 0, 0, // 68
  0, 0, 0, 0, 0, 0, 0, 0, // 70
  0, 0, 0, 0, 0, 0, 0, 0, // 78
  0, 0, 0, 0, 0, 0, 0, 0, // 80
  0, 0, 0, 0, 0, 0, 0, 0, // 88
  0, 0, 0, 0, 0, 0, 0, 0, // 90
  0, 0, 0, 0, 0, 0, 0, 0, // 98
  2, 0, 0, 0, 0, 0, 0, 0, // a0
  0, 0, 0, 0, 0, 0, 0, 0, // a8
  0, 0, 0, 0, 0, 0, 0, 0, // b0
  0, 0, 0, 0, 0, 0, 0, 0, // b8
  0, 0, 0, 0, 0, 0, 0, 0, // c0
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
static unsigned long commandWordsBuffer[256];
typedef struct {
  int x;
  int y;
  int width;
  int height;
  int currentX;
  int currentY;
} GPUWrite_t;
static GPUWrite_t GPUWrite;
static GLuint vramTexture;

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

char * CALLBACK PSEgetLibName(void)
{
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
  printf("GPUopen entered\n");
  psxVub = calloc(VRAM_PIXEL_COUNT * 2, sizeof(unsigned char));

  if (!glInitialized) {
    printf("initGLWindow()...\n");
    initGLWindow(VRAM_WIDTH, VRAM_HEIGHT);
    printf("returned from initGLWindow()\n");
    glInitialized = TRUE;
    makeCurrentContext();
    printf("making GL calls now...\n");
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
  printf("GPUreadStatus\n");
  return STATUSREG;
}

////////////////////////////////////////////////////////////////////////
// processes data send to GPU status register
////////////////////////////////////////////////////////////////////////

// new: for freezing stuff... memset it to 0 at GPUinit !
unsigned long ulStatusControl[256];

void CALLBACK GPUwriteStatus(unsigned long gdata) {
  unsigned long lCommand = (gdata >> 24) & 0xff;
  printf("GPUwriteStatus %08x\n", gdata);
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
      displayArea.x = gdata & 0x03ff;
      displayArea.y = (gdata >> 10) & 0x01ff;
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
  }
}

////////////////////////////////////////////////////////////////////////
// core read from vram
////////////////////////////////////////////////////////////////////////

unsigned long CALLBACK GPUreadData(void) {
  return DATAREG;
}

// new function, used by ePSXe, for example, to read a whole chunk of data

void CALLBACK GPUreadDataMem(unsigned long * pMem, int iSize) {
  for (int row=0; row<GPUWrite.height; row++) {
    for (int col=0; col<GPUWrite.height; col++) {
      
    }
  }
}

void executeWriteTextureToVram(unsigned long * buffer, unsigned int count) {
  GPUWrite.x = GPUWrite.currentX = buffer[1] & 0xffff;
  GPUWrite.y = GPUWrite.currentY = (buffer[1] >> 16) & 0xffff;
  GPUWrite.width = buffer[2] & 0xffff;
  GPUWrite.height = (buffer[2] >> 16) & 0xffff;
}

void executeCommandWordBuffer(unsigned long buffer[256], unsigned int count) {
  unsigned long command = (buffer[0] >> 24) & 0xff;
  
  printf("Executing command %02x\n", command);
  switch (command) {
    case 0x00: // noop
      break;
    case 0x01: // docs unclear - clear fifo, like GP1(01h)? clear texture cache? ??
      break;
    case 0xa0: // write texture to vram
      executeWriteTextureToVram(buffer, count);
      break;
    default:
      printf("Command not yet implemented: %02x\n", command);
  }
}

////////////////////////////////////////////////////////////////////////
// core write to vram
////////////////////////////////////////////////////////////////////////

void CALLBACK GPUwriteData(unsigned long gdata) {
  unsigned long command = (gdata >> 24) & 0xff;
  printf("GPUwriteData %08x\n", gdata);

  commandWordsBuffer[commandWordsReceived] = gdata;
  if (commandWordsReceived == 0) {
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

// new function, used by ePSXe, for example, to write a whole chunk of data
// This represents a number (>= 1) of words written one-by-one to GPU0--
// For example, texture data via DMA2 following GP0(A0h) 
void CALLBACK GPUwriteDataMem(unsigned long * pMem, int iSize) {
  unsigned long * psxVul = (unsigned long *) psxVub;
  int longNum = 0;
  
  printf("GPUwriteDataMem called; %d things (words/pixelpairs?)\n", iSize);
  while ((GPUWrite.currentX < GPUWrite.x + GPUWrite.width) &&
         (GPUWrite.currentY < GPUWrite.y + GPUWrite.height) &&
         (longNum < iSize)) {
    // TODO: handle case when width not divisible by 4 pixels and might wrap mid-long
    psxVul[(GPUWrite.currentY * 1024 + GPUWrite.currentX)/4] = *(pMem + longNum);
    ++longNum;
    GPUWrite.currentX = (GPUWrite.currentX + 4) % GPUWrite.width;
    if (GPUWrite.currentX == 0) {
      ++GPUWrite.currentY;
    }
  }
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
  printf("GPUdmaChain()\n");
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
