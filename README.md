# lvgl_swipe — Display + Touch Test for Waveshare 1.69" LCD

A Zephyr/LVGL test application that draws a green "Swipe to test!" home screen on the [Waveshare 1.69" Touch LCD Module](https://www.waveshare.com/wiki/1.69inch_Touch_LCD_Module#Hardware_Connection_3) and switches screens based on swipe gestures from the CST816S touch controller.

This project supports two boards:

- **nRF5340 DK** — original target, working
- **nRF54LM20 DK** — current target, working as of 2026-04-26

Build directories are kept separate per board (`build_53dk/`, `build_54lm20/`).

## Hardware

### Display

[Waveshare 1.69" Touch LCD Module](https://www.waveshare.com/wiki/1.69inch_Touch_LCD_Module) — 240×280, ST7789V controller, CST816S capacitive touch.

The Waveshare display board has two pin headers, H1 and H2:
- **H1** routes signals through onboard level shifters (3.3V MCU side ↔ configurable display side voltage).
- **H2** is direct (no level shifters) for cases where MCU and display run at the same voltage.

For both DKs below, jumpers are set so the display board's VDDIO and VDD_TP are sourced from the DK's 3.3V rail and signals on H1 are passed through at 3.3V both sides:

| Jumper | Setting |
|---|---|
| J1 | 1.2 + 1.3 (Ext → display side) |
| J2 | 2.1 + 2.2 |
| J3 | 3.1 + 3.2 |
| J4 | 4.1 + 4.2 |
| J1.1 ("Ext") | wired to a 3.3V VDD pin on the DK |

H1.10 (VCC) is wired to a DK 3.3V VDD pin to power the MCU side of the level shifters.

## nRF5340 DK pin connections

Pins on H1 of the display board → nRF5340 DK GPIOs:

| H1 Pin | Module Pin | Function | nRF5340 Pin |
|--------|------------|----------|-------------|
| H1.1 | LCD_DC | Display D/C | P1.11 |
| H1.2 | LCD_CS | Display CS | P0.11 |
| H1.3 | LCD_CLK | SPI SCK | P0.06 |
| H1.4 | LCD_DIN | SPI MOSI | P0.25 |
| H1.5 | LCD_RST | Display Reset | P1.10 |
| H1.6 | TP_SCL | I2C SCL | P1.03 |
| H1.7 | TP_SDA | I2C SDA | P1.02 |
| H1.8 | TP_RST | Touch Reset | P0.20 |
| H1.9 | TP_IRQ | Touch IRQ | P1.01 |
| H1.10 | VCC | 3.3V | VDD |
| H1.11 | GND | GND | GND |
| H1.12 | LCD_BL | Backlight | VDD (hardwired) |

The 5340 has flexible peripheral pin routing — most peripherals can map to most GPIOs. Backlight is hardwired to VDD (always on at full brightness).

## nRF54LM20 DK pin connections

The 54L family has strict peripheral-to-port routing rules that make the 5340 pin choices invalid. After porting:

| H1 Pin | Module Pin | Function | nRF54LM20 Pin |
|--------|------------|----------|---------------|
| H1.1 | LCD_DC | Display D/C | P1.11 |
| H1.2 | LCD_CS | Display CS | P1.15 |
| H1.3 | LCD_CLK | SPI SCK | P1.13 |
| H1.4 | LCD_DIN | SPI MOSI | P1.14 |
| H1.5 | LCD_RST | Display Reset | P1.10 |
| H1.6 | TP_SCL | I2C SCL | P0.03 |
| H1.7 | TP_SDA | I2C SDA | P0.04 |
| H1.8 | TP_RST | Touch Reset | P1.05 |
| H1.9 | TP_IRQ | Touch IRQ | P1.04 |
| H1.10 | VCC | 3.3V | VDD |
| H1.11 | GND | GND | GND |
| H1.12 | LCD_BL | Backlight | P0.07 (PWM) |

**Peripheral instances:**
- Touch I2C → `i2c30` (TWIM30, LP domain)
- Display SPI → `spi21` (SPIM21, Peripheral domain)
- Backlight → `pwm20` driving a `pwm-leds` node, controlled via Zephyr's LED API

## nRF54L pin planning notes

These rules took a long debugging session to nail down. Documented here so future me doesn't have to relearn them:

- **TWIM/SPIM/UARTE instance numbering reflects the power domain**, and each domain has a dedicated GPIO port:
  - `*30` (LP domain) → must use **P0** pins
  - `*20`, `*21`, `*22` (Peripheral domain) → must use **P1** pins
  - `*00` (MCU domain, includes QSPI/HSSPI) → must use **P2** pins
- Trying to route, e.g., TWIM22 to a P0 pin or SPIM21 to a P2 pin will appear to initialize fine in Zephyr but won't actually drive the pins.
- The nRF54LM20 DK has many GPIOs pre-allocated by the board itself:
  - **P0.05–P0.09**: Button 3 (P0.05), UART0 TXD/RXD/RTS/CTS (P0.06–P0.09). Avoid for application use unless you also disconnect the debugger UART via the Board Configurator.
  - **P1.08, P1.09, P1.26**: Buttons 0/1/2
  - **P1.16–P1.19**: UART1 (TXD/RXD/RTS/CTS)
  - **P1.20, P1.21**: 32.768 kHz crystal (free via SB1/SB2 cut, SB3/SB4 short)
  - **P1.22, P1.25, P1.27, P1.28, P1.31**: LEDs 0–3
  - **P2.00–P2.05**: External flash (QSPI). All of P2 is essentially off-limits without disabling the onboard flash.
  - **P2.06–P2.10**: Trace
- **NFC pins (P1.01 = NFC1, P1.02 = NFC2)** are configured for the NFC antenna by default. To use them as GPIO/I2C/etc:
  1. Add `&uicr { nfct-pins-as-gpios; };` to the overlay (this writes UICR so the chip clears `NFCT->PADCONFIG` at boot). Note: `&nfct { status = "disabled"; };` alone is **not** sufficient — that only unbinds the Zephyr driver.
  2. On the DK, move 0Ω resistors from R33/R34 to R3/R4 to break the NFC antenna routing and connect the pins to the GPIO header.

For full DK pin map see the [nRF54LM20 DK Hardware User Guide](https://docs.nordicsemi.com/bundle/ug_nrf54lm20_dk/page/UG/nRF54LM20_DK/hw_desription/connector_if.html).

## Building

```bash
# nRF5340 DK
west build --build-dir build_53dk -b nrf5340dk/nrf5340/cpuapp --pristine

# nRF54LM20 DK
west build --build-dir build_54lm20 -b nrf54lm20dk/nrf54lm20a/cpuapp --pristine --sysbuild
```

Then flash:

```bash
west flash --build-dir build_54lm20
```

## Configuration

### prj.conf

Notable settings for the 54LM20 build:

- `CONFIG_PM_DEVICE` and `CONFIG_PM_DEVICE_RUNTIME` are **disabled**. With runtime PM enabled on this SDK version, the ST7789V driver wasn't being woken at init time and the display stayed black. Reintroduce PM later once basic functionality is solid.
- `CONFIG_LED=y` and `CONFIG_LED_PWM=y` enable the LED API used to drive the PWM backlight.
- `CONFIG_I2C_SHELL=y`, `CONFIG_GPIO_SHELL=y`, `CONFIG_DEVICE_SHELL=y` provide the diagnostic shell commands used during bring-up (`i2c scan i2c30`, `gpio set`, `device list`).

### Debugging tips

If the display is black or touch fails, the shell commands worth running in order:

```
device list
```
Confirms which devices initialized. Look for `cst816s@15 (READY)` — if it shows `DISABLED`, the I2C init failed.

```
i2c scan i2c30
```
A working I2C bus with the touch chip connected returns `15` in the address grid. Empty grid = I2C not reaching the chip.

```
gpio conf gpio@10a000 7 o
gpio set gpio@10a000 7 1
```
Manually drives the backlight pin high to verify the LED circuit independent of PWM.

## CST816S driver debugging notes (legacy, from 5340 work)

When debugging the touch controller, these modifications to Zephyr's [cst816s driver](https://github.com/zephyrproject-rtos/zephyr/blob/main/drivers/input/input_cst816s.c) were useful:

Force debug-level logging:

```c
//LOG_MODULE_REGISTER(cst816s, CONFIG_INPUT_LOG_LEVEL);
LOG_MODULE_REGISTER(cst816s, LOG_LEVEL_DBG);
```

Some CST816 variants report a different chip ID. If the driver fails with `failed reading chip id` but I2C scan finds the device, try changing:

```c
//#define CST816S_CHIP_ID 0xB4
#define CST816S_CHIP_ID 0xB5
```

Add logging inside `cst816s_process()` to see touch coordinates and event types in real time:

```c
if (pressed) {
    LOG_DBG("pressed event: %d, row: %d, col: %d", event, row, col);
    input_report_abs(dev, INPUT_ABS_X, col, false, K_FOREVER);
    input_report_abs(dev, INPUT_ABS_Y, row, false, K_FOREVER);
    input_report_key(dev, INPUT_BTN_TOUCH, 1, true, K_FOREVER);
} else {
    LOG_DBG("not pressed event: %d, row: %d, col: %d", event, row, col);
    input_report_key(dev, INPUT_BTN_TOUCH, 0, true, K_FOREVER);
}
```

These are not required for the 54LM20 build to work — the stock driver paired with the correct pin/instance configuration is sufficient.