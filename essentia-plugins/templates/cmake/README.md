# Essentia JUCE Plugin CMake Template

This template provides a starting point for creating audio plugins using JUCE and Essentia. It includes a basic plugin structure with CMake build configuration.

## Prerequisites

- CMake 3.22 or higher
- JUCE 8.x
- Essentia 2.1+ (built as static or dynamic library)
- C++17 compatible compiler
- Eigen3 (required by Essentia)

## Getting Started

1. **Copy the Template**
   ```bash
   cp -r templates/cmake/MyPlugin your-plugin-name
   cd your-plugin-name
   ```

2. **Configure Your Plugin**
   Edit `CMakeLists.txt` and update the following:
   - Project name
   - Company name
   - Bundle ID
   - Plugin manufacturer code
   - Plugin code
   - Product name

3. **Build Configuration**
   ```bash
   # Create build directory
   mkdir build && cd build

   # Configure with CMake
   cmake .. -DCMAKE_PREFIX_PATH="path/to/essentia/install" -DCOPY_PLUGIN_AFTER_BUILD=ON

   # Build
   cmake --build . --parallel
   ```

   Available CMake options:
   | Variable                    | Default | Description                                                               |
   |-----------------------------|---------|---------------------------------------------------------------------------|
   | `COPY_PLUGIN_AFTER_BUILD`   | `OFF`   | Automatically install the plugin after every successful build             |
   | `CUSTOM_PLUGIN_INSTALL_DIR` | *(none)*| Override the destination path used when copying the plugin                |
   | `PLUGIN_FORMATS`            | `"VST3"` | Semicolon‑list of JUCE plugin formats (e.g. `"VST3;AU"`)               |

   > If `COPY_PLUGIN_AFTER_BUILD` is `ON`, the resulting `.vst3` is copied automatically to the user plugin folder  
   > (e.g. `~/Library/Audio/Plug‑Ins/VST3` on macOS).

## Project Structure

```
.
└── MyPlugin
    ├── CMakeLists.txt
    ├── include
    │   └── Audio2Midi
    │       ├── PluginEditor.h
    │       └── PluginProcessor.h
    └── src
        ├── PluginEditor.cpp
        └── PluginProcessor.cpp
```

### Header Organization

The `include` directory contains the public headers for your plugin. When creating a new plugin:

1. Create a new directory in `include` with your plugin name:
   ```bash
   mkdir plguins/YourPluginName
   ```

2. Add your public headers:
   ```cpp
   // include/YourPluginName/PluginProcessor.h
   #pragma once
   #include <JuceHeader.h>
   #include <essentia/algorithmfactory.h>

   class YourPluginProcessor : public juce::AudioProcessor
   {
       // ... your processor implementation
   };
   ```

3. Update `CMakeLists.txt` to include your headers:
   ```cmake
   target_include_directories(${PROJECT_NAME}
       PUBLIC
           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:include>
   )
   ```

## Features

- JUCE audio plugin template with Essentia integration
- CMake build system
- Support for multiple plugin formats (VST3, AU, Standalone)
- Basic plugin structure with processor and editor
- Essentia algorithm integration ready

## Customization

### Adding Essentia Algorithms

1. Include the necessary Essentia headers in your processor:
   ```cpp
   #include <essentia/algorithmfactory.h>
   ```

2. Add algorithm instances to your processor class:
   ```cpp
   private:
       std::unique_ptr<essentia::standard::Algorithm> myAlgorithm;
   ```

3. Initialize in `prepareToPlay`:
   ```cpp
   void MyProcessor::prepareToPlay(double sampleRate, int maxBlockSize)
   {
       essentia::init();
       auto& factory = essentia::standard::AlgorithmFactory::instance();
       myAlgorithm = std::unique_ptr<essentia::standard::Algorithm>(
           factory.create("AlgorithmName"));
       // ... configure algorithm
   }
   ```

### Adding Dependencies

To add new dependencies:

1. Add the dependency to `CMakeLists.txt`:
   ```cmake
   find_package(YourDependency REQUIRED)
   ```

2. Link against your plugin target:
   ```cmake
   target_link_libraries(${PROJECT_NAME}
       PRIVATE
           YourDependency::YourDependency
   )
   ```

## Common Issues

1. **Essentia Not Found**
   - Ensure `CMAKE_PREFIX_PATH` points to your Essentia installation
   - Check that Essentia is built with the same compiler as JUCE
   - Verify that Essentia's include and lib directories are properly structured

2. **Build Errors**
   - Verify all dependencies are installed and found by CMake
   - Check compiler compatibility (C++17 required)
   - Ensure JUCE paths are correctly set
   - Check that your compiler supports the required warning flags

3. **Plugin Not Loading**
   - Verify plugin format is supported by your DAW
   - Check plugin installation directory
   - Ensure all dependencies are available at runtime
   - On macOS, check that the plugin is properly signed

## Contributing

Feel free to submit issues and enhancement requests!

## License

This template is provided under the same license as your project. 