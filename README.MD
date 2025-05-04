# Aspace

Aspace is a simple 2D top-down ship demo built with [Raylib](https://raylib.com/) and C++.

## Features

* Pixel‑art ship with modular sprite‑parts (thrusters, weapons, …)
* Player‑controller wrapper that can swap to bigger ships later
* Camera that follows any `CameraTarget` entity
* Component‑based world grid for lightweight collision / culling
* Pure CMake build – no Makefile hacks – ships with raylib sources

---

## Prerequisites

|               | Windows                         | Linux / macOS                    |
|---------------|---------------------------------|----------------------------------|
| **Compiler**  | MinGW‑w64 10+ (auto‑downloaded with VS Code C/C++) | GCC 10+ / Clang 12+ |
| **CMake**     | ≥ 3.20                          | `sudo apt install cmake` or `brew install cmake` |
| **Ninja**     | *(optional)* `choco install ninja`  | `sudo apt install ninja-build` or `brew install ninja` |

> **No global raylib install is required** – the build pulls the exact tag we need.

---

## Getting the code

```bash
git clone --recursive https://github.com/your‑nick/space‑something.git
cd space‑something
```

*Using `--recursive` is only needed if you keep raylib as a git‑submodule.
With FetchContent (default), a plain `git clone` is enough.*

---

```bash
# Generate build files (will fetch & compile Raylib automatically)
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug

# Please wait: the first configure step downloads Raylib (~5 MB) and builds it.

# Compile the game
cmake --build build
```

### Build (Release)

```bash
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Run

```bash
# On Linux/macOS
./build/bin/Aspace

# On Windows
build\\bin\\Aspace.exe
```

## Project Layout

```
├── CMakeLists.txt      # Build configuration (FetchContent for Raylib)
├── include/            # Public headers (Entity, World, Animator, etc.)
├── src/                # Game source files (main.cpp, BasicShip, PlayerController)
├── rsc/                # Resources (textures, spritesheets)
└── build/              # Out‑of‑source build directory
```

## License

Aspace is released under the MIT License. See [LICENSE](LICENSE) for details.

## Author

**Zenzxnse**
