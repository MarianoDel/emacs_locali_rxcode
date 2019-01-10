//----------------------------------------------------------------------
// #### PROYECTO PARA PLACA ARDUINO STM32 - Arduino Blue Pill Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F103
// ##
// #### MAIN.C ############################################
//---------------------------------------------------------

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "hard.h"

#include "timer.h"
#include "gpio.h"
#include "usart.h"

#include "codes.h"

#include <stdio.h>



/* Externals ------------------------------------------------------------------*/
//--- Externals para avisar data ready en usart
volatile unsigned char usart1_have_data;
volatile unsigned char usart2_have_data;
volatile unsigned char usart3_have_data;
#ifdef STM32F10X_HD
volatile unsigned char usart4_have_data;
volatile unsigned char usart5_have_data;
#endif

unsigned short comms_messages = 0;
char buffSendErr[64];

//--- Externals para enviar keepalive por UART
#define TIME_RUN_DEF 250
volatile unsigned short timeRun = TIME_RUN_DEF;

//--- Externals para muestreos de corriente con el ADC
volatile unsigned char flagMuestreo = 0;
volatile unsigned char take_current_samples = 0;

//--- Externals para armar seniales y comprobar el TIM5 en el inicio del programa
volatile unsigned int session_warming_up_channel_1_stage_time = 0;

//--- Externals para el BUZZER
unsigned short buzzer_timeout = 0;

//--- Externals de los timers
volatile unsigned short wait_ms_var = 0;
volatile unsigned short comms_timeout = 0;
volatile unsigned short timer_standby = 0;


/* Globals ------------------------------------------------------------------*/

//--- Module Functions Declarations ----------
void TimingDelay_Decrement(void);
extern void EXTI0_IRQHandler (void);

//--- Module Function Definitions ----------

