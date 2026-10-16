cmake_minimum_required (VERSION 3.22)

# Must be set before project() to take effect on macOS
set (CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version")

project (tapSynth VERSION 1.0.0 LANGUAGES C CXX)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Use a local JUCE checkout with -DTAPSYNTH_JUCE_DIR=/path/to/JUCE, otherwise fetch the pinned release
set (TAPSYNTH_JUCE_DIR "" CACHE PATH "Path to a local JUCE checkout (leave empty to download)")

if (TAPSYNTH_JUCE_DIR)
    add_subdirectory ("${TAPSYNTH_JUCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE")
else()
    include (FetchContent)
    FetchContent_Declare (JUCE
        URL https://github.com/juce-framework/JUCE/archive/refs/tags/8.0.9.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable (JUCE)
endif()

# AU is an Apple-only format
set (TAPSYNTH_FORMATS VST3)
if (APPLE)
    list (APPEND TAPSYNTH_FORMATS AU)
endif()

juce_add_plugin (tapSynth
    PRODUCT_NAME "tapSynth"
    COMPANY_NAME "tapSynth"
    BUNDLE_ID "com.tapsynth.tapSynth"
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE Cnks
    FORMATS ${TAPSYNTH_FORMATS}
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    NEEDS_CURL TRUE
    NEEDS_WEB_BROWSER FALSE
    VST3_CATEGORIES Instrument Synth
    AU_MAIN_TYPE kAudioUnitType_MusicDevice
    COPY_PLUGIN_AFTER_BUILD FALSE)

juce_generate_juce_header (tapSynth)

# IBM Plex Sans (SIL OFL 1.1, see Resources/Fonts/OFL.txt), embedded so the UI looks the same on every OS
juce_add_binary_data (tapSynthFonts SOURCES
    Resources/Fonts/IBMPlexSans-Regular.ttf
    Resources/Fonts/IBMPlexSans-Medium.ttf
    Resources/Fonts/IBMPlexSans-SemiBold.ttf)

target_sources (tapSynth PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/SynthVoice.cpp
    Source/Data/AdsrData.cpp
    Source/Data/FilterData.cpp
    Source/Data/MeterData.cpp
    Source/Data/OscData.cpp
    Source/UI/AdsrComponent.cpp
    Source/UI/Assets.cpp
    Source/UI/CustomComponent.cpp
    Source/UI/Displays.cpp
    Source/UI/FilterComponent.cpp
    Source/UI/LfoComponent.cpp
    Source/UI/MeterComponent.cpp
    Source/UI/OscComponent.cpp
    Source/UI/PromptBar.cpp
    Source/UI/ReverbComponent.cpp
    Source/UI/TapLookAndFeel.cpp)

target_compile_definitions (tapSynth PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=1
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1)

target_link_libraries (tapSynth
    PRIVATE
        tapSynthFonts
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags)
