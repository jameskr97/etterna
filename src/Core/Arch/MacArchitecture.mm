#include "MacArchitecture.hpp"
#include "Core/Services/Locator.hpp"

#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>
#include <Metal/Metal.h>
#include <sys/sysctl.h>
#include <fmt/format.h>

std::string MacArchitecture::getSystem() const {
    return fmt::format("macOS {}", NSProcessInfo.processInfo.operatingSystemVersionString.UTF8String);
}

std::string MacArchitecture::getArchitecture() const {
    throw std::logic_error("Not Implemented");
}

std::string MacArchitecture::getKernel() const {
    auto type = MacArchitecture::getSysctlName("kern.ostype");
    auto release = MacArchitecture::getSysctlName("kern.osrelease");
    return fmt::format("{} Version {}", type, release);
}

std::size_t MacArchitecture::getSystemMemory() const {
    return NSProcessInfo.processInfo.physicalMemory;
}

std::string MacArchitecture::getSystemCPU() const {
    return MacArchitecture::getSysctlName("machdep.cpu.brand_string");
}

unsigned MacArchitecture::getSystemCores() const {
    return static_cast<unsigned>(NSProcessInfo.processInfo.activeProcessorCount);
}

std::string MacArchitecture::getSystemGPU() const {
    // MTLCreateSystemDefaultDevice will get the preferred gpu device
    return [MTLCreateSystemDefaultDevice() name].UTF8String;
}

Core::ScreenDimensions MacArchitecture::getScreenDimensions() const {
    auto dims = NSScreen.mainScreen.frame;
    return {dims.size.width, dims.size.height};
}

std::string MacArchitecture::getDefaultLanguage() const {
    return NSLocale.currentLocale.languageCode.UTF8String;
}

bool MacArchitecture::openWebsite(const std::string& url) const {
    // Allocate String and attempt to open
    NSURL *nsurl = [[NSURL alloc] initWithString:@(url.c_str())];
    bool successful = [NSWorkspace.sharedWorkspace openURL:(nsurl)];

    // Deallocate string and return success value.
    [nsurl release];
    return successful;
}

std::string MacArchitecture::getClipboard() const {
    // Get all clipboard items
    NSArray<NSPasteboardItem *> *items = NSPasteboard.generalPasteboard.pasteboardItems;

    // Return empty string if there is nothing in pasteboard.
    if (items.count == 0)
        return "";

    // Get first element in pasteboard, and return converted string.
    NSPasteboardItem *item = NSPasteboard.generalPasteboard.pasteboardItems[0];
    return [item stringForType:(NSPasteboardTypeString)].UTF8String;
}

/**
 * A helper function to make accessing sysctl names easier
 * @param name The sysctl name to access (full list in `man sysctl`)
 * @return A string of the desired value
 */
std::string MacArchitecture::getSysctlName(const char *name) {
    char buffer[200]; // Buffer to store result in
    size_t sz = sizeof(buffer);  // sysctlbyname will only take a reference to a size
    sysctlbyname(name, &buffer, &sz, nullptr, 0);
    return buffer;
}
