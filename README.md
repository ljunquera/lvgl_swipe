# Configuration and setup

## Debug cst816s driver
I made some changes to the [cst816s.c](https://github.com/zephyrproject-rtos/zephyr/blob/main/drivers/input/input_cst816s.c) driver to see what is happening and ensure the chip works.

Set the debug level of the driver:
```
//LOG_MODULE_REGISTER(cst816s, CONFIG_INPUT_LOG_LEVEL);
LOG_MODULE_REGISTER(cst816s, LOG_LEVEL_DBG);
```

Comment out the chip ID and use:

```
//#define CST816S_CHIP_ID 0xB4
#define CST816S_CHIP_ID 0xB5
```

I also added to debug statements to the file:
```
static int cst816s_process(const struct device *dev)
{
	const struct cst816s_config *cfg = dev->config;

	int r;
	uint8_t event;
	uint16_t row, col;
	bool pressed;
	uint16_t x;
	uint16_t y;

	r = i2c_burst_read_dt(&cfg->i2c, CST816S_REG_XPOS_H, (uint8_t *)&x, sizeof(x));
	if (r < 0) {
		LOG_ERR("Could not read x data");
		return r;
	}

	r = i2c_burst_read_dt(&cfg->i2c, CST816S_REG_YPOS_H, (uint8_t *)&y, sizeof(y));
	if (r < 0) {
		LOG_ERR("Could not read y data");
		return r;
	}
	col = sys_be16_to_cpu(x) & 0x0fff;
	row = sys_be16_to_cpu(y) & 0x0fff;

	event = (x & 0xff) >> CST816S_EVENT_BITS_POS;
	pressed = (event == EVENT_PRESS_DOWN) || (event == EVENT_CONTACT);

	if (pressed) {
		LOG_DBG("pressed event: %d, row: %d, col: %d", event, row, col);
		input_report_abs(dev, INPUT_ABS_X, col, false, K_FOREVER);
		input_report_abs(dev, INPUT_ABS_Y, row, false, K_FOREVER);
		input_report_key(dev, INPUT_BTN_TOUCH, 1, true, K_FOREVER);
	} else {
		LOG_DBG("not pressed event: %d, row: %d, col: %d", event, row, col);
		input_report_key(dev, INPUT_BTN_TOUCH, 0, true, K_FOREVER);
	}

	return r;
}
```

## Display
The display is [Waveshare - 1.69inch Touch LCD Module](https://www.waveshare.com/wiki/1.69inch_Touch_LCD_Module#Hardware_Connection_3) 

## Development kit connections
Display is connected to the [nRF5340 DK](https://www.nordicsemi.com/Products/Development-hardware/nRF5340-DK) using the following PIN settings:

| Pin No. | Module Pin | nRF5340 Pin |
|--------|------------|-------------|
| 1 | VCC | VDD (3.3) |
| 2 | GND | GND |
| 3 | LCD_DIN | P0.25 |
| 4 | LCD_CLK | P0.06 |
| 5 | LCD_CS | P0.11 |
| 6 | LCD_DC | P1.11 |
| 7 | LCS_RST | P1.10 |
| 8 | LCS_BL | VDD (3.3) |
| 9 | TP_SDA | P1.02 |
| 10 | TP_SCL | P1.03 |
| 11 | TP_RST | P0.20 |
| 12 | TP_IRQ | P1.01 |