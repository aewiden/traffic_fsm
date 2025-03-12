/*
 * uart.h
 *
 * NOTE: Check where STD out is going.
 *
 */
#pragma once

#include <stdio.h>    	/* getchar, printf */
#include <stdlib.h>    	/* strtod */
#include <stdbool.h>    /* type bool */
#include <string.h>
#include "xparameters.h"
#include "xuartps.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include "xscugic.h"
#include "gic.h"

/*
 * uart_init -- initialize the UART module
 */
void uart_init(XUartPs *UartInst, u16 DeviceId, XUartPs_Config *Config);

/*
 * uart_enable -- enable UART interrupts
 */
void uart_enable(XUartPs *UartInst, u32 IntId, void (*Handler)(void *, u32, u32));

/*
 * uart_close -- stop UART communication
 */
void uart_close(XUartPs *UartInst, u32 IntId);

/*
 * flush_uart -- clear UART receive buffer
 */
void flush_uart(XUartPs *UartInst);
