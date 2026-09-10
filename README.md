# Drum

Drum is a build system written in C++.

## Building

### Requirements

- C++23 compiler
- CMake
- Ninja
- LLVM/Clang

Configure and build the project with:

```bash
cmake --preset default
cmake --build build
```

### macOS

If using Homebrew LLVM, add `CMAKE_CXX_STDLIB_MODULES_JSON` to your preset.

Find the path with:

```bash
clang++ -print-file-name=c++/libc++.modules.json
```

Then add the resulting path to your `CMakeUserPresets.json`:

```json
"CMAKE_CXX_STDLIB_MODULES_JSON": "<path-to-libc++.modules.json>"
```
