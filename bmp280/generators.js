Blockly.JavaScript['bmp280_read_temperature'] = function(block) {
	var dropdown_addr = block.getFieldValue('addr');
	var code = 'DEV_I2C1.BMP280(0, ' + dropdown_addr + ').readTemperature()';
	return [code, Blockly.JavaScript.ORDER_NONE];
};

Blockly.JavaScript['bmp280_read_pressure'] = function(block) {
	var dropdown_addr = block.getFieldValue('addr');
	var code = 'DEV_I2C1.BMP280(0, ' + dropdown_addr + ').readPressure()';
	return [code, Blockly.JavaScript.ORDER_NONE];
};

