Blockly.Blocks['bmp280_read_temperature'] = {
	init: function() {
		this.jsonInit({
			"message0": Blockly.Msg.BMP280_READ_TEMPERATURE_MESSAGE,
			"args0": [{
				"type": "field_dropdown",
				"name": "addr",
				"options": [
					[ "0x76", "0x76" ],
					[ "0x77", "0x77" ]
				]
			}],
			"output": "Number",
			"colour": 135,
			"tooltip": Blockly.Msg.BMP280_READ_TEMPERATURE_TOOLTIP,
			"helpUrl": ""
		});
	}
};

Blockly.Blocks['bmp280_read_pressure'] = {
	init: function() {
		this.jsonInit({
			"message0": Blockly.Msg.BMP280_READ_PRESSURE_MESSAGE,
			"args0": [{
				"type": "field_dropdown",
				"name": "addr",
				"options": [
					[ "0x76", "0x76" ],
					[ "0x77", "0x77" ]
				]
			}],
			"output": "Number",
			"colour": 135,
			"tooltip": Blockly.Msg.BMP280_READ_PRESSURE_TOOLTIP,
			"helpUrl": ""
		});
	}
};

Blockly.Blocks['bmp280_read_altitude'] = {
	init: function() {
		this.jsonInit({
			"type": "bmp280_read_altitude",
			"message0": Blockly.Msg.BMP280_READ_ALTITUDE_MESSAGE,
			"args0": [{
				"type": "field_dropdown",
				"name": "addr",
				"options": [
					[ "0x76", "0x76" ],
					[ "0x77", "0x77" ]
				]
			}, {
				"type": "field_number",
				"name": "seaLevelhPa",
				"value": 1013.25,
				"min": 0
			}],
			"inputsInline": true,
			"output": "Number",
			"colour": 135,
			"tooltip": Blockly.Msg.BMP280_READ_ALTITUDE_TOOLTIP,
			"helpUrl": ""
		});
	}
};
