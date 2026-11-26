# Framework comparison (plan, "Comparação de frameworks"): the same pitch test in GoogleTest, Catch2 and doctest.
# Built only with -DTAPSYNTH_BUILD_FRAMEWORK_COMPARISON=ON (and the tests on, for GoogleTest). Not part of the
# experiment: the frozen suite in tests/ stays the only one the study runs.

include (FetchContent)
FetchContent_Declare (Catch2
    URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.7.1.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_Declare (doctest
    URL https://github.com/doctest/doctest/archive/refs/tags/v2.4.11.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
set (DOCTEST_WITH_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable (Catch2 doctest)

set (common_sources
    "${PROJECT_SOURCE_DIR}/tests/common/Analysis.cpp"
    "${PROJECT_SOURCE_DIR}/tests/common/Render.cpp")

foreach (framework gtest catch2 doctest)
    add_executable (pitch_${framework} ${framework}_pitch.cpp ${common_sources})

    # Same include paths, definitions and options as tapSynth_tests (tests/CMakeLists.txt)
    target_include_directories (pitch_${framework} PRIVATE
        "${PROJECT_SOURCE_DIR}/Source"
        $<TARGET_PROPERTY:tapSynth,INCLUDE_DIRECTORIES>)
    target_compile_definitions (pitch_${framework} PRIVATE $<TARGET_PROPERTY:tapSynth,COMPILE_DEFINITIONS>)
    target_compile_options (pitch_${framework} PRIVATE $<TARGET_PROPERTY:tapSynth,COMPILE_OPTIONS>)
endforeach()

target_link_libraries (pitch_gtest   PRIVATE tapSynth GTest::gtest)
target_link_libraries (pitch_catch2  PRIVATE tapSynth Catch2::Catch2)
target_link_libraries (pitch_doctest PRIVATE tapSynth doctest::doctest)
