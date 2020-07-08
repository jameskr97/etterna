list(APPEND CORE_HPP
        Misc/PlogLogger.hpp
        Services/Locator.hpp)
list(APPEND CORE_CPP
        Services/IArchitecture.hpp
        Services/ILogger.cpp
        Services/Locator.cpp
        Misc/PlogLogger.cpp)

if(APPLE)
    list(APPEND CORE_HPP Arch/MacArchitecture.hpp)
    list(APPEND CORE_CPP Arch/MacArchitecture.mm)
endif()

target_sources(Etterna PRIVATE ${CORE_HPP} ${CORE_CPP})