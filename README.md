# SimpleNES rasterm Integration

This reference application adapts the archived C++11
[amhndu/SimpleNES](https://github.com/amhndu/SimpleNES) emulator to present its native
256×240 framebuffer through Rasterm instead of SFML. Emulator audio, input, and timing
remain application responsibilities.

## Compatibility

- Windows x64 and Windows Terminal 1.22+; 1.23+ is recommended.
- Visual Studio 2022, CMake 3.20+, an installed rasterm package, and OpenCV `core` and
  `imgproc` from the same x64 MSVC configuration.
- NTSC ROMs using no mapper or mappers 1, 2, and 3 are the reliable upstream baseline.
  Mappers 4, 7, 11, and 66 remain experimental emulator behavior, not rasterm limits.

## Build

From the rasterm repository root:

```powershell
cmake -S apps/SimpleNES -B apps/SimpleNES/build -A x64 `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build apps/SimpleNES/build --config Release --parallel
```

## Run

```powershell
apps/SimpleNES/build/Release/SimpleNES.exe C:/Games/game.nes
```

`Escape` exits, `F2` pauses, `F3` advances one paused frame, and `F4`/`F5` adjust log
verbosity. Logs go to `simplenes.log`, never the active graphics stream.

Key bindings are read from `keybindings.conf`:

| Control | Player 1 | Player 2 |
|---|---|---|
| Start | Enter | Numpad 9 |
| Select | Right Shift | Numpad 8 |
| A / B | J / K | Numpad 5 / Numpad 6 |
| Direction | W / S / A / D | Arrow keys |

SimpleNES remains an integration/benchmark application and is not part of the installed
rasterm library or its compatibility guarantee.
