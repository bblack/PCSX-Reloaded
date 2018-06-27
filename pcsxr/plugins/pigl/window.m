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

// Even though window is only used in a block in a function below, we keep a ref
// to it here. This is to prevent it from being removed immediately by ARC
// after creation, before it's ever shown.
// https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/ProgrammingWithObjectiveC/EncapsulatingData/EncapsulatingData.html
// https://stackoverflow.com/a/314311/406882
NSWindow * window; // apparently must have a strong ref,
NSOpenGLView * openGLView;
NSWindow * window2;
NSOpenGLView * openGLView2;

static inline void RunOnMainThreadSync(dispatch_block_t block) {
  if ([NSThread isMainThread]) {
    block();
  } else {
    dispatch_sync(dispatch_get_main_queue(), block);
  }
}

void initGLWindow(int width, int height) {
  RunOnMainThreadSync(^{
    NSRect frame = NSMakeRect(0, 0, width, height);
    NSWindowStyleMask styleMask =
      NSWindowStyleMaskTitled |
      NSWindowStyleMaskClosable;
    window = [[NSWindow alloc] initWithContentRect:frame
      styleMask:styleMask
      backing:NSBackingStoreBuffered
      defer:NO];
    
    NSRect glViewFrame = NSMakeRect(0, 0, width, height);
    NSOpenGLPixelFormatAttribute attrs[] = {};
    NSOpenGLPixelFormat* pixFmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    openGLView = [NSOpenGLView alloc];
    openGLView = [openGLView initWithFrame:glViewFrame pixelFormat:pixFmt];
    
    [[window contentView] addSubview:openGLView];
    
    [window setTitle:@"pigl"];
    [window center];
    [window makeKeyAndOrderFront:NSApp];
  });
}

void initScreenWindow(int width, int height) {
  RunOnMainThreadSync(^{
    NSRect frame = NSMakeRect(0, 0, width, height);
    NSWindowStyleMask styleMask =
      NSWindowStyleMaskTitled |
      NSWindowStyleMaskClosable;
    window = [[NSWindow alloc] initWithContentRect:frame
      styleMask:styleMask
      backing:NSBackingStoreBuffered
      defer:NO];
    
    NSRect glViewFrame = NSMakeRect(0, 0, width, height);
    NSOpenGLPixelFormatAttribute attrs[] = {};
    NSOpenGLPixelFormat* pixFmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    window2 = [[NSWindow alloc] initWithContentRect:frame
                                          styleMask:styleMask
                                            backing:NSBackingStoreBuffered
                                              defer:NO];
    
    openGLView2 = [NSOpenGLView alloc];
    openGLView2 = [openGLView2 initWithFrame:glViewFrame pixelFormat:pixFmt];
    
    [[window2 contentView] addSubview:openGLView2];
    
    [window2 setTitle:@"pigl2"];
    [window2 makeKeyAndOrderFront:NSApp];
  });
}

void makeCurrentContext(){
  [[openGLView openGLContext] makeCurrentContext];
}

void makeCurrentContext2(){
  [[openGLView2 openGLContext] makeCurrentContext];
}
