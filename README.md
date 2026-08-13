# SWA Tronix 2A

**StreetWave Audio** • VST3 / Standalone optical compressor

SWA Tronix 2A is an original software compressor inspired by classic tube/optical leveling amplifiers. It is not affiliated with or endorsed by Teletronix, Universal Audio, or any other manufacturer.

## Highlights

- Optical-style level detector with slow attack/release behaviour.
- Compress / Limit modes.
- Input, Peak Reduction, Gain, Mix, Tube Color and Detector HPF controls.
- Auto make-up gain.
- Gain-reduction meter.
- 10 factory presets.
- Four visual themes: Photorealistic vintage-inspired, Modern, Neon Purple, and Animated Motion.
- Fully resizable editor from 720x480 to 1800x1200.
- Parameter state saved/restored through JUCE's AudioProcessorValueTreeState.
- CMake project that can fetch JUCE automatically for easy GitHub builds.

## Build locally

### Windows 11 + Visual Studio 2022/2026

```powershell
cmake -B build -S . -DJUCE_BUILD_EXAMPLES=OFF
cmake --build build --config Release
```

The VST3 is copied to the standard build output location by JUCE's CMake helper.

### Linux

```bash
sudo apt install build-essential cmake ninja-build libasound2-dev libfreetype6-dev libfontconfig1-dev libgl1-mesa-dev libx11-dev libxcomposite-dev libxcursor-dev libxext-dev libxinerama-dev libxrandr-dev
cmake -B build -S . -G Ninja
cmake --build build --config Release
```

### macOS

```bash
cmake -B build -S .
cmake --build build --config Release
```

## GitHub Actions

The repository includes a workflow that builds the plug-in on Windows, Ubuntu and macOS. Push the folder to a GitHub repository and GitHub Actions will compile it automatically.

## Important design note

The DSP and front panel are an original implementation designed to evoke the behaviour and visual language of classic optical compressors, rather than copying proprietary source code, branding, or trade dress.
