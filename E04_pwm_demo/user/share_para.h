#ifndef _SHARE_PARA_H_
#define _SHARE_PARA_H_

#include "zf_common_typedef.h"

// 自旋锁定义
extern volatile uint32 spin_lock;

#define LOCK()   while(spin_lock){}; spin_lock=1
#define UNLOCK() spin_lock=0

// 速度环输出 → 给Core0电流环使用
extern volatile int32  speed_loop_output;

// 速度反馈 来自霍尔
extern volatile int32  motor_speed_feedback;

#endif
