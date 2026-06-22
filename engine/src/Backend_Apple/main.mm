#include "ApplePrivateInclude.h"
#include "Apple_KeyboardSystem.h"
#include "MetalSurface.h"

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include <atomic>
#include <cstring>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <chrono>

class e00::PlatformData {
public:
  std::chrono::steady_clock::time_point lastTick = std::chrono::steady_clock::now();
};

@interface EngineWindowDelegate : NSObject <NSWindowDelegate>
@end

namespace {
std::atomic<bool> g_running{false};
std::atomic<bool> g_hasFocus{true};
NSWindow *g_window = nil;
EngineWindowDelegate *g_windowDelegate = nil;
CAMetalLayer *g_layer = nil;

std::mutex g_eventMutex;
std::queue<NSEvent *> g_eventQueue;

id<MTLDevice> g_device = nil;
id<MTLCommandQueue> g_cmdQueue = nil;

std::unique_ptr<platform::Surface> g_mainSurface;
e00::Vec2D<uint16_t> g_requestedWindowSize{640, 480};
uint16_t g_integerScale = 1;

e00::Vec2D<uint16_t> ScaledWindowSize() {
  return {
      static_cast<uint16_t>(g_requestedWindowSize.x * g_integerScale),
      static_cast<uint16_t>(g_requestedWindowSize.y * g_integerScale)};
}

void RecreateMainSurface() {
    auto surface = std::make_unique<apple::MetalSurface>(
                                                         g_requestedWindowSize,
                                                         e00::DrawableSurface::BitDepth::DEPTH_8,
                                                         g_integerScale);
    @autoreleasepool {
        surface->setPresentTexture([g_device newTextureWithDescriptor:surface->getDescriptor()]);
    }

    g_mainSurface = std::move(surface);
}

apple::MetalSurface *MainAppleSurface() {
  return static_cast<apple::MetalSurface *>(g_mainSurface.get());
}

NSRect MakeWindowFrame() {
  const auto scaledSize = ScaledWindowSize();

  return NSMakeRect(
      100,
      100,
      static_cast<CGFloat>(scaledSize.x),
      static_cast<CGFloat>(scaledSize.y));
}

void CreateApplicationMenu() {
  NSString *appName = [[NSProcessInfo processInfo] processName];

  NSMenu *mainMenu = [[NSMenu alloc] initWithTitle:@""];
  NSMenuItem *appMenuItem = [[NSMenuItem alloc] initWithTitle:@""
                                                       action:nil
                                                keyEquivalent:@""];
  [mainMenu addItem:appMenuItem];

  NSMenu *appMenu = [[NSMenu alloc] initWithTitle:appName];

  NSString *aboutTitle = [NSString stringWithFormat:@"About %@", appName];
  NSMenuItem *aboutItem = [[NSMenuItem alloc] initWithTitle:aboutTitle
                                                     action:@selector(orderFrontStandardAboutPanel:)
                                              keyEquivalent:@""];
  [appMenu addItem:aboutItem];

  [appMenu addItem:[NSMenuItem separatorItem]];

  NSString *quitTitle = [NSString stringWithFormat:@"Quit %@", appName];
  NSMenuItem *quitItem = [[NSMenuItem alloc] initWithTitle:quitTitle
                                                    action:@selector(terminate:)
                                             keyEquivalent:@"q"];
  [appMenu addItem:quitItem];

  [mainMenu setSubmenu:appMenu forItem:appMenuItem];
  [NSApp setMainMenu:mainMenu];
}

void CreateCocoaWindow() {
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
  [NSApp finishLaunching];

  if ([NSWindow respondsToSelector:@selector(setAllowsAutomaticWindowTabbing:)]) {
    [NSWindow setAllowsAutomaticWindowTabbing:NO];
  }

  CreateApplicationMenu();
    
  NSRect frame = MakeWindowFrame();
  g_window = [[NSWindow alloc] initWithContentRect:frame
                                        styleMask:(NSWindowStyleMaskTitled |
                                                   NSWindowStyleMaskClosable |
                                                   NSWindowStyleMaskResizable |
                                                   NSWindowStyleMaskMiniaturizable)
                                          backing:NSBackingStoreBuffered
                                            defer:NO];

  [g_window setTitle:@"Engine0"];
  [g_window makeKeyAndOrderFront:nil];
  
  g_windowDelegate = [[EngineWindowDelegate alloc] init];
  [g_window setDelegate:g_windowDelegate];

  g_layer = [CAMetalLayer layer];
  g_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
  g_layer.frame = [[g_window contentView] bounds];
  g_layer.contentsScale = [NSScreen mainScreen].backingScaleFactor;
  g_layer.magnificationFilter = kCAFilterNearest;
  g_layer.drawableSize = CGSizeMake(
      static_cast<CGFloat>(g_requestedWindowSize.x),
      static_cast<CGFloat>(g_requestedWindowSize.y));

  NSView *view = [g_window contentView];
  [view setWantsLayer:YES];
  view.layer = g_layer;

  [NSApp activateIgnoringOtherApps:YES];
}
}// namespace

