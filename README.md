# Sequential Light FSM (STM32)

A non-blocking, interrupt-driven 3-light controller built on an STM32 NUCLEO-F429ZI board. A single pushbutton drives a finite state machine (FSM) that sequences three LEDs and plays a distinct buzzer tone on each state transition.



## Function

Pressing the pushbutton (while idle) begins the sequence:

```

STATE_IDLE --(button press)--> STATE_RED --(3s)--> STATE_YELLOW --(3s)--> STATE_GREEN (holds)

```

* Releasing the button at any point immediately resets everything back to `STATE_IDLE`, with all LEDs off and buzzer off.
* Each state transition triggers a short (1s) buzzer tone at a distinct pitch.
* Timing is handled via `HAL_GetTick()` timestamp comparisons in the main loop, so the MCU stays responsive to interrupts throughout.



* State transitions and pushbutton edge detection are handled in `HAL_GPIO_EXTI_Callback()`(ISR context). State duration and buzzer timing are evaluated in `main()`'s loop.



## Hardware



**Target board**: NUCLEO-F429ZI (ARM Cortex-M4 @ 180MHz)



Breadboard prototype pinout:



| Pin | Nucleo Connector | Function |
|-----|-------------------|----------|
| PF13 | D7 | Pushbutton input (EXTI, hardware debounced, internal pull-up) |
| PF15 | D2 | Red LED transistor (push-pull, active high) |
| PE13 | D3 | Yellow LED transistor (push-pull, active high) |
| PF14 | D4 | Green LED transistor (push-pull, active high) |
| PE9  | D6 | Passive buzzer (TIM1\_CH1 PWM alternate function) |



The buzzer's tone changes per state. Each transition sets a new PWM frequency (via TIM1 CH1's autoreload value) at 50% duty cycle, then cuts off after 1 second.



## Building



1. Open the project in STM32CubeIDE.

2. The `.ioc` file contains the full pin/peripheral configuration. Regenerate via CubeMX if needed.

3. Build and flash to a NUCLEO-F429ZI over the onboard ST-LINK.



## Licensing



Vendor files under `Drivers/` retain their STMicroelectronics/ARM licenses. Application logic (FSM, interrupt handling, buzzer control) in `main.c` is authored by me.

