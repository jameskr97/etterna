#ifndef CORE_SERVICE_IARCHITECTURE_HPP
#define CORE_SERVICE_IARCHITECTURE_HPP

#include <string>
#include <utility>
#include <vector>

namespace Core {
/**
 * A ScreenDimension struct to access width and height by name.
 */
struct ScreenDimensions {
    double width, height;
};

/**
 * A class to handle all system specific tasks. Intended to be implemented once
 * for each operating system/kernel.
 */
class IArchitecture {
public:
    /**
     * Determine the system name with the version number.
     * - Unix should return it's distributions name (Ubuntu 18.04, Debian 9, ArchLinux, Xubuntu...)
     * - Windows should return "Windows 10"
     * - Apple systems should return "macOS 10.15.4"
     */
    virtual std::string getSystem() const = 0;

    /**
     * Determine if the current system is 64bit or 32bit.
     * - If 32bit, return "i386"
     * - If 64bit, return "x86"
     */
    virtual std::string getArchitecture() const = 0;

    /**
     * Determine the current systems kernel type and version
     * - Windows should return "Windows NT"
     * - Linux should return "Linux Kernel <version number>"
     * - Mac should return "Darwin"
     */
    virtual std::string getKernel() const = 0;

    /**
     * Determine the total amount of memory available on a system.
     * @return RAM in gigabytes
     */
    virtual std::size_t getSystemMemory() const = 0;

    /**
     * Get the system CPU information
     * String should include
     * - CPU Manufacturer
     * - CPU Model
     * - CPU Frequency
     */
    virtual std::string getSystemCPU() const = 0;

    /**
     * Get the number of CPU's available on the system.
     */
    virtual unsigned getSystemCores() const = 0;

    /**
     * Get the system GPU information
     * String should include
     * - GPU Manufacturer
     * - GPU Model
     * - GPU VRAM
     */
    virtual std::string getSystemGPU() const = 0;
    
    /**
     * Get the screen dimensions of the screen the game window is currently displayed on.
     * @return A pair with the first being the width, and second, height
     */
    virtual ScreenDimensions getScreenDimensions() const = 0;

    /**
     * Get the operating system's primary language.
     * @return A 2-letter RFC-639 language code.
     */
    virtual std::string getDefaultLanguage() const = 0;

    /**
     * Send a URL to be opened with the system's default web browser
     * @param url URL to open
     * @return true if successful, false if unsuccessful.
     */
    virtual bool openWebsite(const std::string& url) const = 0;

    /**
     * Get contents of the system clipboard.
     *
     * Return empty string if empty, or if clipboard is non-text content.
     * @return A string of the clipboard contents.
     */
    virtual std::string getClipboard() const = 0;
};
}

#endif //CORE_SERVICE_IARCHITECTURE_HPP