@implementation EngineWindowDelegate
- (BOOL)windowShouldClose:(id)sender {
  [sender orderOut:nil];
  return NO;
}
- (void)windowDidBecomeKey:(NSNotification *)notification {
  g_hasFocus = true;
}
- (void)windowDidResignKey:(NSNotification *)notification {
  g_hasFocus = false;
}
@end


namespace platform {
std::error_code Init() {
  @autoreleasepool {
    g_running = false;
    g_hasFocus = true;

    g_device = MTLCreateSystemDefaultDevice();
    if (!g_device) {
      return std::make_error_code(std::errc::no_such_device);
    }

   g_cmdQueue = [g_device newCommandQueue];
    if (!g_cmdQueue) {
      return std::make_error_code(std::errc::no_such_device);
    }
      
   RecreateMainSurface();
   CreateCocoaWindow();
   if (g_layer) {
     g_layer.device = g_device;
     g_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
     g_layer.framebufferOnly = NO;
     g_layer.magnificationFilter = kCAFilterNearest;
     g_layer.drawableSize = CGSizeMake(
        static_cast<CGFloat>(g_requestedWindowSize.x),
        static_cast<CGFloat>(g_requestedWindowSize.y));
   }
  
  }
    
  g_running = true;
  return {};
}

void Exit() {
  @autoreleasepool {
    g_running = false;

    if (g_window) {
      [g_window setDelegate:nil];
      [g_window close];
      g_window = nil;
    }

    g_windowDelegate = nil;
    g_layer = nil;
    g_mainSurface.reset();

    g_cmdQueue = nil;
    g_device = nil;
  }
}
void SetSettings(std::string_view key, std::string_view value) {
  bool sizeChanged = false;
  if (key == "width") {
    const auto newWidth = static_cast<uint16_t>(std::strtoul(value.data(), nullptr, 10));
    if (newWidth != g_requestedWindowSize.x) {
        g_requestedWindowSize.x = newWidth;
        sizeChanged = true;
    }
  } else if (key == "height") {
    const auto newHeight = static_cast<uint16_t>(std::strtoul(value.data(), nullptr, 10));
    if (newHeight != g_requestedWindowSize.y) {
        g_requestedWindowSize.y = newHeight;
        sizeChanged = true;
    }
  } else if (key == "scale" || key == "integer_scale") {
    const auto newScale = std::max<uint16_t>(static_cast<uint16_t>(std::strtoul(value.data(), nullptr, 10)), 1);
    if (newScale != g_integerScale) {
        g_integerScale = newScale;
        sizeChanged = true;
    }
  }

  if (sizeChanged && g_window) {
      @autoreleasepool {
          const auto scaled = ScaledWindowSize();
          [g_window setContentSize:NSMakeSize(scaled.x, scaled.y)];
          g_layer.frame = [[g_window contentView] bounds];
          g_layer.drawableSize = CGSizeMake(
              static_cast<CGFloat>(g_requestedWindowSize.x),
              static_cast<CGFloat>(g_requestedWindowSize.y));
          
          if (MainAppleSurface() && (MainAppleSurface()->Size() != g_requestedWindowSize || MainAppleSurface()->Scale() != g_integerScale)) {
              RecreateMainSurface();
          }
      }
  }
}

void Yield() {
  std::this_thread::yield();
}

void SetWindowTitle(e00::Engine &engine, const std::string_view &windowTitle) {
  if (!g_window) {
    return;
  }

  const auto title = std::string(windowTitle);
  NSString *nsTitle = [NSString stringWithUTF8String:title.c_str()];
  [g_window setTitle:nsTitle];
}

bool HasFocus(e00::Engine &engine) {
  return g_hasFocus;
}

void ProcessEvents(e00::Engine &engine) {
  auto *data = engine.GetPlatformData();
  if (!data) return;

  @autoreleasepool {
    // Poll all available events
    while (true) {
      NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                          untilDate:[NSDate distantPast]
                                             inMode:NSDefaultRunLoopMode
                                            dequeue:YES];

      if (!event) {
        break;
      }

      switch ([event type]) {
        case NSEventTypeKeyDown:
          if (([event modifierFlags] & NSEventModifierFlagCommand) != 0 &&
              [[event charactersIgnoringModifiers] isEqualToString:@"q"]) {
            engine.QueueActionForNextTick(e00::Engine::BuiltInAction_Quit());
            break;
          }

          if (![event isARepeat]) {
            engine.ProcessInputEvent(MakeAppleKey(event, e00::InputEvent::Type::KeyDown));
          }
          break;

        case NSEventTypeKeyUp:
          engine.ProcessInputEvent(MakeAppleKey(event, e00::InputEvent::Type::KeyUp));
          break;

        default:
          break;
      }

      [NSApp sendEvent:event];
    }

    [NSApp updateWindows];

    if (g_window) {
      if (![g_window isVisible]) {
        engine.QueueActionForNextTick(e00::Engine::BuiltInAction_Quit());
      }
    }
    
    // Ticking logic
    auto now = std::chrono::steady_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - data->lastTick);
    if (delta.count() > 0) {
        engine.Tick(delta);
        e00::ResourceManager::GlobalResourceManager().Tick(delta);
        data->lastTick += delta;
    }
    
