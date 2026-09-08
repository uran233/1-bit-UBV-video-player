# 🎬 1-Bit UBV Video Player

A custom, high-performance monochrome video processing pipeline and playback engine built from scratch. It features a proprietary 1-bit binary container format (`.ubv`), a Python conversion tool, and an optimized **x86 Assembly bit-unpacking engine** integrated with **SDL3**.

---

## 🛠️ Architecture & Pipeline

The project splits video processing into three distinct layers:

1. **Preprocessing (`konwert.py`)**: Uses `ffmpeg` to scale input video to 640x480, lock the framerate to 25 FPS, invert monochrome values, and pack raw 1-bit pixel streams into the custom `.ubv` binary structure.
2. **Low-Level Bit Unpacking (`engine.asm`)**: An optimized x86 Assembly routine (`cdecl`) that reads packed 1-bit byte buffers, isolates individual bits using bitwise shifts/tests, and unpacks them directly into a 32-bit `XRGB8888` texture buffer.
3. **Application & Playback (`player.c`)**: Written in C using SDL3. Handles binary header parsing, texture stream locking, automatic letterbox presentation scaling, and real-time playback/seeking controls.

---

## 📁 `.ubv` Binary Specification

The **Uranko Bitmap Video** container uses a compact 32-byte binary header:

| Offset | Type | Field | Description |
|---|---|---|---|
| `0x00` | `char[4]` | `Magic` | File signature (`MBPB`) |
| `0x04` | `uint16` | `Width` | Video width in pixels (e.g., 640) |
| `0x06` | `uint16` | `Height` | Video height in pixels (e.g., 480) |
| `0x08` | `uint16` | `FPS` | Target framerate (e.g., 25) |
| `0x0A` | `uint32` | `Num Frames` | Total frame count |
| `0x0E` | `uint32` | `Raw Size` | Size of a single packed frame buffer in bytes |
| `0x12` | `char[14]`| `Padding` | Reserved zero-padding bytes |

---

## ⌨️ Controls

- `SPACE` — Pause / Resume playback
- `LEFT ARROW` — Seek 5 seconds backward
- `RIGHT ARROW` — Seek 5 seconds forward

---

## 🖥️ System Requirements & Architecture

- **OS:** Linux or Windows (x86 / x86_64)
- **Compiler:** GCC / Clang
- **Assembler:** NASM / YASM
- **Libraries:** SDL3 development headers
- **Python Pipeline:** Python 3.x + `ffmpeg`

*Note: The bit-unpacking engine is written in 32-bit x86 Assembly (`cdecl` calling convention) for maximum low-level performance on x86 architectures.*

---

## 🧰 Building & Running

### 1. Assemble & Compile (Linux Example)

~~~bash
# Assemble x86 decoding engine
nasm -f elf32 engine.asm -o engine.o

# Compile C player and link SDL3
gcc player.c engine.o -o player $(pkg-config --cflags --libs sdl3)
~~~

### 2. Convert Video & Play

~~~bash
# Convert any input video to 640x480 25fps .ubv format
python3 konwert.py input.mp4

# Run the player
./player input.ubv
~~~

