/*
 * ht16k33.h
 *
 *  Created on: Sep 5, 2025
 *      Author: songj
 */

#ifndef BSP_LEDSEGMENT_HT16K33_H_
#define BSP_LEDSEGMENT_HT16K33_H_
#ifndef HT16K33_H
#define HT16K33_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

// HT16K33默认I2C地址
#define HT16K33_I2C_ADDR 0x70

// 命令定义
#define HT16K33_OSCILLATOR_ON 0x21
#define HT16K33_DISPLAY_ON 0x81
#define HT16K33_BRIGHTNESS 0xE0
#define HT16K33_BLINK 0x85

// 段码映射表索引
#define SEGMENT_0 0
#define SEGMENT_1 1
#define SEGMENT_2 2
#define SEGMENT_3 3
#define SEGMENT_4 4
#define SEGMENT_5 5
#define SEGMENT_6 6
#define SEGMENT_7 7
#define SEGMENT_8 8
#define SEGMENT_9 9
#define SEGMENT_A 10
#define SEGMENT_B 11
#define SEGMENT_C 12
#define SEGMENT_D 13
#define SEGMENT_E 14
#define SEGMENT_F 15
#define SEGMENT_EMPTY 16
#define SEGMENT_MINUS 17
#define SEGMENT_TOP_C 18
#define SEGMENT_DEGREE 19

// 显示模式
typedef enum {
    DISPLAY_MODE_INTEGER,    // 整数模式
    DISPLAY_MODE_FLOAT,      // 浮点数模式
    DISPLAY_MODE_RAW         // 原始段码模式
} display_mode_t;

// HT16K33设备结构体
typedef struct {
    I2C_HandleTypeDef *hi2c;    // I2C句柄
    uint8_t i2c_addr;           // I2C地址
    uint8_t brightness;         // 亮度(0-15)
    uint8_t digits;             // 数码管数量
    uint8_t buffer[8];          // 显示缓冲区
    display_mode_t mode;        // 显示模式
} HT16K33_HandleTypeDef;

// 函数声明
HAL_StatusTypeDef HT16K33_Init(HT16K33_HandleTypeDef *hdev, I2C_HandleTypeDef *hi2c, uint8_t digits);
HAL_StatusTypeDef HT16K33_Clear(HT16K33_HandleTypeDef *hdev);
HAL_StatusTypeDef HT16K33_SetBrightness(HT16K33_HandleTypeDef *hdev, uint8_t brightness);
HAL_StatusTypeDef HT16K33_WriteDisplay(HT16K33_HandleTypeDef *hdev);
HAL_StatusTypeDef HT16K33_DisplayInteger(HT16K33_HandleTypeDef *hdev, int32_t number);
HAL_StatusTypeDef HT16K33_DisplayFloat(HT16K33_HandleTypeDef *hdev, float number, uint8_t decimal_places);
HAL_StatusTypeDef HT16K33_DisplayNumber(HT16K33_HandleTypeDef *hdev, double number, uint8_t decimal_places, uint8_t symbol);
HAL_StatusTypeDef HT16K33_DisplayTemperature(HT16K33_HandleTypeDef *hdev, float temperature, uint8_t decimal_places);
HAL_StatusTypeDef HT16K33_SetDigit(HT16K33_HandleTypeDef *hdev, uint8_t position, uint8_t digit, bool dot);
HAL_StatusTypeDef HT16K33_SetSegments(HT16K33_HandleTypeDef *hdev, uint8_t position, uint8_t segments);
HAL_StatusTypeDef HT16K33_SetBlink(HT16K33_HandleTypeDef *hdev, uint8_t mode);

#endif // HT16K33_H
#endif /* BSP_LEDSEGMENT_HT16K33_H_ */
