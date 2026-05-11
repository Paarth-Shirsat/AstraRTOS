# GPIO Driver
General Purpose Input/Output is a hardware block which is used by the CPU to configure the physical pins on the board, CPU uses it to either set then high/low or to read the voltages on them.

On the STM Discovery board (and generally on most microcontrollers) pins are grouped into ports. The F429 has GPIOA, GPIOB, GPIOC all the way upto GPIOK, that's 11 ports with 16 pins each, around 140+ pins in total.

The reason ports are grouped is that each port shares one configuration register. So when we want to configure all 16 pins of a port we just write a single 32 bit word and they all get configured in one go. Much faster than having a seperate register for every pin on the chip.

## Memory Map 
Each port has about 10 registers, each register controlling a different aspect of the 16 pins in that port. Each register on Cortex-M is 32 bits (4 bytes), hence 40 bytes of registers storage per port. The 10 configure registers are as follows:

- **MODER** (mode register): sets the mode of each pin, 2 bits per pin. The four modes are input, output, alternate function, analog
- **OTYPER** (output type): 1 bit per pin, push-pull or open-drain
- **OSPEEDR** (output speed): 2 bits per pin, controls how fast the pin can switch (slew rate)
- **PUPDR** (pull up/pull down): 2 bits per pin, enables internal pull-up or pull-down resistors
- **IDR** (input data register): 1 bit per pin, reading this tells you the current voltage level on the pin
- **ODR** (output data register): 1 bit per pin, writing here drives the pin high or low
- **BSRR** (bit set/reset register): atomic version of ODR, more on this below
- **AFRL / AFRH** (alternate function low/high): 4 bits per pin, selects which peripheral owns the pin (UART, SPI, timer, etc)

> look into the reference manual RM0090 section 8.4.11 for the full register map

The chip reserves 0x400 for each port, even though only 40 bytes of registers storage is needed per port, the rest of the storage is reserved. Hence the port addresses are as follows:

| Port  | Base address  |
|-------|--------------|
| GPIOA | `0x40020000` |
| GPIOB | `0x40020400` |
| GPIOC | `0x40020800` |
| GPIOD | `0x40020C00` |
| GPIOE | `0x40021000` |
| GPIOF | `0x40021400` |
| GPIOG | `0x40021800` |
| GPIOH | `0x40021C00` |
| GPIOI | `0x40022000` |
| GPIOJ | `0x40022400` |
| GPIOK | `0x40022800` |

Each base address is exactly `0x400` apart from the next, i.e. there is a 1 KB slot per port.

## Setting mode with 2 bits per pin
Using the MODER register we are able to set the mode for each pin, it's a 32-bit register and there are 16 pins per port, so each pin gets 2 bits to determine the mode.

Pin N's mode bits live at positions `[2N+1 : 2N]`. So for example pin 13's mode bits are at positions 26 and 27.

This is why the gpio code looks like:

```c
int pin_offset = pin*2;
GPIO_MODER(base) &= ~(3 << pin_offset);
GPIO_MODER(base) |= (mode << pin_offset);
```

The `& ~(3 << pin_offset)` clears the existing 2 bits first and the `|= (mode << pin_offset)` writes the new value. We have to clear first because if the pin was already configured to something else, ORing will mess up the configuration.

## BSRR

Setting a pin high using ODR can be done by doing `GPIO_ODR |= (1 << 13)`. However this compiles into three instructions which are load ODR, OR with the mask and then store back. If an interrupt or context switch happen whilte we are doing these instructions and another piece of code modifies the ODR and the logic breaks

This is known as a race condition which breaks the RTOS hence we use the hardware register BSRR which is a write only register where:

- Writing `1` to bit N sets pin N high
- Writing `1` to bit (N+16) resets pin N low

And since this is a single store instruction (only requires 1 one cycle), no other tasks can break this logic which is why `gpio_write` uses BSRR:

```c
if(value == 1){
    GPIO_BSRR(base) = (1 << pin);        // set
}else{
    GPIO_BSRR(base) = (1 << (pin + 16)); // reset    
}
```

## IDR

When we want to read GPIO pins we use the IDR (input data register). It is a read only register where the low 16 bits show the values of the respecive pins in that port i.e. bit N is 1 if pin N is high and 0 if it is low

To read a single pin we shift the register down by the pin number with right shift to get the value of that pin

```c
uint32_t gpio_read(uint32_t base, uint32_t pin){    
    return ((GPIO_IDR(base)) >> pin) & 1;
}
```

This only works when the pin is configured in input mode or otherwise the input buffer is disconnected from the pin and IDR will just read garbage values.

## Alternate functions

Other than simple input and output most pins on the F429 are multiplexed. For example PA2 can used as plain GPIO or as USART2_TX or as TIM2_CH3 etc. The MODER register has a mode called "alternate function" and when a pin is in that mode, the AFRL/AFRH registers decide which peripheral is to be configured by the pin

The UART driver puts PA2 and PA3 into AF7 (USART2 alternate function) so that the UART peripheral can drive those pins instead of the GPIO block

## Register sequence

Flow of using GPIO:

1. Enable the port clock in `rcc_enable_gpio()`
2. Set the pin mode using `gpio_set_mode()`
3. To write pin `gpio_write()`
4. To read pin level `gpio_read()`
5. To toggle pin `gpio_toggle()`