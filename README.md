# LAUTA FX — Stereo Creative Performance Processor

**LAUTA FX** is a professional stereo creative performance audio effects plugin built with C++ and JUCE. It is specifically designed for live electronic music performance, transitions, builds, breakdowns, and master/bus processing in **Ableton Live**.

---

## 1. Overview & Core Philosophy

During a live electronic performance or DJ set, managing dozens of individual plugin parameters is impractical. **LAUTA FX** distills complex, multi-stage audio manipulation into **Four Core Performance Macros**, while preserving full fine-tuning access to every individual module parameter.

- **Audio Stability First**: Zero dynamic memory allocations (`new`/`malloc`), zero file I/O, zero blocking locks, and zero network calls inside the real-time audio callback (`processBlock`).
- **Musical Parameter Scaling**: Non-linear interpolation and carefully shaped response curves ensure smooth, natural builds without sudden volume spikes or harsh clipping.
- **Natural Tail Preservation**: Returning the `SPACE` or `MOVEMENT` macros to zero instantly stops feeding new signal into the effects, but allows existing reverb and delay tails to ring out naturally.
- **Safety Ceiling Limiter**: An integrated transparent soft-knee safety limiter prevents runaway feedback loops, resonance spikes, and unexpected master output jumps.

---

## 2. Signal Flow Architecture

```
                      +-------------------+
                      |   Stereo Input    |
                      +---------+---------+
                                |
             +------------------+-------------------+
             | (Dry Reference Copy)                 |
             v                                      v
    +-----------------+                    +-----------------+
    |  Filter Stage   |                    |   Master Mix    |
    | (LowPass/HP/BP) |                    |    Crossfade    |
    +--------+--------+                    +--------+--------+
             |                                      ^
    +--------v--------+                             |
    | Saturation Stage|                             |
    |  (Auto-Makeup)  |                             |
    +--------+--------+                             |
             |                                      |
    +--------v--------+                             |
    |   Delay Stage   |                             |
    | (Tempo-Synced)  |                             |
    +--------+--------+                             |
             |                                      |
    +--------v--------+                             |
    |  Reverb Stage   |                             |
    | (Diffusion FDN) |                             |
    +--------+--------+                             |
             |                                      |
    +--------v--------+                             |
    |  Repeat/Glitch  |                             |
    | (Slicing Loop)  |                             |
    +--------+--------+                             |
             |                                      |
             +--------------> [Wet Signal] -------->+
                                                    |
                                           +--------v--------+
                                           | Safety Limiter  |
                                           +--------+--------+
                                                    |
                                           +--------v--------+
                                           |  Stereo Output  |
                                           +-----------------+
```

---

## 3. The Four Performance Macros

Each macro operates in a normalised range from `0.0` to `1.0`. At `0.0`, each macro is neutral:

| Macro | Musical Function | Coordinated Actions Across 0.0 → 1.0 |
| :--- | :--- | :--- |
| **ENERGY** | Build & Tension Coordinator | Sweeps filter cutoff dynamically (frequency drops or rises depending on mode), increases filter resonance, and drives warm saturation with automatic output-level compensation (+8 dB drive max). |
| **SPACE** | Spatial Wash & Ambience | Progressively blends stereo delay wet level (up to 45%) and algorithmic reverb wet level (up to 65%), while opening room size. Releasing to 0 preserves natural decay tails. |
| **MOVEMENT** | Rhythmic Motion & Excitement | Increases stereo delay feedback (up to 92%) and, beyond 0.35, smoothly engages deterministic rhythmic repeat slicing on the audio grid. |
| **WILD** | Controlled Glitch & Dramatic Tension | Shifts repeat division down to 1/16 and 1/32, applies texture decay, adds edge saturation (+6 dB), producing dramatic stutter fills without loss of tempo sync. |

---

## 4. Effect Modules Detail

