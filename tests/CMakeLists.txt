# Test executable for the audio-testing study. It links the plugin's shared-code library (the `tapSynth`
# target that juce_add_plugin creates and the VST3/AU wrappers also link), so tests run the real processor.

include (FetchContent)
FetchContent_Declare (googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
set (gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set (INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable (googletest)

add_executable (tapSynth_tests
    main.cpp
    common/Render.cpp
    smoke/SmokeTest.cpp)

# The JUCE modules are linked PRIVATE into tapSynth, so reuse its include paths and definitions
# (JuceHeader.h, module headers, JucePlugin_* macros) instead of compiling the modules a second time
target_include_directories (tapSynth_tests PRIVATE
    "${PROJECT_SOURCE_DIR}/Source"
    $<TARGET_PROPERTY:tapSynth,INCLUDE_DIRECTORIES>)
target_compile_definitions (tapSynth_tests PRIVATE
    $<TARGET_PROPERTY:tapSynth,COMPILE_DEFINITIONS>)
target_compile_options (tapSynth_tests PRIVATE
    $<TARGET_PROPERTY:tapSynth,COMPILE_OPTIONS>)

target_link_libraries (tapSynth_tests PRIVATE tapSynth GTest::gtest)

include (GoogleTest)
gtest_discover_tests (tapSynth_tests DISCOVERY_TIMEOUT 60)
