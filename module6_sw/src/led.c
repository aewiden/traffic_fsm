/*
 * led.c -- Implementation of the LED module (led.h)
 * 			for a Zybo Z7 board with 4 onboard LEDs connected to AXI GPIO
 */

#include "led.h"

#define LED_CHANNEL 1
#define NUM_LEDS 4

#define LED4_PIN	7
#define PS_GPIO_DEVICE_ID XPAR_XGPIOPS_0_DEVICE_ID

static XGpioPs psGpio;

static XGpio ledPort;
static u32	ledStates = 0;

static XGpio led6Port;       // second AXIGPIO handle
static u32   led6_rgb_state; // bits for R/G/B

void led_init(void) {
    XGpio_Initialize(&ledPort, XPAR_AXI_GPIO_0_DEVICE_ID); /* Initialize GPIO */
    XGpio_SetDataDirection(&ledPort, LED_CHANNEL, 0x0); /* direction to output for all LEDs */
    XGpio_DiscreteWrite(&ledPort, LED_CHANNEL, 0x0);    /* Turn off all LEDs */

    XGpio_Initialize(&led6Port, XPAR_AXI_GPIO_3_DEVICE_ID);
    XGpio_SetDataDirection(&led6Port, 1, 0x0);
    XGpio_DiscreteWrite(&led6Port, 1, 0b000);
}

void led_set(u32 led, bool tostate) {
    if (led >= 0 && led <= 3) {
    	u32 currState = XGpio_DiscreteRead(&ledPort, 1);
    	if (tostate) {
    		currState |= (1 << led);
    	} else {
    		currState &= ~(1 << led);
    	}
    	XGpio_DiscreteWrite(&ledPort, LED_CHANNEL, currState);
    }
    else if (led == ROJO || led == VERDE || led == AZUL || led == AMAR) {
    	u32 colorVal = 0b000;
    	if (tostate) {
			switch (led) {
				case ROJO:
					colorVal = 0b100; //100
					break;
				case VERDE:
					colorVal = 0b010; //010
					break;
				case AZUL:
					colorVal = 0b001; //001
					break;
				case AMAR:
					colorVal = 0b110; //011
					break;
			}
    	}
    	XGpio_DiscreteWrite(&led6Port, 1, colorVal);
    }
}

bool led_get(u32 led) {
    if (led < NUM_LEDS) {
        return ( (ledStates & (1 << led)) != 0) ? LED_ON : LED_OFF;
    } else if ((led == ROJO) || (led == AZUL) || (led == VERDE) || (led == AMAR)) {
        u32 val = led6_rgb_state & 0x7;
        return (val == 0) ? LED_OFF : LED_ON;
    } else {
    	return LED_OFF;
    }
}

void led_toggle(u32 led) {
    if (led < NUM_LEDS) {
        ledStates ^= (1 << led);
        XGpio_DiscreteWrite(&ledPort, LED_CHANNEL, ledStates);
        return;
    }

    else if ((led == ROJO) || (led == VERDE) || (led == AZUL) || (led == AMAR)) {
            bool currentlyOn = led_get(led);
            if (currentlyOn) {
                led_set(led, LED_OFF);
            } else {
                led_set(led, LED_ON);
            }
    }
}

void led4_init(void)
{
    XGpioPs_Config *gpioConfigPtr;

    gpioConfigPtr = XGpioPs_LookupConfig(PS_GPIO_DEVICE_ID);
    if (gpioConfigPtr == NULL) {
        printf("ERROR: XGpioPs_LookupConfig failed for device ID %d\n", PS_GPIO_DEVICE_ID);
        return;
    }

    if (XGpioPs_CfgInitialize(&psGpio, gpioConfigPtr, gpioConfigPtr->BaseAddr) != XST_SUCCESS) {
        printf("ERROR: XGpioPs_CfgInitialize failed in led4_init()!\n");
        return;
    }

    // Set the pin direction to output
    XGpioPs_SetDirectionPin(&psGpio, LED4_PIN, 1);
    XGpioPs_SetOutputEnablePin(&psGpio, LED4_PIN, 1);

    // start LED4 off
    XGpioPs_WritePin(&psGpio, LED4_PIN, 0);
}

/* Turn LED4 ON */
void led4_on(void)
{
    XGpioPs_WritePin(&psGpio, LED4_PIN, 1);
}

/* Turn LED4 OFF */
void led4_off(void)
{
    XGpioPs_WritePin(&psGpio, LED4_PIN, 0);
}

