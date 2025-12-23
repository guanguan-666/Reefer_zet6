#include "rtthread.h"
#include "main.h"              
#include "stm32f1xx_hal.h"     
#include "bsp_lora.h"          
#include "SX1278.h"            
#include "app_lora_protocol.h" 
#include <string.h>
#include <stdlib.h> 
#include <math.h>

// --- 全局变量 ---
static rt_thread_t tid_lora = RT_NULL;
static SX1278_hw_t sx1278_hw;
static SX1278_t sx1278;

// --- 外部引用 ---
// 如果没有，可以注释掉下面这行
// extern void Get_System_Status(float *temp, float *hum, float *pid_out);

// --- 发送数据函数 ---
void Send_Sensor_Data(void) {
    Node_Data_Frame_t packet;
    
    // 模拟数据 (确保不会因为野指针获取数据出错)
    float temp = 26.5f;
    float hum = 55.0f;
    float pid = 120.0f;

    // 填充数据包
    memset(&packet, 0, sizeof(packet)); // 清空内存，防止乱码
    packet.head = LORA_DATA_HEAD;
    packet.dev_id = MY_DEVICE_ID;
    packet.type = 0; 
    packet.temperature = temp;
    packet.humidity = hum;
    packet.pid_output = pid;
    
    // 简单的校验和
    uint8_t *p = (uint8_t*)&packet;
    packet.checksum = 0;
    for(int i=0; i<sizeof(Node_Data_Frame_t)-1; i++) {
        packet.checksum += p[i];
    }

    // 拆分整数和小数打印 (例如 26.5)
    int temp_int = (int)temp;                  // 整数部分 26
    int temp_dec = (int)((temp - temp_int) * 10); // 小数部分 5

    rt_kprintf("[LoRa] TX Data: ID=%d, T=%d.%d\n", packet.dev_id, temp_int, abs(temp_dec));
    
    // 调用发送接口
    SX1278_LoRaEntryTx(&sx1278, sizeof(Node_Data_Frame_t), 2000);
    SX1278_LoRaTxPacket(&sx1278, (uint8_t*)&packet, sizeof(Node_Data_Frame_t), 2000);
    
    // 发送完后切回 RX
    SX1278_LoRaEntryRx(&sx1278, 16, 2000); 
}

// --- LoRa 核心线程 (轮询模式) ---
void lora_thread_entry(void *parameter) {
    uint8_t rx_buffer[64];
    int rx_len;

    rt_kprintf(">>> LoRa Polling Node Started. ID: %d <<<\n", MY_DEVICE_ID);

    // 1. 启动接收模式
    SX1278_LoRaEntryRx(&sx1278, 16, 2000);

    while (1) {
        // --- 轮询查询 ---
        // 每 10ms 检查一次是否有数据包
        rt_thread_mdelay(10); 

        // 检查接收状态
        rx_len = SX1278_LoRaRxPacket(&sx1278);
        
        if (rx_len > 0) {
            // 读取数据
            SX1278_read(&sx1278, rx_buffer, rx_len);

            // 打印收到的原始数据头，用于调试
            // rt_kprintf("[Debug] RX: 0x%02X (Len=%d)\n", rx_buffer[0], rx_len);

            // --- 解析 ---
            if (rx_buffer[0] == LORA_BEACON_HEAD) {
                rt_kprintf("[LoRa] Beacon Recv! Syncing...\n");

                // H-TDMA 延时逻辑
                uint32_t wait_ms = CAP_PERIOD + ((MY_DEVICE_ID - 1) * SLOT_TIME);
                
                // 等待时隙
                rt_thread_mdelay(wait_ms);

                // 发送数据
                Send_Sensor_Data();
                
                // 重新进入接收模式，准备下一轮
                SX1278_LoRaEntryRx(&sx1278, 16, 2000);
            }
        }
    }
}

// --- 初始化函数 ---
int App_LoRa_Init(void) {
    rt_kprintf("Initializing LoRa...\n");

    // 1. 硬件初始化
    SX1278_hw_init(&sx1278_hw); 
    sx1278.hw = &sx1278_hw;

    // 2. 射频初始化
    // 务必确认这里的频率 433000000 是否和你买的模块匹配
    SX1278_init(&sx1278, 433000000, SX1278_POWER_17DBM, SX1278_LORA_SF_7, 
                SX1278_LORA_BW_125KHZ, SX1278_LORA_CR_4_5, SX1278_LORA_CRC_EN, 10);

    rt_kprintf("SX1278 Init Done.\n");

    // 3. 创建并启动线程
    // 栈大小设为 2048 足够了
    tid_lora = rt_thread_create("lora_pol", lora_thread_entry, RT_NULL, 2048, 10, 10);
    if (tid_lora != RT_NULL) {
        rt_thread_startup(tid_lora);
    } else {
        rt_kprintf("Error: LoRa Thread Create Failed!\n");
    }
    return 0;
}

// 自动初始化
//INIT_APP_EXPORT(App_LoRa_Init);

