#include "zf_common_headfile.h"
#include "zf_driver_uart.h"
#pragma section all "cpu1_dsram"

#define PWM_UH        ATOM0_CH0_P02_0
#define PWM_UL        ATOM0_CH1_P02_1
#define PWM_VH        ATOM0_CH2_P02_2
#define PWM_VL        ATOM0_CH3_P02_3
#define PWM_WH        ATOM0_CH4_P02_4
#define PWM_WL        ATOM0_CH5_P02_5

#define HALL_U        P00_0
#define HALL_V        P00_1
#define HALL_W        P00_2
#define SD_PIN        P00_3
#define DEBUG_UART    UART_1

// ==================== 保持调速参数 ====================
#define TARGET_SPEED   5
#define SPEED_FILTER   0.2f
#define KP_SPD         2.0f
#define KI_SPD         0.6f
#define DELAY_MIN      6
#define DELAY_MAX      25
// ============================================================

extern volatile uint16 duty; // 只读取不修改，由电流环控制

uint8  step = 1;
uint32 pulse = 0;
float  speed = 0;
uint16 comm_delay = 12;

// 换相表
const uint8 hall_table[7] = {0, 0x0C, 0x14, 0x11, 0x21, 0x22, 0x0A};

void output(uint8 hall);
uint8 read_hall(void);

void core1_main(void)
{
    disable_Watchdog();
    interrupt_global_enable(0);
    gpio_init(SD_PIN, GPO, 1, GPO_PUSH_PULL);
    cpu_wait_event_ready();

    uint32 last_time = system_getval_ms();
    float err = 0;
    float integral = 0;
    float speed_raw = 0;

    while(1)
    {
        // 换相逻辑
        output(step);
        system_delay_ms(comm_delay);
        read_hall();
        step++;
        if(step > 6) step = 1;

        // 速度环，只调节换相延时
        if(system_getval_ms() - last_time >= 20)
        {
            last_time = system_getval_ms();
            speed_raw = pulse * 2.0f;
            pulse = 0;

            speed = speed * (1 - SPEED_FILTER) + speed_raw * SPEED_FILTER;

            err = TARGET_SPEED - speed;
            integral += err;

            if(integral > 15)  integral = 15;
            if(integral < -15) integral = -15;

            // 速度环只调节换相延时，完全不动PWM占空比
            comm_delay = 12 - (err * KP_SPD + integral * KI_SPD);

            if(comm_delay < DELAY_MIN) comm_delay = DELAY_MIN;
            if(comm_delay > DELAY_MAX) comm_delay = DELAY_MAX;

            // 严格VOFA格式，双曲线输出
            char buf[32];
            sprintf(buf, "%.1f,%.1f\n", (float)TARGET_SPEED, speed);
            uart_write_string(DEBUG_UART, buf);
        }
    }
}

// PWM输出函数，duty由电流环提供
void output(uint8 hall)
{
    uint8 b = hall_table[hall];

    pwm_set_duty(PWM_UH, 0);
    pwm_set_duty(PWM_UL, 0);
    pwm_set_duty(PWM_VH, 0);
    pwm_set_duty(PWM_VL, 0);
    pwm_set_duty(PWM_WH, 0);
    pwm_set_duty(PWM_WL, 0);

    if(b & 0x20) pwm_set_duty(PWM_UH, duty);
    if(b & 0x10) pwm_set_duty(PWM_VH, duty);
    if(b & 0x08) pwm_set_duty(PWM_WH, duty);

    if(b & 0x04) pwm_set_duty(PWM_UL, 10000);
    if(b & 0x02) pwm_set_duty(PWM_VL, 10000);
    if(b & 0x01) pwm_set_duty(PWM_WL, 10000);
}

// 霍尔读取函数
uint8 read_hall(void)
{
    static uint8 last_hall = 0;
    uint8 u = gpio_get_level(HALL_U);
    uint8 v = gpio_get_level(HALL_V);
    uint8 w = gpio_get_level(HALL_W);
    uint8 h = u | (v << 1) | (w << 2);

    if(h != last_hall && h >= 1 && h <= 6)
    {
        pulse++;
        last_hall = h;
    }
    return h;
}

#pragma section all restore
