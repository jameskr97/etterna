#ifndef STEP_MANIA_H
#define STEP_MANIA_H

#include <string>

struct Game;
class RageTimer;
class VideoModeParams;

int sm_main (int argc, char *argv[]);

/** @brief Utility functions for controlling the whole game. */
namespace StepMania {
    void ApplyGraphicOptions();
    void ResetPreferences();
    void ResetGame();
    std::string GetInitialScreen();
    void InitializeCurrentGame(const Game *g);
    // If successful, return filename of screenshot in sDir, else return ""
    std::string SaveScreenshot(const std::string &Dir, bool SaveCompressed, bool MakeSignature,
                               const std::string &NamePrefix, const std::string &NameSuffix);
    void GetPreferredVideoModeParams(VideoModeParams &paramsOut);
}

#endif
