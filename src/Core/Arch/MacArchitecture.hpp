#ifndef CORE_ARCH_MACARCHITECTURE_HPP
#define CORE_ARCH_MACARCHITECTURE_HPP

#include "Core/Services/IArchitecture.hpp"

/**
 * An implementation of IArchitecture specifically for macOS.
 */
class MacArchitecture : public Core::IArchitecture {
public:
    std::string getSystem() const override;

    std::string getArchitecture() const override;

    std::string getKernel() const override;

    std::size_t getSystemMemory() const override;

    std::string getSystemCPU() const override;

    unsigned getSystemCores() const override;

    std::vector<std::string> getSystemGPUS() const override;

    Core::ScreenDimensions getScreenDimensions() const override;

    std::string getDefaultLanguage() const override;

    bool openWebsite(const std::string& url) const override;

    std::string getClipboard() const override;

private:
    static std::string getSysctlName(const char* name);
};


#endif //CORE_ARCH_MACARCHITECTURE_HPP