1. **Filter Module** (`Source/DSP/FilterModule.h`):
   - Resonant State Variable Filter (TPT / ZDF topology).
   - Modes: LowPass, HighPass, BandPass.
   - Cutoff frequency: 20 Hz – 20,000 Hz (multiplicative smoothing, click-free).
   - Resonance (Q): 0.1 – 5.0.

2. **Saturation Module** (`Source/DSP/SaturationModule.h`):
   - Soft-knee hyperbolic tangent (`tanh`) waveshaper.
   - Dynamic makeup compensation: $Gain_{comp} = 1 / \sqrt{1 + 0.5 \cdot (Drive - 1)}$ prevents perceived volume explosion when driving hard.
   - Mix: 0% – 100%.

3. **Tempo-Synchronized Delay** (`Source/DSP/DelayModule.h`):
   - Dual left/right buffers with subtle ping-pong stereo offset.
   - Musical divisions: `1/16`, `1/8`, `1/8 D` (dotted eighth), `1/4`, `1/2`.
   - Feedback loop damping filter and soft saturation to guarantee absolute feedback stability.

4. **Reverb Module** (`Source/DSP/ReverbModule.h`):
   - High-density stereo algorithmic diffusion reverb.
   - Independent Room Size, High-frequency Damping, and Wet Mix controls.
   - Zero tail cutting on bypass / macro reset.

5. **Repeat / Glitch Module** (`Source/DSP/RepeatModule.h`):
   - Deterministic rhythmic audio looper / beat repeat.
   - Slices audio into musical intervals (`1/4`, `1/8`, `1/16`, `1/32`).
   - 5ms raised-cosine crossfade at loop boundaries prevents clicks or audio pops.
   - Progressive cycle decay for riser builds.

6. **Safety Limiter** (`Source/DSP/SafetyLimiter.h`):
   - Zero-latency transparent ceiling limiter operating above -0.45 dBFS.
   - Ensures no unexpected gain change or resonance burst damages PA systems or ears.

---

## 5. Factory Presets

- **INIT**: Flat neutral starting point. All macros at `0.0`. Clean passthrough.
- **GROOVE**: Subtle saturation warmth (+3.5 dB), 1/8 dotted delay groove, and subtle ambient space.
- **BUILD**: High energy tension with filter resonance sweep, saturation bite, delay feedback, and 1/16 stutter repeat.
- **BREAKDOWN**: Deep spatial reverb wash (90% size, 60% wet), 1/4 note delay, and warm lowpass filter roll-off.

---

## 6. Ableton Live Workflow & MIDI Mapping Guide

### Recommended Ableton Live Routing
1. **Master Channel Placement**:
   - In Ableton Live, navigate to **Plug-Ins** > **VST3** (or **Audio Units** on macOS) > **Lauta Audio** > **LAUTA FX**.
   - Drag **LAUTA FX** onto the **Master Track**, placing it directly before your master brickwall limiter.
2. **Return Channel Alternative**:
   - Can also be loaded on a Return Track (e.g. Return A) with `Master Mix` set to 100% for parallel dub-style processing.

### MIDI Hardware Mapping (e.g., MIDI Fighter Twister, APC40, Launch Control, Push)
1. Open Ableton Live's **MIDI Map Mode** (press `Ctrl + M` on Windows or `Cmd + M` on macOS).
2. Click on any of the four macro dials in the LAUTA FX user interface:
   - Click **ENERGY** and turn Knob 1 on your hardware controller.
   - Click **SPACE** and turn Knob 2 on your controller.
   - Click **MOVEMENT** and turn Knob 3 on your controller.
   - Click **WILD** and turn Knob 4 on your controller.
3. Turn off MIDI Map Mode (`Ctrl + M` / `Cmd + M`).
4. The 4 macros are now mapped for expressive tactile control.

