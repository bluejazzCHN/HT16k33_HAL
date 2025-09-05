#include "ht16k33.h"
#include <math.h>

// 段码映射表
static const uint8_t segment_map[] = { 0x3F, // 0
		0x06, // 1
		0x5B, // 2
		0x4F, // 3
		0x66, // 4
		0x6D, // 5
		0x7D, // 6
		0x07, // 7
		0x7F, // 8
		0x6F, // 9
		0x77, // A
		0x7C, // B
		0x39, // C
		0x5E, // D
		0x79, // E
		0x71, // F
		0x00, // 空
		0x40, // 减号
		0x61, // TOP_C
		0x63  // 度符号
		};

// 初始化HT16K33
HAL_StatusTypeDef HT16K33_Init(HT16K33_HandleTypeDef *hdev,
		I2C_HandleTypeDef *hi2c, uint8_t digits) {
	HAL_StatusTypeDef status;

	// 设置设备参数
	hdev->hi2c = hi2c;
	hdev->i2c_addr = HT16K33_I2C_ADDR << 1; // 左移1位，因为HAL库需要7位地址
	hdev->digits = (digits > 8) ? 8 : digits;
	hdev->brightness = 15; // 默认最大亮度
	hdev->mode = DISPLAY_MODE_INTEGER;

	// 开启振荡器
	uint8_t cmd = HT16K33_OSCILLATOR_ON;
	status = HAL_I2C_Master_Transmit(hdev->hi2c, hdev->i2c_addr, &cmd, 1, 100);
	if (status != HAL_OK)
		return status;

	// 开启显示，不闪烁
	cmd = HT16K33_DISPLAY_ON;
	status = HAL_I2C_Master_Transmit(hdev->hi2c, hdev->i2c_addr, &cmd, 1, 100);
	if (status != HAL_OK)
		return status;

	// 设置亮度
	status = HT16K33_SetBrightness(hdev, hdev->brightness);
	if (status != HAL_OK)
		return status;

	// 清空显示
	return HT16K33_Clear(hdev);
}

// 清空显示
HAL_StatusTypeDef HT16K33_Clear(HT16K33_HandleTypeDef *hdev) {
	for (uint8_t i = 0; i < hdev->digits; i++) {
		hdev->buffer[i] = 0x00;
	}
	return HT16K33_WriteDisplay(hdev);
}

// 设置亮度
HAL_StatusTypeDef HT16K33_SetBrightness(HT16K33_HandleTypeDef *hdev,
		uint8_t brightness) {
	if (brightness > 15)
		brightness = 15;
	hdev->brightness = brightness;

	uint8_t cmd = HT16K33_BRIGHTNESS | brightness;
	return HAL_I2C_Master_Transmit(hdev->hi2c, hdev->i2c_addr, &cmd, 1, 100);
}

// 设置闪烁模式
HAL_StatusTypeDef HT16K33_SetBlink(HT16K33_HandleTypeDef *hdev, uint8_t mode) {
	if (mode > 2)
		mode = 0; // HT16K33通常支持0-2三种闪烁模式，0为不闪烁

	uint8_t cmd[2];
	// 首先需要确保振荡器开启（如果之前未初始化）
	cmd[0] = HT16K33_OSCILLATOR_ON;
	HAL_I2C_Master_Transmit(hdev->hi2c, hdev->i2c_addr, cmd, 1, 10);

	// 设置显示开关和闪烁模式
	// 格式: 1000 0ABb (A:闪烁开关, Bb:闪烁频率)
	cmd[0] = HT16K33_DISPLAY_ON | ((mode > 0) ? 0x01 : 0x00); // 确保最高位是1（命令标识），并设置开关位
	if (mode > 0) {
		cmd[0] |= ((mode - 1) << 1); // 设置闪烁频率位
	}

	return HAL_I2C_Master_Transmit(hdev->hi2c, hdev->i2c_addr, cmd, 1, 10);
}

