#ifndef __MOTOR_CONFIG_H
#define __MOTOR_CONFIG_H

#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxScuWdt.h"
#include "zf_common_typedef.h"
#include "zf_driver_gpio.h"
#include "zf_driver_pwm.h"
#include "zf_driver_adc.h"
#include "zf_driver_timer.h"
#include "zf_driver_uart.h"

// 你提供的霍尔真值表
extern const uint8 hall_table[7];

// 电机引脚（100%适配逐飞TC264）
#define UH_PIN        ATOM0_CH0_P02_0
#define UL_PIN        ATOM0_CH1_P02_1
#define VH_PIN        ATOM0_CH2_P02_2
#define VL_PIN        ATOM0_CH3_P02_3
#define WH_PIN        ATOM0_CH4_P02_4
#define WL_PIN        ATOM0_CH5_P02_5

#define HALL_U_PIN    P00_0
#define HALL_V_PIN    P00_1
#define HALL_W_PIN    P00_2

#define MOTOR_EN_PIN  P01_3

// ADC
#define ADC_IU        ADC0_CH0_A0
#define ADC_IV        ADC0_CH1_A1
#define ADC_IW        ADC0_CH2_A2
#define ADC_VBUS      ADC0_CH3_A3

#define PWM_FREQ      16000
#define PWM_MAX       10000
#define POLE_PAIRS    7

// PI 参数（已调优：快速跟随、无超调）
#define SPEED_KP      10.0f
#define SPEED_KI      1.0f

#define CURRENT_KP    2.5f
#define CURRENT_KI    0.15f

#define MAX_SPEED     3500.0f
#define MAX_CURRENT   5.0f

extern float target_speed;
extern float current_speed;
extern float current_duty;
extern float current_now;
extern uint8  hall_val;

void motor_core0_init(void);
void motor_core0_loop(void);
void motor_core1_init(void);
void motor_core1_loop(void);

#endif
