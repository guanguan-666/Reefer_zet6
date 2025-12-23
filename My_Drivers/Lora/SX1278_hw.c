#include "SX1278_hw.h"
#include "main.h"              
#include "stm32f1xx_hal.h"     
#include <string.h>            

// 声明外部 SPI 句柄
extern SPI_HandleTypeDef hspi1; 

// --- 硬件初始化 ---
void SX1278_hw_init(SX1278_hw_t *hw) {
    // 1. 强制开启时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_SPI1_CLK_ENABLE(); 

    // 2. 初始化 GPIO
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // NSS (片选)
    GPIO_InitStruct.Pin = LORA_NSS_Pin; 
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LORA_NSS_GPIO_Port, &GPIO_InitStruct);
    
    // RESET (复位)
    GPIO_InitStruct.Pin = LORA_RST_Pin;
    HAL_GPIO_Init(LORA_RST_GPIO_Port, &GPIO_InitStruct);

    // DIO0 (中断/轮询)
    GPIO_InitStruct.Pin = LORA_DIO0_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN; 
    HAL_GPIO_Init(LORA_DIO0_GPIO_Port, &GPIO_InitStruct);

    // 3. 默认状态
    HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LORA_RST_GPIO_Port, LORA_RST_Pin, GPIO_PIN_SET);
}

// --- 设置 NSS ---
void SX1278_hw_SetNSS(SX1278_hw_t *hw, int value) {
    HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, 
                      (value == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// --- 复位 ---
void SX1278_hw_Reset(SX1278_hw_t *hw) {
    HAL_GPIO_WritePin(LORA_RST_GPIO_Port, LORA_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(10); 
    HAL_GPIO_WritePin(LORA_RST_GPIO_Port, LORA_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(20); 
}

// --- SPI 命令 ---
void SX1278_hw_SPICommand(SX1278_hw_t *hw, uint8_t cmd) {
    SX1278_hw_SetNSS(hw, 0); // 拉低片选
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 1000);
}

// --- SPI 读取 ---
uint8_t SX1278_hw_SPIReadByte(SX1278_hw_t *hw) {
    uint8_t tx = 0xFF; 
    uint8_t rx = 0;
    HAL_SPI_TransmitReceive(&hspi1, &tx, &rx, 1, 1000);
    return rx;
}

// --- SPI 写入 ---
void SX1278_hw_SPIWriteByte(SX1278_hw_t *hw, uint8_t val) {
    uint8_t rx = 0;
    HAL_SPI_TransmitReceive(&hspi1, &val, &rx, 1, 1000);
}

// --- 延时函数 ---
void SX1278_hw_DelayMs(uint32_t ms) {
    HAL_Delay(ms);
}

// --- 【新增】读取 DIO0 状态 ---
// 这是你刚刚报错缺失的函数
int SX1278_hw_GetDIO0(SX1278_hw_t *hw) {
    return (HAL_GPIO_ReadPin(LORA_DIO0_GPIO_Port, LORA_DIO0_Pin) == GPIO_PIN_SET);
}
