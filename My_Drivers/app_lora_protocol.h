#ifndef __APP_LORA_PROTOCOL_H__
#define __APP_LORA_PROTOCOL_H__

#include <stdint.h>

// --- 网络配置 ---
#define LORA_NET_ID         0x88        // 网络ID，区分不同网络
#define LORA_BEACON_HEAD    0xBE        // 信标帧头 (Beacon)
#define LORA_DATA_HEAD      0xDA        // 数据帧头 (Data)
#define LORA_JOIN_HEAD      0xJO        // 入网请求帧头 (Join)

// --- H-TDMA 时序配置 (单位: ms) ---
#define TDMA_CYCLE_TIME     10000       // 总周期 10秒
#define CAP_PERIOD          2000        // 竞争期(CSMA) 2秒，用于入网或报警
#define SLOT_TIME           1000        // 每个节点的时间片 1秒

// --- 设备信息 ---
// 修改此处：烧录不同板子时，请修改 DEVICE_ID (例如 1号板写1，2号板写2)
#define MY_DEVICE_ID        2           

// --- 数据包结构 ---
#pragma pack(1) // 强制1字节对齐

// 1. 网关广播的信标帧 (Beacon)
typedef struct {
    uint8_t head;           // 0xBE
    uint8_t net_id;         // 网络ID
    uint32_t timestamp;     // 网关的时间戳 (用于同步)
    uint8_t occupied_slots; // 当前已分配的时隙掩码 (可选)
} Beacon_Frame_t;

// 2. 节点上传的数据帧 (Data)
typedef struct {
    uint8_t head;           // 0xDA
    uint8_t dev_id;         // 本机ID
    uint8_t type;           // 数据类型 (0:常规心跳, 1:报警)
    float   temperature;    // 温度
    float   humidity;       // 湿度
    float   pid_output;     // PID计算出的控制量 (配合你的毕业设计)
    uint8_t checksum;       // 简单的校验和
} Node_Data_Frame_t;

#pragma pack()

#endif

