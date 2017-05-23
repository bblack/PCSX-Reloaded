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
#include <OpenGL/gl.h>

@interface PiGLView : NSOpenGLView
- (void)clearBuffer:(BOOL)display;
@end

@implementation PiGLView
//- (void) init:(NSRect*)frameRect pixelFormat:(NSOpenGLPixelFormat*):format {
//  [super init:frameRect pixelFormat:format];
//}
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
    window = [NSWindow alloc];
    [window initWithContentRect:frame
                      styleMask:NSTitledWindowMask
                      backing:NSBackingStoreBuffered
                      defer:NO];
    
    NSRect glViewFrame = NSMakeRect(0, 0, width, height);
    NSOpenGLPixelFormatAttribute attrs[] = {};
    NSOpenGLPixelFormat* pixFmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    openGLView = [NSOpenGLView alloc];
    openGLView = [openGLView initWithFrame:glViewFrame pixelFormat:pixFmt];
    [openGLView reshape];
    
    [[window contentView] addSubview:openGLView];
    
    [window setBackgroundColor:[NSColor blueColor]];
    [window setTitle:@"pigl"];
    [window center];
    [window makeKeyAndOrderFront:NSApp];
    
  });
  
  // seems run must be in the above block. if run is split into a subsequent enqueued block, or they're both in such a block, the window never appears.
}

void makeCurrentContext(){
  [[openGLView openGLContext] makeCurrentContext];
}
