#include "stm32f4xx.h"

void init_USB(void);
void init_GPIO(void);
void send_char_over_USB(uint8_t ch);
void delay(uint32_t ms);
void send_value_to_terminal(uint8_t value);
uint8_t output = 0;

int main(void) {
	 init_USB();  // Initialize USB peripheral
	    init_GPIO(); // Initialize GPIO peripheral

	    uint8_t data = 0x41; // Character value to be sent over USB

	    while (1) {
	        send_char_over_USB(data); // Send character over USB

	        // Output character as a signal on GPIO pin PA5
	        GPIOA->BSRR = GPIO_BSRR_BS_5; // Set GPIOA Pin 5 high

	        delay(1000); // Delay for 1 second

	        GPIOA->BSRR = GPIO_BSRR_BR_5; // Set GPIOA Pin 5 low

	        // Read the input from GPIO pin PA6 within 2 seconds
	        delay(2000); // Delay for 2 seconds

	        uint8_t output = (GPIOA->IDR & GPIO_IDR_IDR_6) ? 1 : 0; // Read input from GPIOA Pin 6

	        send_value_to_terminal(output); // Send the read value to the terminal via USB
	    }
}

void init_USB(void) {
    // Enable USB OTG FS clock
    RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;

    // Enable USB pins (PA11: USB_DM, PA12: USB_DP)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    GPIOA->MODER |= GPIO_MODER_MODER11_1 | GPIO_MODER_MODER12_1; // Alternate function mode
    GPIOA->OTYPER &= ~(GPIO_OTYPER_OT_11 | GPIO_OTYPER_OT_12); // Push-pull output type
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR11 | GPIO_OSPEEDER_OSPEEDR12; // High-speed output
    GPIOA->AFR[1] |= (10 << (4 * (11 - 8))) | (10 << (4 * (12 - 8))); // Alternate function AF10 for USB
}

void init_GPIO(void) {
    // Enable GPIOA clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // Configure GPIO pins PA5 (output) and PA6 (input)
    GPIOA->MODER |= GPIO_MODER_MODER5_0; // Output mode
    GPIOA->MODER &= ~GPIO_MODER_MODER6; // Input mode
}

void send_char_over_USB(uint8_t ch) {
    while (!(USART1->SR & USART_SR_TXE)); // Wait until TX buffer is empty
    USART1->DR = (ch & 0xFF); // Send character
}

void delay(uint32_t ms) {
    SysTick->LOAD = (SystemCoreClock / 1000) - 1; // Configure the SysTick timer for 1ms interrupts
    SysTick->VAL = 0; // Reset the SysTick counter value
    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk; // Enable the SysTick timer

    for (uint32_t i = 0; i < ms; i++) {
        while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)); // Wait until the SysTick timer reaches zero
    }

    SysTick->CTRL = 0; // Disable the SysTick timer
}

void send_value_to_terminal(uint8_t value){
    // Convert the value to a string
    char buffer[10];
    sprintf(buffer, "%d\r\n", value);

    // Send the string over USB to be displayed on the terminal
    uint8_t len = strlen(buffer);
    for (uint8_t i = 0; i < len; i++) {
        while (!(USART1->SR & USART_SR_TXE));
        USART1->DR = buffer[i];
    }
}
