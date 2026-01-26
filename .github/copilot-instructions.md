# AI Coding Agent Instructions for IoT ESP32 Projects

## Project Overview
This is an IoT lab course repository (TIN4024.003) with 16+ student teams developing embedded systems projects using **ESP32 microcontrollers**. Each team works in independent `TEAM_XX/` folders with individual member subdirectories. Core projects include LED blink demonstrations and traffic light simulations.

## Architecture & Key Components

### Project Structure
- **TEAM_XX/**: Isolated team working directories containing member-specific subfolders
- **Member folders**: Individual student projects with `platformio.ini`, `diagram.json`, `wokwi.toml`, `src/`, `include/`, `lib/`, `test/` directories
- **_Documents/**: Shared learning materials covering Arduino fundamentals, blocking vs non-blocking patterns, and ESP32 GPIO programming

### Build & Simulation Pipeline
1. **PlatformIO** (`platformio.ini`): Manages compilation for ESP32Dev board with Arduino framework
2. **Wokwi** (`diagram.json`, `wokwi.toml`): Online simulator for circuit design and testing before hardware deployment
3. **Firmware Path**: `.pio/build/esp32dev/firmware.bin` (auto-referenced by wokwi.toml)

## Critical Patterns & Conventions

### GPIO Pin Definitions
Use `#define` for pin mappings - common patterns:
```cpp
#define RED_LED 17
#define GREEN_LED 16
#define YELLOW_LED 25
```

### Timing Strategies (Non-Blocking Preferred)
- **Blocking** (simple, inefficient): `delay(1000)` - freezes entire program
- **Non-Blocking** (recommended): Use `millis()` + timer variable pattern from `utils.h`:
  ```cpp
  unsigned long previousMillis = 0;
  if (millis() - previousMillis >= interval) {
    previousMillis = millis();
    // Execute task
  }
  ```

### Code Reusability
- Utility functions in `lib/` or `include/utils.h` (example: `IsReady()`, `StringFormat()`)
- Common helpers: timing checks, string formatting for Serial output
- Reference: [TEAM_05/NguyenHuynhMinhTien/utils.h](TEAM_05/NguyenHuynhMinhTien/utils.h)

## Developer Workflows

### Required Tools Setup
```bash
# Install in VS Code:
1. PlatformIO Extension (for C/C++ compilation)
2. Wokwi Simulator Extension (for circuit testing)
```

### Build & Test Workflow
```bash
# Build project
pio run

# Upload to simulator
# Wokwi simulator runs via diagram.json configuration

# Run tests (when present)
pio test
```

### Debugging Approach
- Serial debug output: `Serial.begin(115200)` + `Serial.println()`
- Wokwi provides virtual oscilloscope and serial monitor for real-time observation
- Pin configuration visible in `diagram.json` connections array

## Integration Points & Dependencies

### External Tools (Non-Code)
- **Wokwi**: Circuit simulation platform (requires free account at wokwi.com)
- **GitHub**: Repository for team collaboration and version control

### Common Circuit Connections (from diagram.json)
- ESP32 pins → LED anodes (via `connections` array in diagram.json)
- GND pins → LED cathodes
- Serial TX/RX → Virtual Serial Monitor for debugging

## Project-Specific Conventions

1. **Language**: Vietnamese comments in code and documentation (team learning materials use Vietnamese)
2. **Traffic Light Projects**: Extend basic LED blink with timing sequences (4s green → 3s yellow → 5s red)
3. **File Naming**: 
   - `LED_Blink/` or `lED_BLINK/` - inconsistent capitalization (accept both patterns)
   - `src/main.cpp` is standard entry point
4. **Circuit Design**: Always include ESP32 DevKit C V4 board + LED components in diagram.json

## When Assisting Teams
- Confirm `platformio.ini` targets `esp32dev` board before suggesting code
- Validate GPIO pin definitions against `diagram.json` connections
- Prefer non-blocking timing patterns for any delay/loop logic
- Recommend code organization: reusable utilities in `utils.h`, logic in `main.cpp`
