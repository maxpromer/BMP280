#ifndef __BMP280_CLASS_H__
#define __BMP280_CLASS_H__

#include "driver.h"
#include "device.h"
#include "i2c-dev.h"
#include "kidbright32.h"
#include "bmp280.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"

static I2CDev *_i2c_dev;

static void delay_ms(uint32_t period_ms);
static int8_t i2c_reg_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);
static int8_t i2c_reg_read(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);

class BMP280 : public Device {
	private:		
		enum {
			s_detect,
			s_init,
			s_read,
			s_wait,
			s_error
		} state;
		TickType_t tickcnt, polling_tickcnt;

		double temperature = 0.0, pressure = 0.0;
		
		struct bmp280_dev bmp;
		struct bmp280_config conf;

	public:
		// constructor
		BMP280(int bus_ch, int dev_addr) ;
		
		// override
		void init(void);
		void process(Driver *drv);
		int prop_count(void);
		bool prop_name(int index, char *name);
		bool prop_unit(int index, char *unit);
		bool prop_attr(int index, char *attr);
		bool prop_read(int index, char *value);
		bool prop_write(int index, char *value);
		
		// method
		double readTemperature() ;
		double readPressure() ;

		
};

#endif