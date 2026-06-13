# Developer Guide: Integrating Essentia with JUCE

> **Scope**  
> A concise, copy-pastable recipe for embedding any Essentia *standard* algorithm (RMS, Energy, SpectralCentroid, etc.) into a real-time JUCE plug-in.  
> For streaming/overlap networks see the appendix at the end.

---

## 1. Prerequisites

| Dependency | Version tested | Notes |
|------------|---------------|-------|
| JUCE       | 8.x           | CMake *or* Projucer projects work. |
| Essentia   | 2.1+ (built as static **or** dynamic library) | Build with `ESSENTIA_BUILD_STANDARD` enabled. |
| C++        | C++17         | (Needed by both JUCE and Essentia builds.) |

Add Essentia's include path and the compiled library to your plug-in target.

---

## 2. Template: Four-step integration

Below `**bold**` items are the *only* parts you change for a new algorithm.

```cpp
// Processor.h
#include <essentia/algorithmfactory.h>

class MyProcessor : public juce::AudioProcessor
{
    // ...
private:
    // (1) Pointer(s) to Essentia algorithms
    essentia::standard::Algorithm* **alg** { nullptr };

    // (2) Buffers for inputs / outputs
    std::vector<essentia::Real> **inBuf**;
    std::vector<essentia::Real> **outBuf**;  // if needed
    essentia::Real              **scalarOut** {}; // if needed
};
```

### 2.1 `prepareToPlay`

```cpp
void MyProcessor::prepareToPlay (double /*sr*/, int /*maxBlock*/)
{
    essentia::init();
    auto& factory = essentia::standard::AlgorithmFactory::instance();

    // (3) create algorithm
    **alg** = factory.create("**AlgorithmName**");          // e.g. "RMS"

    // (4) allocate buffers
    **inBuf**.resize (getBlockSize());

    // (5) connect ports once
    **alg**->input ("**inputName**" ).set (**inBuf**);
    **alg**->output("**outputName**").set (**scalarOut** /*or outBuf*/);

    // (6) prime to avoid audio-thread allocations
    std::fill (**inBuf**.begin(), **inBuf**.end(), 0.f);
    **alg**->compute();
}
```

### 2.2 `processBlock`

```cpp
void MyProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                juce::MidiBuffer&)
{
    const int n = buffer.getNumSamples();

    // (7) copy JUCE samples to Essentia vector
    const float* read = buffer.getReadPointer (0);      // mono example
    **inBuf**.assign (read, read + n);

    // (8) run algorithm
    **alg**->compute();

    // (9) use results
    //     *scalarOut   // scalar
    //     outBuf[k]    // vector
}
```

### 2.3 `releaseResources`

```cpp
void MyProcessor::releaseResources()
{
    delete **alg**;
    **alg** = nullptr;
    essentia::shutdown();
}
```

---

## 3. Cheat-sheet: Port names & buffer shapes

| Algorithm class | Input port | Output type | Remarks |
|-----------------|-----------|-------------|---------|
| Frame statistic (RMS, Energy...) | `array` | scalar `rms`, `energy`, ... | Works on any vector length. |
| Frame transform (Windowing, FFT) | `frame` | vectors `magnitude`, `phase` | Needs zero-padding only for specific FFT sizes. |
| Pitch / Onset estimators | `signal` **or** `spectrum` | scalar | Usually better with the *streaming* API. |
| Full feature chains | many | many | Assemble with Essentia Streaming and feed overlapped frames. |

---

## 4. Mini example: RMS meter

```cpp
// Prepare (excerpt)
rmsAlg   = factory.create("RMS");
rmsInBuf.resize (getBlockSize());
rmsAlg->input("array").set (rmsInBuf);
rmsAlg->output("rms").set (rmsValue);
std::fill (rmsInBuf.begin(), rmsInBuf.end(), 0.f);
rmsAlg->compute();

// Process
rmsInBuf.assign (buffer.getReadPointer (0),
                 buffer.getReadPointer (0) + buffer.getNumSamples());
rmsAlg->compute();
float dBFS = essentia::amp2db (rmsValue);
```

Displays -3.01 dBFS for a full-scale sine wave.