int main (void)
{
    unsigned char i = 0;
    unsigned long ii = 0;

    unsigned int new_code = 0;
    unsigned short new_lambda = 0;
    char s_buf [128];

    main_state_t main_state = MAIN_WAIT_SILENCE;
    resp_t resp = resp_continue;

    //Configuracion systick    
    if (SysTick_Config(72000))
    {
        while (1)	/* Capture error */
        {
            if (LED)
                LED_OFF;
            else
                LED_ON;

            for (i = 0; i < 255; i++)
            {
                asm (	"nop \n\t"
                        "nop \n\t"
                        "nop \n\t" );
            }
        }
    }

    // Configuracion led. & Enabled Channels
    GpioInit();

    //enciendo usart1
    Usart1Config();

    // //enciendo usart2 para comunicacion con micros
    // Usart2Config();
    

    //-- Welcome Messages --------------------
#ifdef HARD
    Usart1Send(HARD);
    Wait_ms(100);    
#else
#error	"No Hardware defined in hard.h file"
#endif

#ifdef SOFT
    Usart1Send(SOFT);
    Wait_ms(100);    
#else
#error	"No Soft Version defined in hard.h file"
#endif
#ifdef FEATURES
    Usart1Send((char *) FEATURES);
    Wait_ms(100);
#endif

    // TIM_1_Init();
    // TIM_3_Init();
    TIM_4_Init();

    // UpdateTIMSync(DUTY_FOR_DMAX);
    // EXTIOn();    

    Usart1Send((char *) "Listo para recibir codigos\n");
    // while (1)
    // {
    //     LED_ON;
    //     // EXTIOn();
    //     Wait_ms(1000);
    //     LED_OFF;
    //     // EXTIOff();
    //     Wait_ms(1000);
    // }

    LED_OFF;
    OCODE_OFF;
    
    while (1)
    {
        switch (main_state)
        {
        case MAIN_WAIT_SILENCE:
            CodesWaitFiveReset();
            main_state++;
            break;
            
        case MAIN_WAIT_SILENCE_A:

            resp = CodesWaitFive();

            if (resp == resp_ok)
            {
                //estuve 5ms sin nada
                LED_ON;
                OCODE_ON;

                main_state = MAIN_RX;
                // Usart1Send((char *) "nuevo header\n");
                CodesRecvCode16Reset();
            }

            if (resp == resp_error)
                main_state = MAIN_WAIT_SILENCE;
            
            break;

        case MAIN_RX:
            resp = CodesRecvCode16(&i);

            if (resp != resp_continue)
            {
                if (resp == resp_ok)
                {
                    // sprintf(s_buf, "bits: %d OK\n", i);
                    main_state = MAIN_GET_CODE_HT;
                }

                if (resp == resp_error)
                {
                    sprintf(s_buf, "bits: %d ERR\n", i);
                    main_state = MAIN_WAIT_SILENCE;
                }
                
                LED_OFF;
                OCODE_OFF;
                Usart1Send(s_buf);
            }
            break;

        case MAIN_GET_CODE_HT:
            resp = CodesUpdateTransitionsHT(i, &new_code, &new_lambda);
                        
            if (resp == resp_ok)
            {
                sprintf(s_buf, "code: 0x%x lambda: %d bits: %d HT\n",
                        new_code,
                        new_lambda,
                        i);

                Usart1Send(s_buf);
                main_state = MAIN_WAIT_SILENCE;
            }
            else
            {
                main_state = MAIN_GET_CODE_PT_EV;
            }
            break;
            
        case MAIN_GET_CODE_PT_EV:
            resp = CodesUpdateTransitionsPT_EV(i, &new_code, &new_lambda);

            if (resp == resp_ok)
            {
                sprintf(s_buf, "code: 0x%x lambda: %d bits: %d PT or EV\n",
                        new_code,
                        new_lambda,
                        i);
            }
            else
            {
                sprintf(s_buf, "code error\n");
            }

            Usart1Send(s_buf);
            main_state = MAIN_WAIT_SILENCE;
            break;
            
        default:
            main_state = MAIN_WAIT_SILENCE;
            break;

        }
    }

    
    //---- Prueba Usart3 ----------
    // while (1)
    // {
    //     // Wait_ms(2000);
    //     L_ALARMA_OFF;
    //     Usart3Send("HOLA!!!\n");
    //     Wait_ms(100);

    //     if (usart3_have_data)
    //     {
    //         usart3_have_data = 0;
    //         L_ALARMA_ON;
    //         ReadUsart3Buffer(local_buff, 64);
    //         if (strcmp((const char *) "HOLA!!!", local_buff) == 0)
    //             L_ZONA_ON;
    //         else
    //             L_ZONA_OFF;

    //         Wait_ms(100);
    //         L_ALARMA_OFF;
    //         L_ZONA_OFF;
    //         Wait_ms(1900);
    //     }
    // }
    //---- Fin Prueba Usart3 ----------

    //---- Prueba contra PC o Raspberry ----------
    // while (1)
    // {
    //     UpdateRaspberryMessages();
    // }
    //---- Fin Prueba contra PC o Raspberry ----------    


    //---- Prueba Usart3 loop en terminal ----------
    // while (1)
    // {
    //     if (usart3_have_data)
    //     {
    //         usart3_have_data = 0;
    //         L_ALARMA_ON;
    //         ReadUsart3Buffer(local_buff, 64);
    //         Wait_ms(1000);
    //         i = strlen(local_buff);
    //         if (i < 62)
    //         {
    //             local_buff[i] = '\n';
    //             local_buff[i+1] = '\0';
    //             Usart3Send(local_buff);
    //         }
    //         L_ALARMA_OFF;
    //     }
    // }
    //---- Fin Prueba Usart3 loop en terminal ----------

    //---- Prueba Usart3 envia caracter solo 'd' ----------
    // while (1)
    // {
    //     unsigned char snd = 'd';
    //     Usart3SendUnsigned(&snd, 1);
    //     // USART3->DR = 'd';
    //     Wait_ms(100);
    // }
    //---- Fin Prueba Usart3 envia caracter solo 'd' ----------
}

//--- End of Main ---//

//--- Module Functions Definitions ----------------------
void TimingDelay_Decrement(void)
{
    if (wait_ms_var)
        wait_ms_var--;

    if (comms_timeout)
        comms_timeout--;
    
    if (timer_standby)
        timer_standby--;

    // if (timer_filters)
    //     timer_filters--;
    
    // if (timer_led)
    //     timer_led--;

    // if (timer_led_pwm < 0xFFFF)
    //     timer_led_pwm ++;
    
}

void EXTI0_IRQHandler (void)
{
    if(EXTI->PR & 0x00000001)	//Line0
    {
        if (LED)
            LED_OFF;
        else
            LED_ON;

        if (SENSE_MOSFET_A)
        {
            DisablePreload_MosfetA();
            UpdateTIM_MosfetA(0);
            EnablePreload_MosfetA();
            UpdateTIM_MosfetA(DUTY_FOR_DMAX);            
        }
        else if (SENSE_MOSFET_B)
        {
            DisablePreload_MosfetB();
            UpdateTIM_MosfetB(0);
            EnablePreload_MosfetB();
            UpdateTIM_MosfetB(DUTY_FOR_DMAX);
        }
        else
        {
            //llegue tarde
        }
        
        EXTI->PR |= 0x00000001;
    }
}

//--- end of file ---//

