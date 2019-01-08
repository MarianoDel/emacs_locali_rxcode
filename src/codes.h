//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F103
// ##
// #### CODES.H ################################
//---------------------------------------------

#ifndef CODES_H_
#define CODES_H_

#include "gpio.h"
#include "hard.h"

#define CODES_TIMER_CNT    (TIM4->CNT)

#define RX_CODE_PLLUP_ON Gpio5PullUpOn()
#define RX_CODE_PLLUP_OFF Gpio5PullUpOff()

#define SIZEOF_BUFF_TRANS	64


//Wait States
typedef enum {
	CW_INIT = 0,
	CW_WAITING

} wait_state_t;

//Code States
typedef enum {
	C_INIT = 0,
	C_SEND_PILOT_A,
	C_SEND_PILOT_B,
	C_SENDING,
	C_SEND_ONE_A,
	C_SEND_ONE_B,
	C_SEND_ONE_C,
	C_SEND_ZERO_A,
	C_SEND_ZERO_B,
	C_SEND_ZERO_C,
	C_FINISH

} CodeStateTX_t;

typedef enum {
	C_RXINIT = 0,
	C_RXINIT_PULLUP,
	C_RXWAIT_PILOT_A,
	C_RXWAIT_PILOT_B,
	C_RXWAIT_BITS_B,
	C_RXWAIT_BITS_C,
	C_RXERROR,
	C_RXOK

} CodeStateRX_t;

//--- Funciones del Modulo ---
unsigned char SendCode16 (unsigned int, unsigned char, unsigned short);
unsigned char SendCode16FixLambda (unsigned int, unsigned char, unsigned short, unsigned short);
unsigned char SendCode16WithPilot (unsigned int, unsigned char, unsigned short, unsigned short);
void SendCode16Reset (void);
resp_t CodesRecvCode16 (unsigned char *);
void CodesRecvCode16Reset (void);
resp_t CodesUpdateTransitions (unsigned char, unsigned int *, unsigned short *);
resp_t CodesWaitFive (void);
void CodesWaitFiveReset (void);

#endif /* CODES_H_ */
