#include "Locator.hpp"

// Static variable default values
std::unique_ptr<ArchHooks> Locator::archHooks = nullptr;
std::unique_ptr<Core::ILogger> Locator::logger = nullptr;
std::unique_ptr<Core::IArchitecture> Locator::arch = nullptr;

ArchHooks *Locator::getArchHooks() {
    return archHooks.get();
}

Core::ILogger *Locator::getLogger() {
    return logger.get();
}

Core::IArchitecture *Locator::getArch() {
    return arch.get();
}

void Locator::provide(ArchHooks *hooks) {
    if(!Locator::archHooks)
        Locator::archHooks = std::unique_ptr<ArchHooks>(hooks);
}

void Locator::provide(Core::ILogger *logger) {
    if(!Locator::logger)
        Locator::logger = std::unique_ptr<Core::ILogger>(logger);
}

void Locator::provide(Core::IArchitecture *arch) {
    if(!Locator::arch)
        Locator::arch = std::unique_ptr<Core::IArchitecture>(arch);
}
