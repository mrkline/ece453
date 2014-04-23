#include "cc430x513x.h"
#include "HAL/RF1A.h"
#include "HAL/HAL_PMM.h"

/*******************
 * Function Definition
 */
void Transmit(unsigned char *buffer, unsigned char length);
void ReceiveOn(void);
void ReceiveOff(void);


void InitRadio(void);

void uart_putc(unsigned char c);
void uart_puts(const char *str);


