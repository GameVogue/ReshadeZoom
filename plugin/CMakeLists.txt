add_library(reshade_test_plugin SHARED main.cpp)

target_include_directories(reshade_test_plugin PRIVATE ${CMAKE_SOURCE_DIR}/extern/reshade/include)

set_target_properties(reshade_test_plugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
