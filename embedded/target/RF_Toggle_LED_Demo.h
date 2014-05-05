#include "cc430x513x.h"
#include "HAL/RF1A.h"
#include "HAL/HAL_PMM.h"

/*******************
 * Function Definition
 */
void Transmit(unsigned char *buffer, unsigned char length);
void ReceiveOn(void);
void ReceiveOff(void);

//void InitButtonLeds(void);
void InitRadio(void);

void Sensors(void);
void LEDs(void);

void uart_putc(unsigned char c);
void uart_puts(const char *str);
void uart_getc(void);
//void uart_gets(void);

void gpio_config(void);				//Configure the GPIOs
void adc_config(void);				//Configure the ADC
void uart_config(void);				//Configure the UART
void timer_config(void);
