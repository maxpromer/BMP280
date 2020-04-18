#ifndef __BMP280_CPP__
#define __BMP280_CPP__

#include "BMP280.h"
#include "Adafruit_BMP280.h"

I2CDev *_i2c_dev;

BMP280::BMP280(int bus_ch, int dev_addr) {
	channel = bus_ch;
	address = dev_addr;
	polling_ms = 500;
}

void BMP280::init(void) {
	// clear initialized flag
	initialized = false;
	
	// Debug
	esp_log_level_set("*", ESP_LOG_INFO);
	
	// Start initialized
	state = s_detect;
}

int BMP280::prop_count(void) {
	// not supported
	return 0;
}

bool BMP280::prop_name(int index, char *name) {
	// not supported
	return false;
}

bool BMP280::prop_unit(int index, char *unit) {
	// not supported
	return false;
}

bool BMP280::prop_attr(int index, char *attr) {
	// not supported
	return false;
}

bool BMP280::prop_read(int index, char *value) {
	// not supported
	return false;
}

bool BMP280::prop_write(int index, char *value) {
	// not supported
	return false;
}
// --------------------------------------

void BMP280::process(Driver *drv) {
	I2CDev *i2c = (I2CDev *)drv;
	switch (state) {
		case s_detect:
			// detect i2c device
			if (i2c->detect(channel, address) == ESP_OK) {
				uint8_t buff = 0xD0; // Chip ID
				if (i2c->read(channel, address, &buff, 1, &buff, 1) == ESP_OK) {
					if (buff == 0x58) {
						_i2c_dev = i2c;
						
						state = s_init;
					} else {
						state = s_error;
					}
				} else {
					state = s_error;
				}
			} else {
				state = s_error;
			}
			break;
			
		case s_init: {
			if (bmp.begin(address)) {
				/* Default settings from datasheet. */
				bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
								Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
								Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
								Adafruit_BMP280::FILTER_X16,      /* Filtering. */
								Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
				
				// clear error flag
				error = false;
				// set initialized flag
				initialized = true;

				state = s_read;	
			} else {
				state = s_error;
			}
			break;
		}
		
		case s_read: {
			if (is_tickcnt_elapsed(tickcnt, polling_ms)) {
				tickcnt = get_tickcnt();
				
				uint8_t buff = 0xD0; // Chip ID
				if (i2c->read(channel, address, &buff, 1, &buff, 1) == ESP_OK) {
					if (buff == 0x58) {
						temperature = bmp.readTemperature();
						pressure = bmp.readPressure();
					} else {
						state = s_error;
					}
				} else {
					state = s_error;
				}
			}
			break;
		}
		
		case s_wait:
			if (error) {
				// wait polling_ms timeout
				if (is_tickcnt_elapsed(tickcnt, polling_ms)) {
					state = s_detect;
				}
			}
			break;

		case s_error:
			temperature = 0.0;
			pressure = 0.0;
		
			// set error flag
			error = true;
			// clear initialized flag
			initialized = false;
			// get current tickcnt
			tickcnt = get_tickcnt();
			// goto wait and retry with detect state
			state = s_wait;
			break;

	}
}

// Method
double BMP280::readTemperature() {
	return temperature;
}

double BMP280::readPressure() {
	return pressure;
}

#endif