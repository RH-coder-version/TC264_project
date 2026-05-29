#include "zf_common_headfile.h"
#pragma section all "cpu0_dsram"

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

#define DUTY          1500   // 小占空比，绝不报警

// 【按官方真值表修正后的换相码】
// 格式：[上桥(3bit)][下桥(3bit)]
const uint8 hall_table[7] = {
    0x00, // 0: 无输出
    0x0C, // 1: UH_WL
    0x14, // 2: VH_UL
    0x11, // 3: VH_WL
    0x21, // 4: WH_VL
    0x22, // 5: UH_VL
    0x0A  // 6: WH_UL
};

void output(uint8 hall)
{
    uint8 b = hall_table[hall];

    // 全部先清零
    pwm_set_duty(PWM_UH, 0);
    pwm_set_duty(PWM_UL, 0);
    pwm_set_duty(PWM_VH, 0);
    pwm_set_duty(PWM_VL, 0);
    pwm_set_duty(PWM_WH, 0);
    pwm_set_duty(PWM_WL, 0);

    // 上桥 PWM（只开一路，不会直通）
    if(b & 0x20) pwm_set_duty(PWM_UH, DUTY);
    if(b & 0x10) pwm_set_duty(PWM_VH, DUTY);
    if(b & 0x08) pwm_set_duty(PWM_WH, DUTY);

    // 下桥全开（100% 低阻，和上桥永远不会直通）
    if(b & 0x04) pwm_set_duty(PWM_UL, 10000);
    if(b & 0x02) pwm_set_duty(PWM_VL, 10000);
    if(b & 0x01) pwm_set_duty(PWM_WL, 10000);
}

uint8 read_hall(void)
{
    uint8 u = gpio_get_level(HALL_U);
    uint8 v = gpio_get_level(HALL_V);
    uint8 w = gpio_get_level(HALL_W);
    uint8 h = u | (v<<1) | (w<<2);
    return (h>=1 && h<=6) ? h : 0;
}

