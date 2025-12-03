#include "fuzzyPID.h"
#include "rtthread.h"

#define DOF 6

/* 1. 【关键修改】将大数组移到函数外部，并加上 static 关键字 */
/* 这样它们就不占线程栈空间了，而是占全局内存 */
static int rule_base[][qf_default] = {
        //delta kp rule base
        {PB, PB, PM, PM, PS, ZO, ZO},
        {PB, PB, PM, PS, PS, ZO, NS},
        {PM, PM, PM, PS, ZO, NS, NS},
        {PM, PM, PS, ZO, NS, NM, NM},
        {PS, PS, ZO, NS, NS, NM, NM},
        {PS, ZO, NS, NM, NM, NM, NB},
        {ZO, ZO, NM, NM, NM, NB, NB},
        //delta ki rule base
        {NB, NB, NM, NM, NS, ZO, ZO},
        {NB, NB, NM, NS, NS, ZO, ZO},
        {NB, NM, NS, NS, ZO, PS, PS},
        {NM, NM, NS, ZO, PS, PM, PM},
        {NM, NS, ZO, PS, PS, PM, PB},
        {ZO, ZO, PS, PS, PM, PB, PB},
        {ZO, ZO, PS, PM, PM, PB, PB},
        //delta kd rule base
        {PS, NS, NB, NB, NB, NM, PS},
        {PS, NS, NB, NM, NM, NS, ZO},
        {ZO, NS, NM, NM, NS, NS, ZO},
        {ZO, NS, NS, NS, NS, NS, ZO},
        {ZO, ZO, ZO, ZO, ZO, ZO, ZO},
        {PB, PS, PS, PS, PS, PS, PB},
        {PB, PM, PM, PM, PS, PS, PB}};

static int mf_params[4 * qf_default] = {-3, -3, -2, 0,
                                 -3, -2, -1, 0,
                                 -2, -1,  0, 0,
                                 -1,  0,  1, 0,
                                  0,  1,  2, 0,
                                  1,  2,  3, 0,
                                  2,  3,  3, 0};

static float fuzzy_pid_params[DOF][pid_params_count] = {{0.65f,  0,     0,    0, 0, 0, 1},
                                                 {-0.34f, 0,     0,    0, 0, 0, 1},
                                                 {-1.1f,  0,     0,    0, 0, 0, 1},
                                                 {-2.4f,  0,     0,    0, 0, 0, 1},
                                                 {1.2f,   0,     0,    0, 0, 0, 1},
                                                 {1.2f,   0.05f, 0.1f, 0, 0, 0, 1}};

/* 辅助函数：专门用来打印 float */
void print_float(const char* tag, float val)
{
    // 将浮点数拆分为整数部分和小数部分
    int integer = (int)val;
    int decimal = (int)((val - integer) * 1000); // 保留3位小数
    if (decimal < 0) decimal = -decimal; // 处理负数的小数部分
    
    // 打印格式： Tag: 12.345
    rt_kprintf("%s: %d.%03d\n", tag, integer, decimal);
}

void pid_sim(void) 
{
    rt_kprintf("Start Fuzzy PID Simulation...\n");

    // 2. 使用全局数组初始化
    struct PID **pid_vector = fuzzy_pid_vector_init(fuzzy_pid_params, 2.0f, 4, 1, 0, mf_params, rule_base, DOF);

    if (pid_vector == NULL)
    {
        rt_kprintf("Error: Malloc failed! Check Heap Size.\n");
        return;
    }

    rt_kprintf("PID Init Success.\n");

    int control_id = 5; 
    float real = 0;     
    float idea = max_error * 0.9f; 
    
    // 使用辅助函数打印目标值
    print_float("Target", idea);
    
    bool direct[DOF] = {true, false, false, false, true, true};

    for (int j = 0; j < 1000; ++j) {
        int out = fuzzy_pid_motor_pwd_output(real, idea, direct[control_id], pid_vector[control_id]);
        
        real += (float) (out - middle_pwm_output) / (float) middle_pwm_output * (float) max_error * 0.1f;
        
        // 3. 手动格式化打印：Step 0: Out=600, Real=12.345
        int r_int = (int)real;
        int r_dec = (int)((real - r_int) * 1000);
        if(r_dec < 0) r_dec = -r_dec;

        rt_kprintf("Step %d: Out=%d, Real=%d.%03d\n", j, out, r_int, r_dec);
        
        rt_thread_mdelay(20); 
    }

    delete_pid_vector(pid_vector, DOF);
    rt_kprintf("Simulation Finished.\n");
}
MSH_CMD_EXPORT(pid_sim, Run Fuzzy PID Simulation);
