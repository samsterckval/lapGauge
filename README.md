# ESP32-S3-Touch-LCD-1.85 custom firmware skeleton

**Work in progress.** This is an early-stage skeleton, not a finished
firmware - expect missing features, rough edges, and pins/config that
haven't been validated against real hardware yet (see "Things I couldn't
verify" below). It currently gets a QSPI display bring-up working with a
QMI8658 accelerometer readout rendered as a ball on screen; touch, audio,
and RTC are not wired up yet.

Minimal ESP-IDF project: I2C + TCA9554 expander bring-up, QSPI ST77916 panel
init, LVGL 9 wired in via esp_lvgl_port, and a QMI8658 accelerometer readout
(`main/imu/`) driving a ball that moves/resizes with acceleration. No touch,
audio, or RTC yet - that's the next layer to build once you've confirmed the
display and IMU paths work.

### IMU readout (`main/imu/`)

- `IAccelerometerSensor` is the driver interface; `Qmi8658Sensor` is the only
  implementation today. Swap in a different chip by writing a new class
  against that interface - nothing else in `main/imu/` needs to change.
- `AccelerometerData` holds the latest raw and EMA-filtered sample and knows
  nothing about the sensor driver or the display.
- `BallView` renders a sample as a ball and knows nothing about where the
  data came from.
- `ImuService` owns the polling task that ties the three together.
- Tunables (poll rate, EMA window, max-g range, ball sizing) all live in
  `main/imu/imu_config.hpp`.

## Build

Run these from the repo root (there's no `esp32-s3-lcd-185/` subdirectory -
the project files live at the top level).

```bash
. ~/esp/esp-idf/export.sh        # every new shell
idf.py set-target esp32s3        # first run: this is an S3 board, not a
                                  # plain esp32 - skipping this builds the
                                  # wrong target and fails with bogus
                                  # "unknown opcode" assembler errors
idf.py reconfigure               # pulls the managed components
                                  # (esp_lcd_st77916, esp_lcd_touch_cst816s,
                                  # lvgl, esp_lvgl_port) into managed_components/
idf.py build
```

If you ever change `main/idf_component.yml` (bump a dependency version) on
an existing `build/` directory, prefer `rm -rf build && idf.py set-target
esp32s3` over a plain `idf.py reconfigure` - an incremental reconfigure has
been observed to leave the CMake cache pointing at the wrong (plain esp32)
compiler, producing the same bogus assembler errors.

## Flash + monitor

```bash
idf.py -p /dev/ttyACM0 flash monitor   # Linux, USB-CDC (adjust port)
```

If the port doesn't enumerate: hold BOOT, tap RESET, release RESET, then
release BOOT - that forces download mode manually. Once flashing succeeds
once, subsequent flashes usually auto-trigger download mode fine.

Exit the serial monitor with `Ctrl+]`.

## Things I couldn't verify without your actual hardware in hand

- **`esp_lcd_st77916` API surface**: vendor component APIs shift between
  versions and `main/idf_component.yml`'s pin will pull whatever the latest
  matching version is at the time you run `idf.py reconfigure`. If it
  doesn't compile, `managed_components/espressif__esp_lcd_st77916/README.md`
  after the first fetch is the ground truth, not this file.
- **LVGL version**: pinned to `lvgl/lvgl: "^9.2.2"` with
  `espressif/esp_lvgl_port: "^2.9.0"` (the port component's own manifest
  requires a matching LVGL major version, so bump both together if you ever
  touch this).
- **Color order / gamma**: `rgb_ele_order` is set to RGB as a starting guess.
  If colors come out swapped (blue where you expect red), flip it to
  `LCD_RGB_ELEMENT_ORDER_BGR` - this is a known point of confusion on this
  exact panel per community reports.
- **EXIO bit numbering**: I've mapped LCD_RST/TP_RST/SD_CS to EXIO2/1/3
  straight from the wiki's internal hardware connection table. Worth
  confirming against the schematic PDF once, since "EXIO2" in Waveshare's
  docs isn't always literally bit 2 on every board revision.
- **QMI8658 I2C address**: set to `0x6B` (`main/pins.h`), which matches
  Waveshare's published address for this board family, but the schematic's
  pin table doesn't show the SA0 strap directly - if the chip never ACKs on
  the bus, try `0x6A`.
- **QMI8658 axis orientation**: `main/imu/ball_view.cpp` maps accel X/Y
  directly to screen X/Y and accel Z to ball size, with no assumption about
  mounting orientation. Depending on how the chip is oriented on the PCB,
  you may want to swap or negate an axis so "tilt right" actually moves the
  ball right - that's a one-line change in `BallView::update`.

## About this project

Most of this codebase and its setup (environment config, dependency
wrangling, bug fixes, docs) was produced with the help of an LLM
(Claude), with a human reviewing and directing the work. Treat that as
useful context when judging how much to trust any given part of it.

## Schematic / reference

- [Schematic PDF](https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85/ESP32-S3-Touch-LCD-1.85.pdf)
- [Waveshare wiki page](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85)

## License

Licensed under the GNU General Public License v3.0 (GPLv3) - see
[LICENSE](LICENSE) for the full text.