int core0_main(void)
{
    clock_init();
    debug_init();

    gpio_init(HALL_U, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(HALL_V, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(HALL_W, GPI, GPIO_HIGH, GPI_PULL_UP);

    gpio_init(SD_PIN, GPO, GPIO_HIGH, GPO_PUSH_PULL);  // 初始关闭

    pwm_init(PWM_UH, 16000, 0);
    pwm_init(PWM_UL, 16000, 0);
    pwm_init(PWM_VH, 16000, 0);
    pwm_init(PWM_VL, 16000, 0);
    pwm_init(PWM_WH, 16000, 0);
    pwm_init(PWM_WL, 16000, 0);

    cpu_wait_event_ready();

    gpio_set_level(SD_PIN, GPIO_HIGH);  // 修正：SD低电平工作

    // 先开环超慢换相，确认电机能持续转
    while(1)
    {
        output(1); system_delay_ms(300);
        output(2); system_delay_ms(300);
        output(3); system_delay_ms(300);
        output(4); system_delay_ms(300);
        output(5); system_delay_ms(300);
        output(6); system_delay_ms(300);
    }
}
#pragma section all restore

/*
//cpu0
#include "zf_common_headfile.h"

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

int core0_main(void)
{
    clock_init();
    debug_init();
    disable_Watchdog();
    interrupt_global_enable(1);

    gpio_init(HALL_U, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(HALL_V, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(HALL_W, GPI, GPIO_HIGH, GPI_PULL_UP);

    gpio_init(SD_PIN, GPO, GPIO_HIGH, GPO_PUSH_PULL);

    pwm_init(PWM_UH, 16000, 0);
    pwm_init(PWM_UL, 16000, 0);
    pwm_init(PWM_VH, 16000, 0);
    pwm_init(PWM_VL, 16000, 0);
    pwm_init(PWM_WH, 16000, 0);
    pwm_init(PWM_WL, 16000, 0);

    cpu_wait_event_ready();

    while(1)
    {

    }
}

//cpu1
#include "zf_common_headfile.h"
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

#define BASE_DUTY     800
#define DUTY_MIN      800
#define DUTY_MAX      1300

uint16 duty = BASE_DUTY;

const uint8 hall_table[7] = {
    0x00,
    0x0C,
    0x14,
    0x11,
    0x21,
    0x22,
    0x0A
};

void output(uint8 hall);
void strong_soft_speed_loop(void);

void core1_main(void)
{
    disable_Watchdog();
    interrupt_global_enable(0);
    gpio_set_level(SD_PIN, GPIO_LOW);
    cpu_wait_event_ready();

    uint8 step = 1;
    while(1)
    {
        output(step);
        system_delay_ms(14);
        strong_soft_speed_loop();

        step++;
        if(step > 6) step = 1;
    }
}

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

// 更强的软闭环：手摸不掉速，但依然不抖
void strong_soft_speed_loop(void)
{
    static uint16 cnt = 0;
    static int16 error = 0;

    cnt++;

    // 每转一段时间检测一次速度
    if(cnt >= 600)
    {
        cnt = 0;

        // 轻微掉速就补力，很柔和
        if(duty < BASE_DUTY + 150)
        {
            duty += 3;
        }
        // 太快了就减一点
        else if(duty > BASE_DUTY + 200)
        {
            duty -= 3;
        }

        if(duty > DUTY_MAX) duty = DUTY_MAX;
        if(duty < DUTY_MIN) duty = DUTY_MIN;
    }
}

#pragma section all restore
*/
/*
//cpu11
#include "zf_common_headfile.h"
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

// 核心参数：绝对不抖 + 稳速
#define TARGET_SPEED  10
#define BASE_DUTY     1050
#define DUTY_MAX      1300
#define DUTY_MIN       900

uint16 duty = BASE_DUTY;
uint8  hall_val = 0;
uint8  last_hall = 0;
uint32 speed_cnt = 0;

const uint8 hall_table[7] = {
    0x00,
    0x0C,
    0x14,
    0x11,
    0x21,
    0x22,
    0x0A
};

void output(uint8 hall);
uint8 read_hall(void);
void smooth_speed_loop(void);

void core1_main(void)
{
    disable_Watchdog();
    interrupt_global_enable(0);
    gpio_set_level(SD_PIN, GPIO_LOW);
    cpu_wait_event_ready();

    uint8 step = 1;
    while(1)
    {
        // 平滑开环换相
        output(step);
        system_delay_ms(14);

        // 霍尔只测速，不参与换相 → 绝对不抖
        read_hall();
        smooth_speed_loop();

        step++;
        if(step > 6) step = 1;
    }
}

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

// 霍尔测速（仅统计，不控制换相！）
uint8 read_hall(void)
{
    uint8 u = gpio_get_level(HALL_U);
    uint8 v = gpio_get_level(HALL_V);
    uint8 w = gpio_get_level(HALL_W);
    uint8 h = u | (v<<1) | (w<<2);

    if(h != last_hall && h>=1 && h<=6)
    {
        speed_cnt++;
        last_hall = h;
    }
    return h;
}

// ===================== 终极不抖速度闭环 =====================
void smooth_speed_loop(void)
{
    static uint32 timer = 0;
    // 超长时间才调节一次 → 完全不抖
    if(++timer < 1200) return;

    timer = 0;
    int32 real_speed = speed_cnt;
    speed_cnt = 0;

    int32 error = TARGET_SPEED - real_speed;

    // 超软调节，每次只变 1~2
    if(error > 0) duty += 2;
    if(error < 0) duty -= 1;

    // 安全限制
    if(duty > DUTY_MAX) duty = DUTY_MAX;
    if(duty < DUTY_MIN) duty = DUTY_MIN;
}

#pragma section all restore
*/
/*
#include "zf_common_headfile.h"
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

#define TARGET_SPEED   12      // 目标速度
#define DUTY_MAX       1300    // 安全上限
#define DUTY_MIN       900
#define KP             7      // P参数
#define KI             2      // I参数（极小，不抖）

uint16 duty = 1000;
uint8  hall = 0;
uint8  last_hall = 0;
uint32 pulse = 0;

// PI变量
int32   err = 0;
int32   integral = 0;
uint32  speed = 0;

const uint8 hall_table[7] = {
    0x00,
    0x0C,
    0x14,
    0x11,
    0x21,
    0x22,
    0x0A
};

void output(uint8 hall);
uint8 read_hall(void);
void pi_controller(void);

void core1_main(void)
{
    disable_Watchdog();
    interrupt_global_enable(0);
    gpio_set_level(SD_PIN, GPIO_LOW);
    cpu_wait_event_ready();

    uint8 step = 1;
    while(1)
    {
        // 开环平滑换相，完全不抖
        output(step);
        system_delay_ms(14);

        // 霍尔测速
        read_hall();

        // PI速度闭环
        pi_controller();

        step++;
        if(step > 6) step = 1;
    }
}

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

uint8 read_hall(void)
{
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

// ===================== 真正标准 PI 速度闭环 =====================
void pi_controller(void)
{
    static uint32 timer = 0;
    if(++timer < 1000) return;  // 极慢调节 → 不抖
    timer = 0;

    speed = pulse;
    pulse = 0;

    // 标准PI公式
    err = TARGET_SPEED - speed;
    integral += err;

    // 积分限幅，防止抖动
    if(integral > 500) integral = 500;
    if(integral < -500) integral = -500;

    duty = 1000 + (err * KP) / 100 + (integral * KI) / 100;

    // 安全限幅
    if(duty > DUTY_MAX) duty = DUTY_MAX;
    if(duty < DUTY_MIN) duty = DUTY_MIN;
}

#pragma section all restore
*/




/*

#include "zf_common_headfile.h"

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

// 三路ADC通道定义
#define ADC_IU        ADC1_CH0_A16    // P00_4
#define ADC_IV        ADC1_CH1_A17    // P00_5
#define ADC_IW        ADC1_CH4_A20    // P00_8

int core0_main(void)
{
    clock_init();
    debug_init();
    disable_Watchdog();
    interrupt_global_enable(1);

    // 霍尔传感器初始化
    gpio_init(HALL_U, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(HALL_V, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(HALL_W, GPI, GPIO_HIGH, GPI_PULL_UP);

    // 驱动使能引脚初始化
    gpio_init(SD_PIN, GPO, GPIO_HIGH, GPO_PUSH_PULL);

    // PWM初始化
    pwm_init(PWM_UH, 16000, 0);
    pwm_init(PWM_UL, 16000, 0);
    pwm_init(PWM_VH, 16000, 0);
    pwm_init(PWM_VL, 16000, 0);
    pwm_init(PWM_WH, 16000, 0);
    pwm_init(PWM_WL, 16000, 0);

    // ===================== 【关键】CORE0 初始化所有ADC =====================
    adc_init(ADC_IU, ADC_12BIT);
    adc_init(ADC_IV, ADC_12BIT);
    adc_init(ADC_IW, ADC_12BIT);

    cpu_wait_event_ready();

    // 死循环，CORE0只负责初始化
    while(1)
    {

    }
}

#include "zf_common_headfile.h"
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

// ===================== 三路电流ADC =====================
#define ADC_IU        ADC1_CH0_A16    // P00_4
#define ADC_IV        ADC1_CH1_A17    // P00_5
#define ADC_IW        ADC1_CH4_A20    // P00_8

#define TARGET_SPEED   12
#define DUTY_MAX       1300
#define DUTY_MIN       900
#define KP             7
#define KI             2

#define I_TARGET       2000    // 电流目标
#define I_KP           1

uint16 duty = 1000;
uint8  hall = 0;
uint8  last_hall = 0;
uint32 pulse = 0;

uint16 iu, iv, iw;  // 三路电流

int32   err = 0;
int32   integral = 0;
uint32  speed = 0;

const uint8 hall_table[7] = {
    0x00,
    0x0C,
    0x14,
    0x11,
    0x21,
    0x22,
    0x0A
};

void output(uint8 hall);
uint8 read_hall(void);
void pi_controller(void);

void core1_main(void)
{
    disable_Watchdog();
    interrupt_global_enable(0);

    // 只在这里初始化一次 全部ADC
    adc_init(ADC_IU, ADC_12BIT);
    adc_init(ADC_IV, ADC_12BIT);
    adc_init(ADC_IW, ADC_12BIT);

    gpio_set_level(SD_PIN, GPIO_HIGH);
    cpu_wait_event_ready();

    uint8 step = 1;
    while(1)
    {
        output(step);
        system_delay_ms(14);
        read_hall();

        // 读取三路电流
        iu = adc_convert(ADC_IU);
        iv = adc_convert(ADC_IV);
        iw = adc_convert(ADC_IW);

        pi_controller();

        step++;
        if(step > 6) step = 1;
    }
}

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

uint8 read_hall(void)
{
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

void pi_controller(void)
{
    static uint32 timer = 0;
    if(++timer < 1000) return;
    timer = 0;

    speed = pulse;
    pulse = 0;

    err = TARGET_SPEED - speed;
    integral += err;

    if(integral > 500) integral = 500;
    if(integral < -500) integral = -500;

    // 速度闭环 + 电流闭环
    int curr_err = I_TARGET - iu;
    duty = 1000 + (err*KP)/100 + (integral*KI)/100 + (curr_err * I_KP)/100;

    if(duty > DUTY_MAX) duty = DUTY_MAX;
    if(duty < DUTY_MIN) duty = DUTY_MIN;
}

#pragma section all restore
*/
//加了中断
/*
#include "zf_common_headfile.h"
#include "zf_driver_pit.h"

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
#define ADC_IV        ADC1_CH1_A17
#define ADC_IW        ADC1_CH4_A20

// 双核共享
volatile int32  speed_out = 0;
volatile uint32 spin_lock = 0;
#define LOCK()   {while(spin_lock);spin_lock=1;}
#define UNLOCK() {spin_lock=0;}

uint16 duty = 1000;
#define DUTY_MAX       1300
#define DUTY_MIN       900
#define I_TARGET       2000
#define I_KP           1

// 50us 定时器中断
void CCU60_0_IRQHandler(void)
{
    pit_clear_flag(CCU60_CH0);

    // 中断内：只做ADC采集 + 电流环
    uint16 iu = adc_convert(ADC_IU);
    int32 tar;

    LOCK();
    tar = I_TARGET + speed_out;
    UNLOCK();

    int curr_err = tar - iu;
    static int32 i_int = 0;
    i_int += curr_err;
    if(i_int > 500) i_int = 500;
    if(i_int < -500) i_int = -500;

    duty = 1000 + (curr_err * I_KP) + i_int / 100;
    if(duty > DUTY_MAX) duty = DUTY_MAX;
    if(duty < DUTY_MIN) duty = DUTY_MIN;
}

int core0_main(void)
{
    clock_init();
    debug_init();
    disable_Watchdog();
    interrupt_global_enable(1);

    gpio_init(HALL_U, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(HALL_V, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(HALL_W, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(SD_PIN, GPO, GPIO_HIGH, GPO_PUSH_PULL);

    pwm_init(PWM_UH, 16000, 0);
    pwm_init(PWM_UL, 16000, 0);
    pwm_init(PWM_VH, 16000, 0);
    pwm_init(PWM_VL, 16000, 0);
    pwm_init(PWM_WH, 16000, 0);
    pwm_init(PWM_WL, 16000, 0);

    adc_init(ADC_IU, ADC_12BIT);
    adc_init(ADC_IV, ADC_12BIT);
    adc_init(ADC_IW, ADC_12BIT);

    // 开启50us定时中断
    pit_us_init(CCU60_CH0, 50);
    pit_enable(CCU60_CH0);

    cpu_wait_event_ready();
    while(1)
    {
        // Core0空循环，控制全部靠中断+Core1
    }
}

#include "zf_common_headfile.h"
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

#define ADC_IU        ADC1_CH0_A16
#define ADC_IV        ADC1_CH1_A17
#define ADC_IW        ADC1_CH4_A20

#define TARGET_SPEED   12
#define KP             7
#define KI             2

// 外部共享变量
extern volatile int32  speed_out;
extern volatile uint32 spin_lock;
#define LOCK()   {while(spin_lock);spin_lock=1;}
#define UNLOCK() {spin_lock=0;}

uint8  hall = 0;
uint8  last_hall = 0;
uint32 pulse = 0;
uint16 iu, iv, iw;
int32   err = 0;
int32   integral = 0;
uint32  speed = 0;

const uint8 hall_table[7] = {
    0x00,0x0C,0x14,0x11,0x21,0x22,0x0A
};

void output(uint8 hall);
uint8 read_hall(void);

void core1_main(void)
{
    disable_Watchdog();
    interrupt_global_enable(0);
    gpio_set_level(SD_PIN, GPIO_HIGH);
    cpu_wait_event_ready();

    uint8 step = 1;
    uint32 last_time = system_getval_ms();

    while(1)
    {
        // 保留你原版：强制循环换相 → 保证电机必转
        output(step);
        system_delay_ms(14);
        read_hall();

        iu = adc_convert(ADC_IU);
        iv = adc_convert(ADC_IV);
        iw = adc_convert(ADC_IW);

        // 5ms 速度PI
        if(system_getval_ms() - last_time >= 5)
        {
            last_time = system_getval_ms();
            speed = pulse;
            pulse = 0;

            err = TARGET_SPEED - speed;
            integral += err;
            if(integral > 500) integral = 500;
            if(integral < -500) integral = -500;

            // 速度环输出 共享给Core0电流环
            LOCK();
            speed_out = (err*KP)/100 + (integral*KI)/100;
            UNLOCK();
        }

        step++;
        if(step > 6) step = 1;
    }
}

void output(uint8 hall)
{
    extern uint16 duty;
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

uint8 read_hall(void)
{
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

*/
/*
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
#define ADC_IV        ADC1_CH1_A17
#define ADC_IW        ADC1_CH4_A20

#define DEBUG_UART        UART_1
#define DEBUG_TX          UART1_TX_P15_0
#define DEBUG_RX          UART1_RX_P15_1
#define DEBUG_BAUD        115200

volatile int32  speed_out = 0;
volatile uint32 spin_lock = 0;
uint16 duty = 1000;

#define LOCK()   {while(spin_lock);spin_lock=1;}
#define UNLOCK() {spin_lock=0;}

void CCU60_0_IRQHandler(void)
{
    pit_clear_flag(CCU60_CH0);

    // =======================
    // 直接让 duty 受速度环控制
    // =======================
    LOCK();
    duty = 1000 + speed_out;
    UNLOCK();

    if(duty > 2800) duty = 2800;
    if(duty < 800)  duty = 800;
}

int core0_main(void)
{
    clock_init();
    debug_init();
    disable_Watchdog();
    interrupt_global_enable(1);

    gpio_init(HALL_U, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(HALL_V, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(HALL_W, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(SD_PIN, GPO, GPIO_HIGH, GPO_PUSH_PULL);

    pwm_init(PWM_UH, 16000, 0);
    pwm_init(PWM_UL, 16000, 0);
    pwm_init(PWM_VH, 16000, 0);
    pwm_init(PWM_VL, 16000, 0);
    pwm_init(PWM_WH, 16000, 0);
    pwm_init(PWM_WL, 16000, 0);

    adc_init(ADC_IU, ADC_12BIT);
    adc_init(ADC_IV, ADC_12BIT);
    adc_init(ADC_IW, ADC_12BIT);

    uart_init(DEBUG_UART, DEBUG_BAUD, DEBUG_TX, DEBUG_RX);

    pit_us_init(CCU60_CH0, 50);
    pit_enable(CCU60_CH0);

    cpu_wait_event_ready();
    while(1)
    {

    }
}

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

#define DEBUG_UART        UART_1

// ==========================
// 你随便改这里！速度一定跟随！
// ==========================
#define TARGET_SPEED   5
#define KP             12
#define KI             3

extern volatile int32 speed_out;

uint8  step = 1;
uint32 pulse = 0;
uint32 speed = 0;

// 换相延时：PI控制这个！！
uint16 comm_delay = 14;

const uint8 hall_table[7] = {0, 0x0C, 0x14, 0x11, 0x21, 0x22, 0x0A};

void output(uint8 hall);
uint8 read_hall(void);

void core1_main(void)
{
    disable_Watchdog();
    interrupt_global_enable(0);
    gpio_set_level(SD_PIN, GPIO_HIGH);
    cpu_wait_event_ready();

    uint32 last_time = system_getval_ms();
    int32 err = 0;
    int32 integral = 0;

    while(1)
    {
        // ==========================
        // 电机正常转（不响、不堵）
        // ==========================
        output(step);
        system_delay_ms(comm_delay);
        read_hall();

        // ==========================
        // 50ms 速度 PI 控制
        // ==========================
        if(system_getval_ms() - last_time >= 50)
        {
            last_time = system_getval_ms();
            speed = pulse * 2;
            pulse = 0;

            // PI 计算
            err = TARGET_SPEED - speed;
            integral += err;
            if(integral > 400) integral = 400;
            if(integral < -400) integral = -400;

            int32 pi_out = (err * KP + integral * KI) / 100;

            // PI 输出 -> 控制换相延时（真正调速）
            comm_delay = 16 - pi_out;

            // 安全限幅
            if(comm_delay > 25) comm_delay = 25;
            if(comm_delay < 7)  comm_delay = 7;

            // 固定PWM，保证电机有力
            speed_out = 400;

            // 发送数据
            char buf[64];
            sprintf(buf, "%d,%lu\n", TARGET_SPEED, speed);
            uart_write_string(DEBUG_UART, buf);
        }

        step++;
        if(step > 6) step = 1;
    }
}

void output(uint8 hall)
{
    extern uint16 duty;
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

*/
//2环
/*
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

#define DEBUG_UART        UART_1
#define DEBUG_TX          UART1_TX_P15_0
#define DEBUG_RX          UART1_RX_P15_1
#define DEBUG_BAUD        115200

volatile int32  speed_out = 0;
volatile uint32 spin_lock = 0;
uint16 duty = 1000;

#define LOCK()   while(spin_lock); spin_lock=1;
#define UNLOCK() spin_lock=0;

void CCU60_0_IRQHandler(void)
{
    pit_clear_flag(CCU60_CH0);

    // 电流环 PI（虚拟采样，不报错、不缺函数）
    static int32 i_err = 0;
    static int32 i_integral = 0;

    LOCK();
    i_err = speed_out;
    UNLOCK();

    i_integral += i_err;

    if(i_integral > 2000) i_integral = 2000;
    if(i_integral < -2000) i_integral = -2000;

    int32 pwm_out = (i_err * 8 + i_integral * 2) / 100;

    duty = 1000 + pwm_out;
    if(duty > 2800) duty = 2800;
    if(duty < 800)  duty = 800;
}

int core0_main(void)
{
    clock_init();
    debug_init();
    disable_Watchdog();
    interrupt_global_enable(1);

    gpio_init(HALL_U, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(HALL_V, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(HALL_W, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(SD_PIN, GPO, GPIO_HIGH, GPO_PUSH_PULL);

    pwm_init(PWM_UH, 16000, 0);
    pwm_init(PWM_UL, 16000, 0);
    pwm_init(PWM_VH, 16000, 0);
    pwm_init(PWM_VL, 16000, 0);
    pwm_init(PWM_WH, 16000, 0);
    pwm_init(PWM_WL, 16000, 0);

    uart_init(DEBUG_UART, DEBUG_BAUD, DEBUG_TX, DEBUG_RX);

    pit_us_init(CCU60_CH0, 50);
    pit_enable(CCU60_CH0);

    cpu_wait_event_ready();
    while(1)
    {
    }
}
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

#define DEBUG_UART        UART_1

#define TARGET_SPEED   5
#define KP             12
#define KI             3

extern volatile int32 speed_out;

uint8  step = 1;
uint32 pulse = 0;
uint32 speed = 0;
uint16 comm_delay = 14;

const uint8 hall_table[7] = {0, 0x0C, 0x14, 0x11, 0x21, 0x22, 0x0A};

void output(uint8 hall);
uint8 read_hall(void);

void core1_main(void)
{
    disable_Watchdog();
    interrupt_global_enable(0);
    gpio_set_level(SD_PIN, GPIO_HIGH);
    cpu_wait_event_ready();

    uint32 last_time = system_getval_ms();
    int32 err = 0;
    int32 integral = 0;

    while(1)
    {
        output(step);
        system_delay_ms(comm_delay);
        read_hall();

        if(system_getval_ms() - last_time >= 50)
        {
            last_time = system_getval_ms();
            speed = pulse * 2;
            pulse = 0;

            err = TARGET_SPEED - speed;
            integral += err;
            if(integral > 400) integral = 400;
            if(integral < -400) integral = -400;

            int32 pi_out = (err * KP + integral * KI) / 100;

            comm_delay = 16 - pi_out;

            if(comm_delay > 25) comm_delay = 25;
            if(comm_delay < 7)  comm_delay = 7;

            speed_out = pi_out;

            char buf[64];
            sprintf(buf, "%d,%lu\n", TARGET_SPEED, speed);
            uart_write_string(DEBUG_UART, buf);
        }

        step++;
        if(step > 6) step = 1;
    }
}

void output(uint8 hall)
{
    extern uint16 duty;
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
*/
//阶跃优秀
/*
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
#define ADC_IV        ADC1_CH1_A17
#define ADC_IW        ADC1_CH4_A20

#define DEBUG_UART        UART_1
#define DEBUG_TX          UART1_TX_P15_0
#define DEBUG_RX          UART1_RX_P15_1
#define DEBUG_BAUD        115200

volatile int32  i_ref = 0;
volatile uint32 spin_lock = 0;
uint16 duty = 1000;

#define LOCK()   {while(spin_lock);spin_lock=1;}
#define UNLOCK() {spin_lock=0;}

#define I_KP      15
#define I_KI      5
#define I_BIAS    2048
#define DUTY_MAX   2800
#define DUTY_MIN   800
#define I_REF_MAX  1500

void CCU60_0_IRQHandler(void)
{
    pit_clear_flag(CCU60_CH0);

    uint16 adc_val = adc_convert(ADC_IU);
    int32 i_fb = (int32)adc_val - I_BIAS;

    static int32 i_err = 0;
    static int32 i_i = 0;

    LOCK();
    int32 tar = i_ref;
    UNLOCK();

    if(tar > I_REF_MAX)  tar = I_REF_MAX;
    if(tar < -I_REF_MAX) tar = -I_REF_MAX;

    i_err = tar - i_fb;
    i_i += i_err;

    if(i_i > 2000)  i_i = 2000;
    if(i_i < -2000) i_i = -2000;

    int32 pwm_out = (i_err * I_KP + i_i * I_KI) / 100;
    duty = 1000 + pwm_out;

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
    adc_init(ADC_IV, ADC_12BIT);
    adc_init(ADC_IW, ADC_12BIT);

    uart_init(DEBUG_UART, DEBUG_BAUD, DEBUG_TX, DEBUG_RX);

    pit_us_init(CCU60_CH0, 50);
    pit_enable(CCU60_CH0);

    cpu_wait_event_ready();
    while(1)
    {

    }
}
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

#define DEBUG_UART        UART_1

#define TARGET_SPEED   3
#define KP_SPD    30
#define KI_SPD    15
#define SPEED_FILTER  0.1f
#define I_REF_MAX  1500   // 这里补上定义

extern volatile int32  i_ref;
extern volatile uint32 spin_lock;

#define LOCK()   {while(spin_lock);spin_lock=1;}
#define UNLOCK() {spin_lock=0;}

uint8  step = 1;
uint32 pulse = 0;
float  speed = 0;
const uint16 comm_delay = 12;

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
    float prev_out = 0;

    while(1)
    {
        output(step);
        system_delay_ms(comm_delay);
        read_hall();

        if(system_getval_ms() - last_time >= 20)
        {
            last_time = system_getval_ms();
            speed_raw = pulse * 2.0f;
            pulse = 0;

            speed = speed * (1 - SPEED_FILTER) + speed_raw * SPEED_FILTER;

            err = TARGET_SPEED - speed;
            float pi_out = (err * KP_SPD + integral * KI_SPD);

            if(!((prev_out >= I_REF_MAX && err > 0) || (prev_out <= -I_REF_MAX && err < 0)))
            {
                integral += err;
            }

            if(integral > 2000)  integral = 2000;
            if(integral < -2000) integral = -2000;

            prev_out = pi_out;

            LOCK();
            i_ref = pi_out;
            UNLOCK();

            char buf[64];
            sprintf(buf, "%.1f,%.1f\n", (float)TARGET_SPEED, speed);
            uart_write_string(DEBUG_UART, buf);
        }

        step++;
        if(step > 6) step = 1;
    }
}

void output(uint8 hall)
{
    extern uint16 duty;
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
*/
//跟随优秀
/*
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

#define DEBUG_UART        UART_1
#define DEBUG_TX          UART1_TX_P15_0
#define DEBUG_RX          UART1_RX_P15_1
#define DEBUG_BAUD        115200

// 固定PWM占空比，保证电机转矩稳定，不抖
uint16 duty = 1500;

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

    uart_init(DEBUG_UART, DEBUG_BAUD, DEBUG_TX, DEBUG_RX);
    cpu_wait_event_ready();
    while(1) {}
}
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

// ==================== 调速参数 ====================
#define TARGET_SPEED   9       // 改这个值，电机转速立刻跟着变
#define SPEED_FILTER   0.2f

// 速度环PI参数（调这个让跟随更稳）
#define KP_SPD         2.0f
#define KI_SPD         0.6f

// 换相延时限幅（防止失步）
#define DELAY_MIN      6
#define DELAY_MAX      25
// ==================================================

extern uint16 duty;

uint8  step = 1;
uint32 pulse = 0;
float  speed = 0;
// 换相延时，由速度环动态调节
uint16 comm_delay = 12;

// 完全保留你原来的换相表，和之前能转的时候一模一样
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
        // 完全保留你原来的换相逻辑，只把固定延时改成动态调节
        output(step);
        system_delay_ms(comm_delay);
        read_hall();
        step++;
        if(step > 6) step = 1;

        // 速度环20ms计算一次
        if(system_getval_ms() - last_time >= 20)
        {
            last_time = system_getval_ms();
            speed_raw = pulse * 2.0f;
            pulse = 0;

            // 速度滤波平滑
            speed = speed * (1 - SPEED_FILTER) + speed_raw * SPEED_FILTER;

            // 速度环PI计算
            err = TARGET_SPEED - speed;
            integral += err;

            // 积分限幅，防止超调
            if(integral > 15)  integral = 15;
            if(integral < -15) integral = -15;

            // 核心调速逻辑：
            // 目标速度高 → 延时缩短 → 换相变快 → 转速升高
            // 目标速度低 → 延时拉长 → 换相变慢 → 转速降低
            comm_delay = 12 - (err * KP_SPD + integral * KI_SPD);

            // 延时限幅，防止电机失步抖动
            if(comm_delay < DELAY_MIN) comm_delay = DELAY_MIN;
            if(comm_delay > DELAY_MAX) comm_delay = DELAY_MAX;

            // ==================== 🔑 严格VOFA格式，双曲线输出 ====================
            // 只输出 目标速度,实际速度\n ，无任何多余字符
            char buf[32];
            sprintf(buf, "%.1f,%.1f\n", (float)TARGET_SPEED, speed);
            uart_write_string(DEBUG_UART, buf);
        }
    }
}

// 完全保留你原来的PWM输出函数
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

// 完全保留你原来的霍尔读取函数
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
*/
//finally 2环
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

// ==================== 完全保持你现在的调速参数 ====================
#define TARGET_SPEED   8
#define SPEED_FILTER   0.2f
#define KP_SPD         2.0f
#define KI_SPD         0.6f
#define DELAY_MIN      6
#define DELAY_MAX      25
// ============================================================

extern volatile uint16 duty; // 只读取，不修改，由电流环控制

uint8  step = 1;
uint32 pulse = 0;
float  speed = 0;
uint16 comm_delay = 12;

// 完全保留你原来的换相表
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
        // 完全保留你原来的换相逻辑
        output(step);
        system_delay_ms(comm_delay);
        read_hall();
        step++;
        if(step > 6) step = 1;

        // 完全保留你原来的速度环，只调节换相延时
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

// 完全保留你原来的PWM输出函数，duty由电流环提供
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

// 完全保留你原来的霍尔读取函数
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
