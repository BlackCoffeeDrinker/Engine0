#include <Cocoa/Cocoa.h>

#include <utility>

#include "E0AppDelegate.h"
#include "E0Application.h"
#include "E0View.h"
#include "OSXInclude.hpp"
#include "events.h"

/* setAppleMenu disappeared from the headers in 10.4 */
@interface NSApplication (NSAppleMenu)
- (void)setAppleMenu:(NSMenu *)menu;
@end

namespace {
E0AppDelegate *appDelegate = nil;
NSWindow *window = nil;
E0View *view = nil;

void CreateApplicationMenus() {
  NSString *appName;
  NSString *title;
  NSMenu *appleMenu;
  NSMenu *serviceMenu;
  NSMenu *windowMenu;
  NSMenuItem *menuItem;
  NSMenu *mainMenu;

  if (NSApp == nil) {
    return;
  }

  mainMenu = [[NSMenu alloc] init];

  /* Create the main menu bar */
  [NSApp setMainMenu:mainMenu];

  /* Create the application menu */
  appName = @"Test";
  appleMenu = [[NSMenu alloc] initWithTitle:@""];

  /* Add menu items */
  title = [@"About " stringByAppendingString:appName];
  [appleMenu addItemWithTitle:title action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];

  [appleMenu addItem:[NSMenuItem separatorItem]];
  [appleMenu addItemWithTitle:@"Preferences…" action:nil keyEquivalent:@","];
  [appleMenu addItem:[NSMenuItem separatorItem]];

  serviceMenu = [[NSMenu alloc] initWithTitle:@""];
  menuItem = [appleMenu addItemWithTitle:@"Services" action:nil keyEquivalent:@""];
  [menuItem setSubmenu:serviceMenu];

  [NSApp setServicesMenu:serviceMenu];

  [appleMenu addItem:[NSMenuItem separatorItem]];

  title = [@"Hide " stringByAppendingString:appName];
  [appleMenu addItemWithTitle:title action:@selector(hide:) keyEquivalent:@"h"];

  menuItem = (NSMenuItem *) [appleMenu addItemWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@"h"];
  [menuItem setKeyEquivalentModifierMask:(NSEventModifierFlagOption | NSEventModifierFlagCommand)];

  [appleMenu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];

  [appleMenu addItem:[NSMenuItem separatorItem]];

  title = [@"Quit " stringByAppendingString:appName];
  [appleMenu addItemWithTitle:title action:@selector(terminate:) keyEquivalent:@"q"];

  /* Put menu into the menubar */
  menuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
  [menuItem setSubmenu:appleMenu];
  [[NSApp mainMenu] addItem:menuItem];

  /* Tell the application object that this is now the application menu */
  [NSApp setAppleMenu:appleMenu];

  /* Create the window menu */
  windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];

  /* Add menu items */
  [windowMenu addItemWithTitle:@"Close" action:@selector(performClose:) keyEquivalent:@"w"];
  [windowMenu addItemWithTitle:@"Minimize" action:@selector(performMiniaturize:) keyEquivalent:@"m"];
  [windowMenu addItemWithTitle:@"Zoom" action:@selector(performZoom:) keyEquivalent:@""];

  /* Add the fullscreen toggle menu option. */
  /* Cocoa should update the title to Enter or Exit Full Screen automatically.
     * But if not, then just fallback to Toggle Full Screen.
     */
  menuItem = [[NSMenuItem alloc] initWithTitle:@"Toggle Full Screen" action:@selector(toggleFullScreen:) keyEquivalent:@"f"];
  [menuItem setKeyEquivalentModifierMask:NSEventModifierFlagControl | NSEventModifierFlagCommand];
  [windowMenu addItem:menuItem];

  /* Put menu into the menubar */
  menuItem = [[NSMenuItem alloc] initWithTitle:@"Window" action:nil keyEquivalent:@""];
  [menuItem setSubmenu:windowMenu];
  [[NSApp mainMenu] addItem:menuItem];

