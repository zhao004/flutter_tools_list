# The Flutter tooling requires that developers have CMake 3.10 or later
# installed. You should not increase this version, as doing so will cause
# the plugin to fail to compile for some customers of the plugin.
cmake_minimum_required(VERSION 3.10)

project(flutter_tools_list_library VERSION 0.0.1 LANGUAGES C)

add_library(flutter_tools_list SHARED
  "process/process_tools.c"
)

set(HEADER_FILES
  "process/process_tools.h"
)
set_target_properties(flutter_tools_list PROPERTIES
  PUBLIC_HEADER "${HEADER_FILES}"
  OUTPUT_NAME "flutter_tools_list"
)

target_compile_definitions(flutter_tools_list PUBLIC DART_SHARED_LIB)

# Link required Windows libraries for network and process APIs
if(WIN32)
  target_link_libraries(flutter_tools_list PRIVATE iphlpapi ws2_32)
endif()

if (ANDROID)
  # Support Android 15 16k page size
  target_link_options(flutter_tools_list PRIVATE "-Wl,-z,max-page-size=16384")
endif()
