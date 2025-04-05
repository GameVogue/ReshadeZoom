add_library(zoomscope SHARED main.cpp)

target_include_directories(zoomscope PRIVATE ${CMAKE_SOURCE_DIR}/extern/reshade/include)
include_directories(${CMAKE_SOURCE_DIR}/extern/reshade)

set_target_properties(zoomscope PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
)