  /* Tell the application object that this is now the window menu */
  [NSApp setWindowsMenu:windowMenu];
}
}// namespace

namespace platform {
std::string_view PlatformName() {
  return "OSX";
}

std::error_code Init() {
  if (const auto ec = SYS_Timer_Init()) {
    return ec;
  }

  @autoreleasepool {
    [E0Application sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    appDelegate = [[E0AppDelegate alloc] init];
    [NSApp setDelegate:appDelegate];

    if ([NSApp mainMenu] == nil) {
      CreateApplicationMenus();
    }

    [NSApp finishLaunching];
    [E0Application registerUserDefaults];

    /* Make the view */
    NSRect viewRect = NSMakeRect(0, 0, 640, 480);
    view = [[[E0View alloc] initWithFrame:viewRect] autorelease];

    /* Create Window */
    NSRect screenRect = [[NSScreen mainScreen] frame];
    NSRect windowRect = NSMakeRect(NSMidX(screenRect) - NSMidX(viewRect),
                                   NSMidY(screenRect) - NSMidY(viewRect),
                                   viewRect.size.width,
                                   viewRect.size.height);

    window = [[NSWindow alloc] initWithContentRect:windowRect
                                         styleMask:NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask | NSMiniaturizableWindowMask
                                           backing:NSBackingStoreBuffered
                                             defer:NO];
    [window setAcceptsMouseMovedEvents:YES];
    [window setTitle:[[NSProcessInfo processInfo] processName]];
    [window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    [window makeKeyAndOrderFront:nil];
    [window setContentView:view];

    NSWindowController *windowController = [[NSWindowController alloc] initWithWindow:window];
    [windowController autorelease];
  }

  return {};
}


void SetSettings(std::string_view key,
                 std::string_view value) {
}

void Exit() {
}

void Yield() {
}

void SetWindowTitle(const std::string_view &windowTitle) {
  @autoreleasepool {
    [window setTitle:[NSString stringWithCString:windowTitle.data()
                                          length:windowTitle.length()]];
  }
}

bool HasFocus() {
}

void ProcessEvents(e00::Engine &engine) {
  @autoreleasepool {
  }
}

void ProcessDraw(e00::Engine &engine) {
}

std::unique_ptr<e00::LoggerSink> CreateSink(const std::string &name) {
  class CocoaLogger : public e00::LoggerSink {
    const std::string _name;

  public:
    explicit CocoaLogger(std::string name) : _name(std::move(name)) {
    }

    ~CocoaLogger() override = default;

    void log(const e00::detail::LogMessage &msg) override {
      @autoreleasepool {
        id logLine = [NSString stringWithCString:msg.payload.data() length:msg.payload.length()];
        id where = [NSString stringWithCString:msg.location.file_name() encoding:[NSString defaultCStringEncoding]];
        id commonFormat = [NSString stringWithFormat:@"%@:%d %@", where, msg.location.line(), logLine];

        switch (msg.level) {
          case e00::L_VERBOSE:
            NSLog(@"[V] %@", commonFormat);
            break;
          case e00::L_INFO:
            NSLog(@"[I] %@", commonFormat);
            break;
          case e00::L_WARNING:
            NSLog(@"[W] %@", commonFormat);
            break;
          case e00::L_ERROR:
            NSLog(@"[E] %@", commonFormat);
            break;
          case e00::L_NONE:
            NSLog(@"[!] %@", commonFormat);
            break;
        }
      }
    }
    void flush() override {
    }
  };

  return std::make_unique<CocoaLogger>(name);
}

const std::unique_ptr<Surface> &GetMainSurface() {
}

std::unique_ptr<e00::Stream> OpenStream(const std::string_view &name) {
}

std::unique_ptr<e00::WritableStream> OpenStreamForWrite(const std::string_view &name) {
}

// Make a hardware surface
std::unique_ptr<Surface> CreateSurface(const e00::Vec2D<uint16_t> &size, e00::DrawableSurface::BitDepth depth) {
}


}// namespace platform
