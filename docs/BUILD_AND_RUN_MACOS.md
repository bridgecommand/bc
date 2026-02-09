# Building and Running Bridge Command on macOS (Apple Silicon)

Tested on: Mac Mini (arm64), macOS 15+

## Prerequisites

### Xcode

Xcode must be installed. If installed on an external drive:

```bash
sudo xcode-select -s /Volumes/storage/Applications/Xcode.app/Contents/Developer
```

### Homebrew Dependencies

```bash
brew install libsndfile portaudio libvorbis libogg flac opus mpg123 lame
```

These are transitive dependencies of the bundled static `libsndfile` and `libportaudio`.

## Build

```bash
cd /Users/pb/source/bc
cmake -S src -B build
cmake --build build -j$(sysctl -n hw.ncpu)
```

The build produces these binaries in `build/`:

| Binary | Description |
|--------|-------------|
| `bridgecommand` | Launcher (scenario picker) |
| `bridgecommand-bc` | Main simulator |
| `bridgecommand-ed` | Scenario editor |
| `bridgecommand-mc` | Map controller |
| `bridgecommand-mh` | Multiplayer hub |
| `bridgecommand-rp` | Repeater display |
| `bridgecommand-ini` | INI editor |
| `bc-tests` | Unit tests |

## Run Tests

```bash
./build/bc-tests
```

All tests should pass (currently 89 tests, 235 assertions).

## Run the Simulator

On macOS, Bridge Command requires an app bundle to launch (Irrlicht needs NSApplication/NIB).

### Copy built binaries into app bundle

```bash
cp build/bridgecommand    bin/BridgeCommand.app/Contents/MacOS/BridgeCommand
cp build/bridgecommand-bc bin/BridgeCommand.app/Contents/MacOS/bc.app/Contents/MacOS/bc
```

### Launch

```bash
open bin/BridgeCommand.app
```

Or double-click `bin/BridgeCommand.app` in Finder.

### App Bundle Structure

The app bundle must have this structure (nested apps under `Contents/MacOS/`, not `Helpers/`):

```
bin/BridgeCommand.app/
  Contents/
    Info.plist
    MacOS/
      BridgeCommand        <-- Launcher binary
      bc.app/              <-- Simulator (nested app)
        Contents/MacOS/bc
      ed.app/              <-- Editor
        Contents/MacOS/ed
      mc.app/              <-- Map controller
        Contents/MacOS/mc
      mh.app/              <-- Multiplayer hub
        Contents/MacOS/mh
      rp.app/              <-- Repeater
        Contents/MacOS/rp
      ini.app/             <-- INI editor
        Contents/MacOS/ini
    Resources/
      Models/    - Ship and buoy 3D models
      Scenarios/ - Scenario definitions
      Sounds/    - Audio files (engine, wave, horn, alarm)
      World/     - World terrain, heightmaps, textures
      bc5.ini    - Configuration file
```

The simulator binary navigates from its location up to `Contents/Resources/` to find data files
(`../../../../Resources` from `MacOS/bc.app/Contents/MacOS/`).

The `bc5.ini` in the app bundle is the template. User overrides go to `~/Library/Application Support/Bridge Command/bc5.ini` (created on first run).

## Configuration (bc5.ini)

Key settings for development:

```ini
graphics_mode=2          # 2=windowed (good for dev), 3=borderless fullscreen
graphics_width=1200      # Window width
graphics_height=900      # Window height
water_segments=32        # FFT grid size (power of 2)
disable_shaders=0        # 0=realistic water, 1=fast flat water
debug_mode=1             # Show debug info overlay
```

## Companion Tools

To update a companion tool after rebuilding:

```bash
cp build/bridgecommand-ed bin/BridgeCommand.app/Contents/MacOS/ed.app/Contents/MacOS/ed
cp build/bridgecommand-mc bin/BridgeCommand.app/Contents/MacOS/mc.app/Contents/MacOS/mc
cp build/bridgecommand-mh bin/BridgeCommand.app/Contents/MacOS/mh.app/Contents/MacOS/mh
cp build/bridgecommand-rp bin/BridgeCommand.app/Contents/MacOS/rp.app/Contents/MacOS/rp
cp build/bridgecommand-ini bin/BridgeCommand.app/Contents/MacOS/ini.app/Contents/MacOS/ini
```

## Notes

- OpenXR (VR) is disabled on macOS (`find_package(OpenXR)` is skipped for Apple)
- OpenAL uses the macOS system framework (no Homebrew package needed)
- Build is arm64-only (set in CMakeLists.txt via `CMAKE_OSX_ARCHITECTURES`)
- GDAL is optional; if installed (`brew install gdal`), chart integration is enabled
