# FlowerLED

TIM1: 1kHz           4PWM

TIM2: 1kHz           1PWM

TIM3: 500Hz         5ms按键消抖

TIM4: 100Hz         呼吸灯刷新占空比

TIM5: 2Hz             0.5s*6计时 & 0.5s闪烁

## 状态机：

### Single LED:

0: inAct: Light off

	TIM5 count time, let random.0 - > random.1

1: waitForAct: 0.5s light ON, 0.5s light OFF, ……

	if(SW(this) == SET){

		this.1 - > this.2

		random(except acted LEDs).0 - > random.1

	}	

	if(SW(else) == SET)                 ALL.* - > ALL.3

2: acted: Light ON

	if(LED(all) == 2)                 ALL.2 - > ALL.4

3: failToAct: ALL LED OFF

	if(SW(any) == SET)                 TIM5 count time

	(if TIM5 found the SW(any) is pressed for 2s

	 ALL.3 - > ALL.0

 4: allActed: ALL LED breath lighting
