/*
 * servo.c
 *
 *  Uses:
 *  	XPAR_AXI_TIMER_0_DEVICE_ID -- the axi timer device id
 *
 * NOTE: This program assumes that xtmrctr.h and xtrctr_options.c have been
 * modified as per the assignment.
 */

#include "servo.h"

#define PERIOD 1000000             /* s_axi_aclk = 50MHz -- 20ms period = 1x10^6 * 1/(50*10^-9) */

#define MIN ((double)5.5)
#define MAX ((double)10.25)
#define MID ((double)7.5)		/* nominal midpoint */

static XTmrCtr tmr;


void servo_init(void) {
	u32 options;

	if(XTmrCtr_Initialize(&tmr,XPAR_AXI_TIMER_0_DEVICE_ID) != XST_SUCCESS) {
		printf("\n[ERROR: unable to initialize axi timer]\n");
		return;
	}

	XTmrCtr_Stop(&tmr,0);
	XTmrCtr_Stop(&tmr,1);

	options = XTC_EXT_COMPARE_OPTION | XTC_PWM_ENABLE_OPTION | XTC_DOWN_COUNT_OPTION;
	XTmrCtr_SetOptions(&tmr,0,options);
	XTmrCtr_SetOptions(&tmr,1,options);

	servo_set(MID);
}


void servo_set(double dutycycle) {
	u32 thigh;

	if(dutycycle < MIN) {
		dutycycle = MIN;
		printf("\n[ERROR: minimum limit exceeded]\n");
	}

	if(dutycycle > MAX) {
		dutycycle = MAX;
		printf("\n[ERROR: maximum limit exceeded]\n");
	}
	thigh = (u32)(dutycycle*(PERIOD/100));
	XTmrCtr_Stop(&tmr,0);
	XTmrCtr_Stop(&tmr,1);

	XTmrCtr_SetResetValue(&tmr,0,PERIOD);
	XTmrCtr_SetResetValue(&tmr,1,thigh);
	XTmrCtr_Start(&tmr,0);
	XTmrCtr_Start(&tmr,1);
}
