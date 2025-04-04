add_library(zoomscope SHARED main.cpp)

target_include_directories(zoomscope PRIVATE ${CMAKE_SOURCE_DIR}/extern/reshade/include)

set_target_properties(zoomscope PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
