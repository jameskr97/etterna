#import <Cocoa/Cocoa.h>
#include <iostream>

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

@interface EtternaApplication:NSApplication {}

@end

@implementation EtternaApplication

@end

int main(int argc, const char * argv[]) {
    
    return NSApplicationMain(argc, argv);
}
