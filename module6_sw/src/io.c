#include "io.h"
#include "gic.h"

/* Static variables for internal state */
static XGpio btnport;
static XGpio swport;
static void (*btn_callback)(u32 btn);
static void (*sw_callback)(u32 sw);
static u32 btn_prev_state = 0;
static u32 sw_prev_state = 0;

void btn_handler(void *devicep) {
    XGpio *dev = (XGpio *)devicep;

    u32 btn_value = XGpio_DiscreteRead(dev, 1);
    u32 btn_pressed = btn_value & ~btn_prev_state;

    if (btn_pressed && btn_callback) {
        btn_callback(btn_pressed);
    }

    btn_prev_state = btn_value;
    XGpio_InterruptClear(dev, XGPIO_IR_CH1_MASK);
}

void sw_handler(void *devicep) {
    XGpio *dev = (XGpio *)devicep;

    u32 sw_value = XGpio_DiscreteRead(dev, 1);
    u32 sw_changed = sw_value ^ sw_prev_state;

    if (sw_changed && sw_callback) {
        sw_callback(sw_changed);
    }

    sw_prev_state = sw_value;
    XGpio_InterruptClear(dev, XGPIO_IR_CH1_MASK);
}

void io_btn_init(void (*callback)(u32 btn)) {
    btn_callback = callback;

    if (XGpio_Initialize(&btnport, XPAR_AXI_GPIO_1_DEVICE_ID) != XST_SUCCESS) {
        return;
    }
    XGpio_SetDataDirection(&btnport, 1, 0xFF);

    // Disable interrupts
    XGpio_InterruptDisable(&btnport, XGPIO_IR_CH1_MASK);
    XGpio_InterruptClear(&btnport, XGPIO_IR_CH1_MASK);
    XGpio_InterruptGlobalEnable(&btnport);

    if (gic_connect(XPAR_FABRIC_GPIO_1_VEC_ID, btn_handler, &btnport) != XST_SUCCESS) {
        return;
    }

    // Enable button interrupts
    XGpio_InterruptEnable(&btnport, XGPIO_IR_CH1_MASK);
}

void io_btn_close(void) {
    // Disconnect the interrupt handler
    gic_disconnect(XPAR_FABRIC_GPIO_1_VEC_ID);

    // Disable and clear button interrupts
    XGpio_InterruptDisable(&btnport, XGPIO_IR_CH1_MASK);
    XGpio_InterruptClear(&btnport, XGPIO_IR_CH1_MASK);
}

void io_sw_init(void (*callback)(u32 sw)) {

    sw_callback = callback;
    if (XGpio_Initialize(&swport, XPAR_AXI_GPIO_2_DEVICE_ID) != XST_SUCCESS) {
        return;
    }
    XGpio_SetDataDirection(&swport, 1, 0xFF);


    XGpio_InterruptDisable(&swport, XGPIO_IR_CH1_MASK);
    XGpio_InterruptClear(&swport, XGPIO_IR_CH1_MASK);
    XGpio_InterruptGlobalEnable(&swport);

    if (gic_connect(XPAR_FABRIC_GPIO_2_VEC_ID, sw_handler, &swport) != XST_SUCCESS) {
        return;
    }

    XGpio_InterruptEnable(&swport, XGPIO_IR_CH1_MASK);
}

void io_sw_close(void) {
    // Disconnect the switch interrupt handler
    gic_disconnect(XPAR_FABRIC_GPIO_2_VEC_ID);

    // Disable and clear switch interrupts
    XGpio_InterruptDisable(&swport, XGPIO_IR_CH1_MASK);
    XGpio_InterruptClear(&swport, XGPIO_IR_CH1_MASK);
}
