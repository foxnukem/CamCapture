# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\CamCapture_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\CamCapture_autogen.dir\\ParseCache.txt"
  "CamCapture_autogen"
  )
endif()
