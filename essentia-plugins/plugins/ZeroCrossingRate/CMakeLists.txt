project(ZCRAnalyzerPlugin VERSION 0.1.0)
set_property(GLOBAL PROPERTY USE_FOLDERS YES)

# Create binary data for assets
juce_add_binary_data(ZCRAnalyzerBinaryData
    SOURCES
        ../assets/upflogo.png
        ../assets/essentia_logo.png
)

juce_add_plugin(
    ${PROJECT_NAME}
    COMPANY_NAME              "Music Technology Group"
    BUNDLE_ID                 "com.MusicTechnologyGroup.ZCRAnalyzer"
    IS_SYNTH                  FALSE
    NEEDS_MIDI_INPUT          FALSE
    NEEDS_MIDI_OUTPUT         FALSE
    PLUGIN_MANUFACTURER_CODE  Mtge
    PLUGIN_CODE               Zcra
    COPY_PLUGIN_AFTER_BUILD   ${COPY_PLUGIN_AFTER_BUILD}
    PLUGIN_INSTALL_DIR        "${PLUGIN_INSTALL_DIR}"
    FORMATS                   ${PLUGIN_FORMATS}
    PRODUCT_NAME              "ZCR Analyzer"
)

# point sources to src/
target_sources(${PROJECT_NAME}
    PRIVATE
        src/PluginEditor.cpp
        src/PluginProcessor.cpp
)

# add the public include directory
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${ESSENTIA_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ZCRAnalyzerBinaryData
        essentia
        Eigen3::Eigen
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)