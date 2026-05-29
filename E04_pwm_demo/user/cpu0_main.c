#include "zf_common_headfile.h"
#include "zf_driver_pit.h"
#include "zf_driver_uart.h"

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

#define ADC_IU        ADC1_CH0_A16
#define DEBUG_UART        UART_1
#define DEBUG_TX          UART1_TX_P15_0
#define DEBUG_RX          UART1_RX_P15_1
#define DEBUG_BAUD        115200

// ==================== 电流环参数 ====================
#define I_KP          10
#define I_KI          3
#define I_BIAS        2048
#define I_TARGET      300    // 目标电流（单位：ADC值，对应转矩）
#define DUTY_MAX      2800
#define DUTY_MIN      800
// ==================================================

// 全局变量，双环共享
volatile uint16 duty = 1500;  // 电流环输出的PWM占空比
volatile int32  i_ref  = I_TARGET; // 目标电流（可以固定，也可以由速度环动态调节）

void CCU60_0_IRQHandler(void)
{
    pit_clear_flag(CCU60_CH0);

    // 读取相电流
    uint16 adc_val = adc_convert(ADC_IU);
    int32 i_fb = (int32)adc_val - I_BIAS;

    // 电流环PI计算
    static int32 i_err = 0;
    static int32 i_i = 0;

    i_err = i_ref - i_fb;
    i_i += i_err;

    // 积分限幅
    if(i_i > 2000)  i_i = 2000;
    if(i_i < -2000) i_i = -2000;

    // 计算PWM占空比
    int32 pwm_out = (i_err * I_KP + i_i * I_KI) / 100;
    duty = 1500 + pwm_out;

    // 占空比限幅
    if(duty > DUTY_MAX) duty = DUTY_MAX;
    if(duty < DUTY_MIN) duty = DUTY_MIN;
}

int core0_main(void)
{
    clock_init();
    debug_init();
    disable_Watchdog();
    interrupt_global_enable(1);

    gpio_init(HALL_U, GPI, 0, GPI_PULL_UP);
    gpio_init(HALL_V, GPI, 0, GPI_PULL_UP);
    gpio_init(HALL_W, GPI, 0, GPI_PULL_UP);
    gpio_init(SD_PIN, GPO, 1, GPO_PUSH_PULL);

    pwm_init(PWM_UH, 16000, 0);
    pwm_init(PWM_UL, 16000, 0);
    pwm_init(PWM_VH, 16000, 0);
    pwm_init(PWM_VL, 16000, 0);
    pwm_init(PWM_WH, 16000, 0);
    pwm_init(PWM_WL, 16000, 0);

    adc_init(ADC_IU, ADC_12BIT);
    uart_init(DEBUG_UART, DEBUG_BAUD, DEBUG_TX, DEBUG_RX);

    pit_us_init(CCU60_CH0, 50);
    pit_enable(CCU60_CH0);

    cpu_wait_event_ready();
    while(1) {}
}