### Live Performance Tips:
- Use **ENERGY** to build tension during the 16 bars before a drop.
- Turn **SPACE** up to create a washed-out breakdown; snap it back to `0.0` right as the kick drops — the reverb tail from the breakdown will decay smoothly into the drop.
- Tap **RESET MACROS** to immediately return all four macros to neutral without abrupt audio clicks.

---

## 7. Building from Source

### Prerequisites
- **CMake** 3.22 or higher
- **Ninja** or **Make**
- **C++20 Compiler**:
  - macOS: Apple Clang (Xcode 14+)
  - Windows: GCC 11+ (MinGW-w64) or MSVC 2019/2022

### macOS Build Instructions (VST3 & AU)
```bash
# 1. Clone repository
git clone <repo-url> lauta-fx
cd lauta-fx

# 2. Configure CMake (enables VST3 and AU automatically on macOS)
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# 3. Build VST3, AU, Standalone and Test Suite
cmake --build build --config Release

# 4. Built artifacts will be located at:
# VST3: build/LautaFX_artefacts/Release/VST3/LAUTA FX.vst3
# AU:   build/LautaFX_artefacts/Release/AU/LAUTA FX.component
```

To install on macOS for Ableton Live:
```bash
cp -R "build/LautaFX_artefacts/Release/VST3/LAUTA FX.vst3" ~/Library/Audio/Plug-Ins/VST3/
cp -R "build/LautaFX_artefacts/Release/AU/LAUTA FX.component" ~/Library/Audio/Plug-Ins/Components/
```

### Windows Build Instructions (VST3 & Standalone)
```powershell
# 1. Configure CMake
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# 2. Build Release artifacts
cmake --build build --config Release

# 3. Built binaries located at:
# VST3: build/LautaFX_artefacts/Release/VST3/LAUTA FX.vst3
# Standalone: build/LautaFX_artefacts/Release/Standalone/LAUTA FX.exe
```

To install on Windows for Ableton Live:
```powershell
Copy-Item -Recurse "build\LautaFX_artefacts\Release\VST3\LAUTA FX.vst3" "C:\Program Files\Common Files\VST3\"
```

---

## 8. Automated Test Suite Execution

A comprehensive automated DSP validation suite is included (`Tests/DSPTests.cpp`).

Run the test suite:
```bash
./build/LautaFX_Tests.exe   # Windows
# or
./build/LautaFX_Tests       # macOS / Linux
```

### Verified Test Cases:
1. `[TEST] 1. Instantiation and Bus Layout`: Confirms valid stereo-in / stereo-out bus negotiation and program count.
2. `[TEST] 2. Stereo Signal Flow`: Verifies independent dual-channel audio processing and stereo separation.
3. `[TEST] 3. Silence and Near-Silence`: Validates zero DC offset or noise generation when processing pure silence.
4. `[TEST] 4. Bypass and Reset Behavior`: Confirms 100% bit-exact passthrough when bypass is engaged.
5. `[TEST] 5. Parameter Boundaries & Finite Output`: Validates all parameters at extreme limits (0.0 to 1.0, 20Hz - 20kHz); verifies zero NaNs or Infs.
6. `[TEST] 6. State Save and Restore (Persistence)`: Tests XML serialization roundtrip and parameter preservation.
7. `[TEST] 7. Feedback Stability Under Runaway Conditions`: Injects full-scale impulse under 95% delay feedback and verifies output remains strictly bounded.
8. `[TEST] 8. Repeated Editor Opening & Closing Lifecycle`: Creates, renders, and destroys the plugin GUI 10 times in rapid succession to ensure leak-free lifecycle.

---

## 9. Current Limitations & Roadmap

- **Host Testing**: The plugin has been compiled and validated through standalone testing and automated test runners. While built to exact VST3 and AU specifications for Ableton Live, formal validation in live Ableton Live sessions requires running inside Ableton Live on the target machine.
- **Future Enhancements**:
  - Optional sidechain input for dynamic ducking of reverb/delay against the master kick drum.
  - Granular pitch-shifter freeze mode for experimental ambient sound design.
