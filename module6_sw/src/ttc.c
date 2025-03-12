#include "ttc.h"
#include "gic.h"

static XTtcPs ttc_instance;
static void (*ttc_callback_func)(void);

void ttc_handler(void *callback_ref) {
    XTtcPs *ttc_instance = (XTtcPs *)callback_ref;

    XTtcPs_ClearInterruptStatus(ttc_instance, XTtcPs_GetInterruptStatus(ttc_instance));

    if (ttc_callback_func) {
        ttc_callback_func();
    }
}


void ttc_init(u32 freq, void (*ttc_callback)(void)) {
    XTtcPs_Config *ttc_config;
    u8 prescaler;
    XInterval interval;

    ttc_callback_func = ttc_callback;

    // TTC configuration
    ttc_config = XTtcPs_LookupConfig(XPAR_XTTCPS_0_DEVICE_ID);
    if (ttc_config == NULL) {
        return;
    }

    if (XTtcPs_CfgInitialize(&ttc_instance, ttc_config, ttc_config->BaseAddress) != XST_SUCCESS) {
        return;
    }

    // interval and prescaler for the desired frequency
    XTtcPs_CalcIntervalFromFreq(&ttc_instance, freq, &interval, &prescaler);
    XTtcPs_SetPrescaler(&ttc_instance, prescaler);
    XTtcPs_SetInterval(&ttc_instance, interval);

    // TTC options for interval mode
    XTtcPs_SetOptions(&ttc_instance, XTTCPS_OPTION_INTERVAL_MODE);

    // interrupt handler
    if (gic_connect(XPAR_XTTCPS_0_INTR, (Xil_InterruptHandler)ttc_handler, &ttc_instance) != XST_SUCCESS) {
        return;
    }

    // TTC interrupts
    XTtcPs_EnableInterrupts(&ttc_instance, XTTCPS_IXR_INTERVAL_MASK);
}

void ttc_start(void) {
    XTtcPs_Start(&ttc_instance);
}

void ttc_stop(void) {
    XTtcPs_Stop(&ttc_instance);
}

void ttc_close(void) {
    gic_disconnect(XPAR_XTTCPS_0_INTR);
    XTtcPs_Stop(&ttc_instance);
}
