#ifndef CORE_MISC_GAMEINFO_HPP
#define CORE_MISC_GAMEINFO_HPP

#include <string>

/**
 * A namespace for game constants, or values defined at build time.
 */
namespace Core::GameInfo {
    extern const char* GAME_TITLE;
    extern const char* GAME_VERSION;
    extern const char* GIT_HASH;
    extern const char* BUILD_DATE;
}

#endif //CORE_MISC_GAMEINFO_HPP
