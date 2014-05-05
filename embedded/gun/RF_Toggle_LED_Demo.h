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

void TriggerPull(void);