// 写入显示数据
HAL_StatusTypeDef HT16K33_WriteDisplay(HT16K33_HandleTypeDef *hdev) {
	uint8_t data[17];
	data[0] = 0x00; // 起始地址

	// 填充显示数据
	for (uint8_t i = 0; i < 8; i++) {
		data[2 * i + 1] = hdev->buffer[i];
		data[2 * i + 2] = 0x00; // 第二个字节通常不使用
	}

	return HAL_I2C_Master_Transmit(hdev->hi2c, hdev->i2c_addr, data, 17, 100);
}

// 显示整数
HAL_StatusTypeDef HT16K33_DisplayInteger(HT16K33_HandleTypeDef *hdev,
		int32_t number) {
	return HT16K33_DisplayNumber(hdev, (double) number, 0, SEGMENT_EMPTY);
}

// 显示浮点数
HAL_StatusTypeDef HT16K33_DisplayFloat(HT16K33_HandleTypeDef *hdev,
		float number, uint8_t decimal_places) {
	return HT16K33_DisplayNumber(hdev, (double) number, decimal_places,
			SEGMENT_EMPTY);
}

// 显示数字（支持小数和符号）
HAL_StatusTypeDef HT16K33_DisplayNumber(HT16K33_HandleTypeDef *hdev,
		double number, uint8_t decimal_places, uint8_t symbol) {
	// 限制小数位数
	if (decimal_places > hdev->digits - 1) {
		decimal_places = hdev->digits - 1;
	}

	// 如果有符号，再减少一位可用位数
	uint8_t symbol_space = (symbol != SEGMENT_EMPTY) ? 1 : 0;
	if (decimal_places + symbol_space > hdev->digits - 1) {
		decimal_places = hdev->digits - 1 - symbol_space;
	}

	// 处理负数
	bool is_negative = false;
	if (number < 0) {
		is_negative = true;
		number = -number;
	}

	// 计算整数部分和小数部分
	int32_t integer_part = (int32_t) number;
	double fractional_part = number - integer_part;

	// 计算小数部分对应的整数
	for (uint8_t i = 0; i < decimal_places; i++) {
		fractional_part *= 10;
	}
	int32_t fractional_int = (int32_t) (fractional_part + 0.5); // 四舍五入

	// 计算整数部分的位数
	uint8_t integer_digits = 0;
	int32_t temp = integer_part;
	do {
		integer_digits++;
		temp /= 10;
	} while (temp > 0);

	if (integer_part == 0)
		integer_digits = 1; // 处理0的情况

	// 计算总位数（包括可能的负号、小数点和符号）
	uint8_t total_digits = integer_digits + decimal_places;
	if (is_negative)
		total_digits++; // 负号占一位
	if (symbol != SEGMENT_EMPTY)
		total_digits++; // 符号占一位

	// 检查是否超出显示范围
	if (total_digits > hdev->digits) {
		// 显示错误信息 "----"
		for (uint8_t i = 0; i < hdev->digits; i++) {
			HT16K33_SetDigit(hdev, i, SEGMENT_MINUS, false);
		}
		return HT16K33_WriteDisplay(hdev);
	}

	// 清空显示
	HT16K33_Clear(hdev);

	// 计算起始位置（右对齐）
	uint8_t start_pos = hdev->digits - total_digits;

	// 当前位置
	uint8_t current_pos = start_pos;

	// 显示负号（如果需要）
	if (is_negative) {
		HT16K33_SetDigit(hdev, current_pos, SEGMENT_MINUS, false);
		current_pos++;
	}

	// 显示整数部分
	if (integer_part == 0) {
		HT16K33_SetDigit(hdev, current_pos, 0, false);
		current_pos++;
	} else {
		// 提取整数部分的每一位数字
		uint8_t int_digits[10] = { 0 }; // 最多10位数字
		uint8_t int_count = 0;
		int32_t temp = integer_part;

		while (temp > 0) {
			int_digits[int_count++] = temp % 10;
			temp /= 10;
		}

		// 按正确顺序显示整数部分（从最高位到最低位）
		for (int8_t i = int_count - 1; i >= 0; i--) {
			HT16K33_SetDigit(hdev, current_pos, int_digits[i], false);
			current_pos++;
		}
	}

	// 显示小数点（如果需要）
	if (decimal_places > 0) {
		// 小数点显示在整数部分最后一位数字上
		uint8_t dot_pos = current_pos - 1;
		hdev->buffer[dot_pos] |= 0x80; // 设置小数点
	}

	// 显示小数部分
	if (decimal_places > 0) {
		// 提取小数部分的每一位数字
		uint8_t frac_digits[10] = { 0 }; // 最多10位小数
		uint8_t frac_count = 0;
		int32_t temp = fractional_int;

		// 可能需要补零
		for (uint8_t i = 0; i < decimal_places; i++) {
			frac_digits[frac_count++] = temp % 10;
			temp /= 10;
		}

		// 按正确顺序显示小数部分（从最高位到最低位）
		for (int8_t i = frac_count - 1; i >= 0; i--) {
			HT16K33_SetDigit(hdev, current_pos, frac_digits[i], false);
			current_pos++;
		}
	}

	// 显示符号（如果需要）
	if (symbol != SEGMENT_EMPTY) {
		HT16K33_SetDigit(hdev, current_pos, symbol, false);
	}

	return HT16K33_WriteDisplay(hdev);
}

