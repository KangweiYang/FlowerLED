# STM32F401CCU6 希望之花

该文档描述了一个基于STM32F401CCU6控制5个LED的〝希望之花〞。

## 硬件资源

- STM32F401CCU6主控
- 5个LED
- 5个按键开关
- 5路PWM
- 5个定时器
- 5路中断

### 定时器分配

| 定时器 | 工作模式 | 功能 |
|:-:|:-:|:-:| 
| TIM1 | 1kHz 4路PWM | 用于PWM控制4个LED |
| TIM2 | 1kHz 1路PWM | 用于PWM控制第5个LED |
| TIM3 | 500Hz | 用于按键消抖 |  
| TIM4 | 100Hz | 用于LED呼吸效果PWM占空比刷新|
| TIM5 | 2Hz | 0.5s计时、0.5sLED闪烁 |

## 状态机

每个LED有5个状态:

- inAct: LED熄灭
- waitForAct: LED每0.5s闪烁一次,按下对应按键进入acted状态,3s内没有按键则进入inAct状态
- acted: LED常亮
- allActed: 5个LED呼吸效果
- failToAct: 所有LED熄灭

## 工作流程

根据状态机的状态转换写工作流程:

### inAct

- 上电后所有LED进入inAct状态
- 等待3s后,随机选择一个LED进入waitForAct状态

### waitForAct 

- LED以0.5s周期闪烁一次
- 如果按下对应的按键,则对应的LED进入acted状态, 并随机让另一个LED进入waitForAct状态。
- 如果按下错误按键或超过3s未按下按键,则所有LED进入failToAct状态

### acted

- LED常亮
- 如果5个LED全部进入acted状态,则所有LED进入allActed状态

### failToAct

- 所有LED熄灭
- 如果长按任意按键(超过500ms),则进入inAct状态

### allActed

- 5个LED实现呼吸效果


## 代码实现

- 状态机数组保存每个LED的状态
- 5个中断检测5个按键是否触发, 检测到后进入TIM3进行按键消抖
- TIM3进行按键消抖, 并且检测任意按键是否长按(超过500ms)
- TIM5实现0.5s计时、3s超时计时,根据状态机进行状态切换
- TIM1、TIM2实现4路和1路PWM控制LED
- TIM4实现LED呼吸效果：PWM占空比刷新

这个项目利用5个中断、5个定时器和状态机的思想实现了5个按键对应5个LED的控制器。
