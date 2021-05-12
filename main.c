/*
  __  __  ____  _____                                                      
 |  \/  |/ __ \|  __ \                                                     
 | \  / | |  | | |__) |  ______                                            
 | |\/| | |  | |  ___/  |______|                                           
 | |  | | |__| | |                                                         
 |_|__|_|\____/|_|____       _______    _____ _    _ ______ ______ _______ 
  / ____| |  | |  ____|   /\|__   __|  / ____| |  | |  ____|  ____|__   __|
 | |    | |__| | |__     /  \  | |    | (___ | |__| | |__  | |__     | |   
 | |    |  __  |  __|   / /\ \ | |     \___ \|  __  |  __| |  __|    | |   
 | |____| |  | | |____ / ____ \| |     ____) | |  | | |____| |____   | |   
  \_____|_|  |_|______/_/    \_\_|    |_____/|_|  |_|______|______|  |_|   

  ==== My personal cheat sheet with a few
  ==== examples on how to use basic IO on MD407.
*/

// ====== GPIO =======
// D & E are the once used 
// for peripheral devices 
#define GPIO_A 0x40020000 
#define GPIO_B 0x40020400
#define GPIO_C 0x40020800
#define GPIO_D 0x40020C00
#define GPIO_E 0x40021000


// MODER - Port mode register
// Two bits are used to configure each pin and if it is
// 00=input, 01=output, 10=alternative function or 11=analogue
#define GPIO_D_MODER    ((volatile unsigned int *) GPIO_D)

// OTYPER - Output type register
// On bit for each pin, 0=push-pull, 1=open drain
#define GPIO_D_OTYPER   ((volatile unsigned short *) (GPIO_D + 0x04))

// OSPEEDR - Output Speed Register
// Controls update freq. for each pin. 00=lowest 11=highest, and so on
#define GPIO_D_OSPEEDR  ((volatile unsigned short *) (GPIO_D + 0x08))

// PUPDR - Pull-Up / Pull-Down Register
// 00=floating, 01=pull-up, 10=pull-down  
#define GPIO_D_PUPDR    ((volatile unsigned short *) (GPIO_D + 0x0c))

// IDR - Input data register
#define GPIO_D_IDR        ((volatile unsigned short *) (GPIO_D + 0x10))

// ODR - Output data register
#define GPIO_D_ODR        ((volatile unsigned short *) (GPIO_D + 0x14))

// You can of course define smaller sections too if you want:
#define IDR_D_LOW         ((volatile unsigned char *) (GPIO_D + 0x10))
#define IDR_D_HIGH        ((volatile unsigned char *) (GPIO_D + 0x11))
#define ODR_D_LOW         ((volatile unsigned char *) (GPIO_D + 0x14))
#define ODR_D_HIGH        ((volatile unsigned char *) (GPIO_D + 0x15))

#define GPIO_D_BSRR       ((volatile unsigned int *) (GPIO_D + 0x18))
#define GPIO_D_LCKR       ((volatile unsigned short *) (GPIO_D + 0x1c))
#define GPIO_D_AFRL       ((volatile unsigned int *) (GPIO_D + 0x20))
#define GPIO_D_AFRH       ((volatile unsigned int *) (GPIO_D + 0x20))

// the same offsets goes for the other registers but using another base address
// instead of GPIO_D.

/*
  What pins should I init?
  Sample algoritm from the lecture:

  Set port MODER

  if (has input pins):
    Set PUPD register
  
  if (has output pins):
    Set OTYPER
    set OSPEEDR
  
  if (all port pin):
    *GPIO_FOO = BAR;
  else:
    Mask the specific pins with &= and |=
*/

// ==== SysTick (System timer) =====
#define STK 0xE000E010
#define STK_CTRL  ((volatile unsigned int *) (STK))
#define STK_LOAD  ((volatile unsigned int *) (STK+0x04))
#define STK_VAL   ((volatile unsigned int *) (STK+0x08))
#define STK_CALIB ((volatile unsigned int *) (STK+0x0C))

// To set custom interrupt, set SCB_VTOR to start of table containing irq handlers
// at different offsets depending on iterrupt type
#define SCB_VTOR        ((volatile uint32_t *) 0xE000ED08)

// Make a custom start address to interrupt vector in writeable memory
#define IRQH_BASE      0x2001C000
#define IRQH_STK       ((void (**)(void)) (RESET_BASE + 0x3C)) //interrupt request handler for systick
// !! Don't forget to write base addr to SCB_VTOR using *SCB_VTOR = RESET_BASE; !!


// example usage of async systick, if(systick_flag) then the timer has finished
static volatile uint8_t systick_flag;
static volatile uint32_t delay_count;

void delay_1micro(void) {
  *STK_CTRL = 0;
  *STK_LOAD = 167;
  *STK_VAL = 0;
  *STK_CTRL = 7;
}

void systick_irq_handler(void) {
  *STK_CTRL = 0;
  if (--delay_count) {
    delay_1micro();
  } else {
    systick_flag = 1;
  }
}

//async delay in microseconds
void delay(uint32_t count) {
  if (!count) return;
  delay_count = count;
  systick_flag = 0;
  delay_1micro();
}
// !! ADD *RESET_STK = &systick_irq_handler; TO INIT !!


// example usage of systick, taken directly from the lecture slides
void delay_250ns(void) {
  /* SystemCoreClock = 168000000 */
  *STK_CTRL = 0;
  *STK_LOAD = ( (168/4) -1 );
  *STK_VAL = 0;
  *STK_CTRL = 5; // start the counter

  // wait until the status bit in the control
  // register signals that the countdown is over
  while( (*STK_CTRL & 0x10000 ) == 0);

  *STK_CTRL = 0; // reset
}

void delay_micro(unsigned int us) {
  // to account for the added delay in the software:
  #ifdef SIMULATOR 
    us = us / 1000;
    us++;
  #endif
  
  while(us > 0){
    delay_250ns();
    delay_250ns();
    delay_250ns();
    delay_250ns();
    us--;
  }
}

// ==== Graphics init ====

__attribute__((naked))
void graphic_initalize(void){
    __asm volatile(" .HWORD 0xDFF0\n");
    __asm volatile(" BX LR\n");
}

__attribute__((naked))
void graphic_clear_screen(void){
    __asm volatile(" .HWORD 0xDFF1\n");
    __asm volatile(" BX LR\n");
}

__attribute__((naked))
void graphic_pixel_set(int x, int y){
    __asm volatile(" .HWORD 0xDFF2\n");
    __asm volatile(" BX LR\n");
}

__attribute__((naked))
void graphic_pixel_clear(int x, int y){
    __asm volatile(" .HWORD 0xDFF3\n");
    __asm volatile(" BX LR\n");
}
