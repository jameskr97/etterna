# This collects all the information needed to be passed into GameInfo.in.cpp
# Git Hash Info
execute_process(COMMAND git describe --tags
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
  OUTPUT_VARIABLE PROJECT_GIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE)

# Build Date
string(TIMESTAMP PROJECT_BUILD_DATE "%B %d, %Y")