// 显示温度值（自动处理度数字符位置）
HAL_StatusTypeDef HT16K33_DisplayTemperature(HT16K33_HandleTypeDef *hdev,
		float temperature, uint8_t decimal_places) {
	// 确保小数位数合理
	if (decimal_places > 2)
		decimal_places = 2;

	// 计算数字部分需要的位数
	int32_t integer_part = (int32_t) temperature;
	uint8_t integer_digits = 1; // 至少一位
	int32_t temp = integer_part;

	while (temp /= 10) {
		integer_digits++;
	}

	// 处理负数
	if (temperature < 0)
		integer_digits++;

	// 计算总位数（数字 + 小数点 + 度数字符）
	uint8_t total_digits = integer_digits + decimal_places;
	if (decimal_places > 0)
		total_digits++; // 小数点占一位
	total_digits++; // 度数字符占一位

	// 检查是否超出显示范围
	if (total_digits > hdev->digits) {
		// 显示错误信息 "----"
		for (uint8_t i = 0; i < hdev->digits; i++) {
			HT16K33_SetDigit(hdev, i, SEGMENT_MINUS, false);
		}
		return HT16K33_WriteDisplay(hdev);
	}

	// 显示温度数字部分
	HAL_StatusTypeDef status = HT16K33_DisplayNumber(hdev, temperature,
			decimal_places, SEGMENT_EMPTY);
	if (status != HAL_OK)
		return status;

	// 在正确位置显示度数字符（最右侧）
	uint8_t degree_pos = hdev->digits - 1;
	return HT16K33_SetDigit(hdev, degree_pos, SEGMENT_DEGREE, false);
}

// 设置单个数字
HAL_StatusTypeDef HT16K33_SetDigit(HT16K33_HandleTypeDef *hdev,
		uint8_t position, uint8_t digit, bool dot) {
	if (position >= hdev->digits)
		return HAL_ERROR;
	if (digit > 19)
		digit = SEGMENT_EMPTY; // 超出范围显示为空

	uint8_t segments = segment_map[digit];
	if (dot) {
		segments |= 0x80; // 设置小数点
	}

	hdev->buffer[position] = segments;
	return HAL_OK;
}

// 设置原始段码
HAL_StatusTypeDef HT16K33_SetSegments(HT16K33_HandleTypeDef *hdev,
		uint8_t position, uint8_t segments) {
	if (position >= hdev->digits)
		return HAL_ERROR;

	hdev->buffer[position] = segments;
	return HAL_OK;
}