    // Rate limiting to ~60fps - wait for next events with timeout
    now = std::chrono::steady_clock::now();
    auto nextFrameTime = data->lastTick + std::chrono::microseconds(16666);
    if (now < nextFrameTime) {
        const double waitTime = std::chrono::duration_cast<std::chrono::duration<double>>(nextFrameTime - now).count();
        NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                            untilDate:[NSDate dateWithTimeIntervalSinceNow:waitTime]
                                               inMode:NSDefaultRunLoopMode
                                              dequeue:YES];
        if (event) {
            switch ([event type]) {
                case NSEventTypeKeyDown:
                  if (([event modifierFlags] & NSEventModifierFlagCommand) != 0 &&
                      [[event charactersIgnoringModifiers] isEqualToString:@"q"]) {
                    engine.QueueActionForNextTick(e00::Engine::BuiltInAction_Quit());
                    break;
                  }

                  if (![event isARepeat]) {
                    engine.ProcessInputEvent(MakeAppleKey(event, e00::InputEvent::Type::KeyDown));
                  }
                  break;

                case NSEventTypeKeyUp:
                  engine.ProcessInputEvent(MakeAppleKey(event, e00::InputEvent::Type::KeyUp));
                  break;

                default:
                  break;
              }
              [NSApp sendEvent:event];
              
              // Drain any other events that might have arrived
              while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                                 untilDate:[NSDate distantPast]
                                                    inMode:NSDefaultRunLoopMode
                                                   dequeue:YES])) {
                  [NSApp sendEvent:event];
              }
        }
    }
  }
}

void ProcessDraw(e00::Engine &engine) {
  @autoreleasepool {
    auto *surface = MainAppleSurface();
    if (!surface || !g_layer || !g_device || !g_cmdQueue) {
      return;
    }

    {
      auto painter = GetMainSurface(engine).BeginDraw();
      engine.RootWidget()->Paint(*painter);
    }

    if (!surface->UploadToTexture(g_device)) {
      return;
    }

    id<MTLTexture> sourceTexture = surface->PresentTexture();
    if (!sourceTexture) {
      return;
    }

    id<CAMetalDrawable> drawable = [g_layer nextDrawable];
    if (!drawable) {
      return;
    }

    id<MTLCommandBuffer> commandBuffer = [g_cmdQueue commandBuffer];
    if (!commandBuffer) {
      return;
    }

    id<MTLBlitCommandEncoder> blit = [commandBuffer blitCommandEncoder];
    if (!blit) {
      return;
    }

    const NSUInteger copyWidth = std::min<NSUInteger>(
        [sourceTexture width],
        [[drawable texture] width]);

    const NSUInteger copyHeight = std::min<NSUInteger>(
        [sourceTexture height],
        [[drawable texture] height]);

    MTLOrigin sourceOrigin = {0, 0, 0};
    MTLSize sourceSize = {copyWidth, copyHeight, 1};
    MTLOrigin destinationOrigin = {0, 0, 0};

    [blit copyFromTexture:sourceTexture
              sourceSlice:0
              sourceLevel:0
             sourceOrigin:sourceOrigin
               sourceSize:sourceSize
                toTexture:[drawable texture]
         destinationSlice:0
         destinationLevel:0
        destinationOrigin:destinationOrigin];

    [blit endEncoding];

    [commandBuffer presentDrawable:drawable];
    [commandBuffer commit];
  }
}

Surface &GetMainSurface(e00::Engine &engine) {
  return *g_mainSurface;
}

Surface &GetMainSurface() {
  return *g_mainSurface;
}

bool InitEngine(e00::Engine &engine) {
  engine.SetPlatformData(new e00::PlatformData());
  return true;
}

void QuitEngine(e00::Engine &engine) {
  delete engine.GetPlatformData();
  engine.SetPlatformData(nullptr);
}

}// namespace platform

