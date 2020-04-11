#ifndef __BMP280_CLASS_CPP__
#define __BMP280_CLASS_CPP__

#include "bmp280_class.h"

BMP280::BMP280(int bus_ch, int dev_addr) {
	channel = bus_ch;
	address = dev_addr;
	polling_ms = 100;
}

void BMP280::init(void) {
	// clear initialized flag
	initialized = false;
	
	// Debug
	// esp_log_level_set("*", ESP_LOG_INFO);
	
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
			bmp.delay_ms = delay_ms;
			bmp.dev_id = address;
			bmp.intf = BMP280_I2C_INTF;
			bmp.read = i2c_reg_read;
			bmp.write = i2c_reg_write;
			
			if (bmp280_init(&bmp) == BMP280_OK) {
				/* Always read the current settings before writing, especially when
				 * all the configuration is not modified
				 */
				if (bmp280_get_config(&conf, &bmp) == BMP280_OK) {
					/* configuring the temperature oversampling, filter coefficient and output data rate */
					/* Overwrite the desired settings */
					conf.filter = BMP280_FILTER_COEFF_2;

					/* Temperature oversampling set at 4x */
					conf.os_temp = BMP280_OS_4X;

					/* Pressure over sampling none (disabling pressure measurement) */
					conf.os_pres = BMP280_OS_NONE;

					/* Setting the output data rate as 1HZ(1000ms) */
					// conf.odr = BMP280_ODR_1000_MS;
					conf.odr = BMP280_ODR_62_5_MS;
					
					if (bmp280_set_config(&conf, &bmp) == BMP280_OK) {
						/* Always set the power mode after setting the configuration */
						if (bmp280_set_power_mode(BMP280_NORMAL_MODE, &bmp) == BMP280_OK) {
							// clear error flag
							error = false;
							// set initialized flag
							initialized = true;
						
							state = s_read;
						} else {
							state = s_error;
						}
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
		}
		
		case s_read: {
			if (is_tickcnt_elapsed(tickcnt, polling_ms)) {
				tickcnt = get_tickcnt();
				
				struct bmp280_uncomp_data ucomp_data;
				/* Reading the raw data from sensor */
				if (bmp280_get_uncomp_data(&ucomp_data, &bmp) == BMP280_OK) {
					/* Getting the compensated temperature as floating point value */
					if (bmp280_get_comp_temp_double(&temperature, ucomp_data.uncomp_temp, &bmp) == BMP280_OK) {
						// ESP_LOGI("BMP280", "UT: %d, T: %f \r\n", ucomp_data.uncomp_temp, temperature);
					} else {
						// ESP_LOGI("BMP280", "GET data temperature from BMP280 error!");
						state = s_error;
					}
					
					/* Getting the compensated pressure as floating point value */
					if (bmp280_get_comp_pres_double(&pressure, ucomp_data.uncomp_press, &bmp) == BMP280_OK) {
						// ESP_LOGI("BMP280", "UP: %d, P: %f\r\n", ucomp_data.uncomp_press, pressure);
					} else {
						// ESP_LOGI("BMP280", "GET data pressure from BMP280 error!");
						state = s_error;
					}
				} else {
					// ESP_LOGI("BMP280", "GET data from BMP280 error!");
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

/*!
 *  @brief Function that creates a mandatory delay required in some of the APIs such as "bmg250_soft_reset",
 *      "bmg250_set_foc", "bmg250_perform_self_test"  and so on.
 *
 *  @param[in] period_ms  : the required wait time in milliseconds.
 *  @return void.
 *
 */
static void delay_ms(uint32_t period_ms)
{
    /* Implement the delay routine according to the target machine */
	vTaskDelay( period_ms / portTICK_PERIOD_MS ); 
}

/*!
 *  @brief Function for writing the sensor's registers through I2C bus.
 *
 *  @param[in] i2c_addr : sensor I2C address.
 *  @param[in] reg_addr : Register address.
 *  @param[in] reg_data : Pointer to the data buffer whose value is to be written.
 *  @param[in] length   : No of bytes to write.
 *
 *  @return Status of execution
 *  @retval 0 -> Success
 *  @retval >0 -> Failure Info
 *
 */
static int8_t i2c_reg_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length)
{
	uint8_t buff[length + 1];
	buff[0] = reg_addr;
	memcpy(&buff[1], reg_data, length);
    return _i2c_dev->write(0, i2c_addr, buff, length + 1) == ESP_OK ? 0 : 1;
}

/*!
 *  @brief Function for reading the sensor's registers through I2C bus.
 *
 *  @param[in] i2c_addr : Sensor I2C address.
 *  @param[in] reg_addr : Register address.
 *  @param[out] reg_data    : Pointer to the data buffer to store the read data.
 *  @param[in] length   : No of bytes to read.
 *
 *  @return Status of execution
 *  @retval 0 -> Success
 *  @retval >0 -> Failure Info
 *
 */
static int8_t i2c_reg_read(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length)
{
    return _i2c_dev->read(0, i2c_addr, &reg_addr, 1, reg_data, length) == ESP_OK ? 0 : 1;
}


#endif