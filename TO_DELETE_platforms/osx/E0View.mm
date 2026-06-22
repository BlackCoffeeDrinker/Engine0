
#include "E0View.h"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

@implementation E0View
/* Return a Metal-compatible layer. */
+ (Class)layerClass {
  return NSClassFromString(@"CAMetalLayer");
}

/* Indicate the view wants to draw using a backing layer instead of drawRect. */
- (BOOL)wantsUpdateLayer {
  return YES;
}

/* When the wantsLayer property is set to YES, this method will be invoked to
 * return a layer instance.
 */
- (CALayer *)makeBackingLayer {
  return [self.class.layerClass layer];
}

- (instancetype)initWithFrame:(NSRect)frame
                      highDPI:(BOOL)highDpi
                     windowID:(uint32_t)windowId {
  self = [super initWithFrame:frame];
  if (self != nil) {
    self.highDPI = highDpi;
    self.windowID = windowId;
    self.wantsLayer = YES;

    [self updateDrawableSize];
  }
  return self;
}

- (void)updateDrawableSize {
  auto metalLayer = (CAMetalLayer *) self.layer;
  NSSize size = self.bounds.size;
  NSSize backingSize = size;

  if (self.highDPI) {
    /* Note: NSHighResolutionCapable must be set to true in the app's
         * Info.plist in order for the backing size to be highres.
         */
    backingSize = [self convertSizeToBacking:size];
  }

  metalLayer.contentsScale = backingSize.height / size.height;
  metalLayer.drawableSize = NSSizeToCGSize(backingSize);
}

- (NSView *)hitTest:(NSPoint)point {
  return nil;
}


@end
