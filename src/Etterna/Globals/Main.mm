#import <Cocoa/Cocoa.h>

// TODO: Maybe there is a better location for these to be implemented?
CGFloat scrolled;
float MACMouseScroll()
{
    CGFloat scrolledTmp = scrolled;
    scrolled = 0;
    return scrolledTmp;
}
float MACMouseX()
{
    NSRect frame = [[[NSApplication sharedApplication] mainWindow] frame];
    //NSPoint mouseLoc = [[[NSApplication sharedApplication] mainWindow] mouseLocationOutsideOfEventStream];
    NSPoint mouseLoc = [NSEvent mouseLocation];
    return mouseLoc.x - frame.origin.x - 3; //Needed for padding the release animation
}
float MACMouseY()
{
    NSRect frame = [[[NSApplication sharedApplication] mainWindow] frame];
    //NSPoint mouseLoc = [[[NSApplication sharedApplication] mainWindow] mouseLocationOutsideOfEventStream];
    NSPoint mouseLoc = [NSEvent mouseLocation];
    return frame.size.height - (mouseLoc.y - frame.origin.y - 1) - 10; //Appears to compensate for titlebar
    // This padding should be replaced in the future to use Cocoa calls to content
}
float MACWindowHeight()
{
    NSRect frame = [[[NSApplication sharedApplication] mainWindow] frame];
    return frame.size.height;
}
float MACWindowWidth()
{
    NSRect frame = [[[NSApplication sharedApplication] mainWindow] frame];
    return frame.size.width;
}

// Etterna AppDelegate. Here is where the window is created and then the game initialized
@interface AppDelegate:NSObject <NSApplicationDelegate>
@property(strong) NSWindow *window; // The core window
- (id)init;
@end

@implementation AppDelegate
- (id)init {

    // If self initilized...
    if(self = [super init]) {
        NSRect contentSize = NSMakeRect(10.0, 10.0, 1024.0, 576.0); // Default Window Size
        NSWindowStyleMask mask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
        self.window = [[NSWindow alloc] initWithContentRect:contentSize styleMask:mask backing:NSBackingStoreBuffered defer:false];
        self.window.backgroundColor = [NSColor blackColor];
        self.window.title = @"Etterna";
        [self.window center];
    }
    return self;
}

// - Delegate Methods
- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    [self.window makeKeyAndOrderFront:self]; // Make game window primary window
}
@end


/** @brief macOS Main. This is where it all starts! */
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        NSApplication *application = [NSApplication sharedApplication];

        AppDelegate *appDelegate = [[[AppDelegate alloc] init] autorelease];
        [application setDelegate:appDelegate];
        [application run];
    }
    return NSApplicationMain(argc, argv);
}
