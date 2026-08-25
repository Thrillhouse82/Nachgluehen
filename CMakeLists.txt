cmake_minimum_required(VERSION 3.22)
project(Nachgluehen VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.8
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

set(NACHGLUEHEN_SOURCES
    src/LivingFreezeEngine.cpp
    src/PluginProcessor.cpp
    src/PluginEditor.cpp)

juce_add_plugin(Nachgluehen
    COMPANY_NAME "Nachgluehen"
    BUNDLE_ID "de.nachgluehen.plugin"
    PLUGIN_MANUFACTURER_CODE Nglh
    PLUGIN_CODE Ng01
    FORMATS VST3
    PRODUCT_NAME "Nachgluehen"
    DESCRIPTION "A living freeze texture effect"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    MICROPHONE_PERMISSION_ENABLED FALSE
    COMPANY_WEBSITE ""
    COMPANY_EMAIL "")

target_sources(Nachgluehen PRIVATE ${NACHGLUEHEN_SOURCES})
target_include_directories(Nachgluehen PRIVATE src)
target_compile_definitions(Nachgluehen PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(Nachgluehen PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_extra
    juce::juce_dsp)

enable_testing()
add_executable(NachgluehenTests
    tests/TestMain.cpp
    src/LivingFreezeEngine.cpp
    src/PluginProcessor.cpp
    src/PluginEditor.cpp)
target_include_directories(NachgluehenTests PRIVATE src)
target_compile_definitions(NachgluehenTests PRIVATE JUCE_UNIT_TESTS=1 JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(NachgluehenTests PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp)
add_test(NAME NachgluehenTests COMMAND NachgluehenTests)
