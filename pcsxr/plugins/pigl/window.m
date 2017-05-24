//
//  window.m
//  Pcsxr
//
//  Created by Brian Black on 5/21/17.
//
//
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#include "window.h"
#import <OpenGL/gl.h>

@interface PiGLView : NSOpenGLView
- (id) initWithFrame:(NSRect)frameRect pixelFormat:(NSOpenGLPixelFormat*)format;
@end

@implementation PiGLView
- (id) initWithFrame:(NSRect)frameRect pixelFormat:(NSOpenGLPixelFormat *)format {
  self = [super initWithFrame:frameRect pixelFormat:format];
  return self;
}

- (void) reshape {
  [super reshape];
  //[[self openGLContext] makeCurrentContext];
  //glViewport(0, 0, 1024, 512);
  //glClearColor(1, 0, 0, 0);
  //glClear(GL_COLOR_BUFFER_BIT);
  //glFlush();
  //[[self openGLContext] flushBuffer];
}
@end

// Even though window is only used in a block in a function below, we keep a ref
// to it here. This is to prevent it from being removed immediately by ARC
// after creation, before it's ever shown.
// https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/ProgrammingWithObjectiveC/EncapsulatingData/EncapsulatingData.html
// https://stackoverflow.com/a/314311/406882
NSWindow * window; // apparently must have a strong ref,
NSOpenGLView * openGLView;

static inline void RunOnMainThreadSync(dispatch_block_t block) {
  if ([NSThread isMainThread]) {
    block();
  } else {
    dispatch_sync(dispatch_get_main_queue(), block);
  }
}

void initGLWindow(int width, int height, void * display) {
  RunOnMainThreadSync(^{
    NSRect frame = NSMakeRect(0, 0, width, height);
    NSWindowStyleMask styleMask =
      NSWindowStyleMaskTitled |
      NSWindowStyleMaskClosable;
    window = [NSWindow alloc];
    [window initWithContentRect:frame
                      styleMask:styleMask
                      backing:NSBackingStoreBuffered
                      defer:NO];
    
    NSRect glViewFrame = NSMakeRect(0, 0, width, height);
    NSOpenGLPixelFormatAttribute attrs[] = {};
    NSOpenGLPixelFormat* pixFmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    openGLView = [NSOpenGLView alloc];
    openGLView = [openGLView initWithFrame:glViewFrame pixelFormat:pixFmt];
    //[openGLView reshape];
    // perhaps we need to subclass NSOpenGLView so we can override reshape, which is a callback,
    // so that it calls GL draw functions? 
    
    [[window contentView] addSubview:openGLView];
    
    //[window setBackgroundColor:[NSColor blackColor]];
    [window setTitle:@"pigl"];
    [window center];
    [window makeKeyAndOrderFront:NSApp];
    
  });
  
  // seems run must be in the above block. if run is split into a subsequent enqueued block, or they're both in such a block, the window never appears.
}

void makeCurrentContext(){
  [[openGLView openGLContext] makeCurrentContext];
  [[openGLView openGLContext] update];
}

void flush(){
  [[openGLView openGLContext] flushBuffer];
}
