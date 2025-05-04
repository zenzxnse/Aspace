# 🚀 Space‑Something (working title)

Arcade‑style top‑down space shooter written in **C++17** and **raylib 5**.  
Runs on Windows, Linux and macOS.

<p align="center">
  <img alt="screenshot" src="rsc/screenshots/ingame.gif" width="640">
</p>

---

## 1 · Features

* Pixel‑art ship with modular sprite‑parts (thrusters, weapons, …)
* Player‑controller wrapper that can swap to bigger ships later
* Camera that follows any `CameraTarget` entity
* Component‑based world grid for lightweight collision / culling
* Pure CMake build – no Makefile hacks – ships with raylib sources

---

## 2 · Prerequisites

|               | Windows                         | Linux / macOS                    |
|---------------|---------------------------------|----------------------------------|
| **Compiler**  | MinGW‑w64 10+ (auto‑downloaded with VS Code C/C++) | GCC 10+ / Clang 12+ |
| **CMake**     | ≥ 3.20                          | `sudo apt install cmake` or `brew install cmake` |
| **Ninja**     | *(optional)* `choco install ninja`  | `sudo apt install ninja-build` or `brew install ninja` |

> **No global raylib install is required** – the build pulls the exact tag we need.

---

## 3 · Getting the code

```bash
git clone --recursive https://github.com/your‑nick/space‑something.git
cd space‑something
