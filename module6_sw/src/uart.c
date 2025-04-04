//#include "uart.h"
//#include "gic.h"
//
//#define UART1_INT_ID  	XPAR_XUARTPS_1_INTR
//#define UART0_INT_ID  	XPAR_XUARTPS_0_INTR
//#define UART0_DEVICE_ID XPAR_PS7_UART_0_DEVICE_ID
//#define UART1_DEVICE_ID XPAR_PS7_UART_1_DEVICE_ID
//#define BUTTON3_MASK 	0x08  // Assuming Button 3 corresponds to bit 3
//#define UART_BAUD_RATE 115200
//
//
//void Uart0Handler(void *CallBackRef, u32 Event, u32 EventData, XUartPs *UartInst);
//void Uart1Handler(void *CallBackRef, u32 Event, u32 EventData, XUartPs *UartInst);
//void flush_uart(XUartPs *Uart);
//
//
//void flush_uart(XUartPs *UartInst) {
//    u8 temp_buffer[64];  // Temporary buffer for clearing data
//    while (XUartPs_Recv(UartInst, temp_buffer, sizeof(temp_buffer)) > 0);
//}
//
//void uart_init(XUartPs *UartInst, u16 DeviceId, XUartPs_Config *Config) {
//
//    XUartPs_LookupConfig(DeviceId);
//    XUartPs_CfgInitialize(UartInst, Config, Config->BaseAddress);
//    XUartPs_SetBaudRate(UartInst, UART_BAUD_RATE);
//
//}
//
//void uart_enable(XUartPs *UartInst, u32 IntId, void (*Handler)(void *, u32, u32)) {
//    // Enable UART Interrupts
//    XUartPs_SetInterruptMask(UartInst, XUARTPS_IXR_RXOVR);
//    XUartPs_SetFifoThreshold(UartInst, 1);
//    XUartPs_SetHandler(UartInst, Handler, (void *)UartInst);
//
//    // Connect UART interrupt to GIC
//    gic_connect(IntId, (Xil_InterruptHandler)XUartPs_InterruptHandler, UartInst);
//}
//
//void uart_close(XUartPs *UartInst, u32 IntId) {
//    gic_disconnect(IntId);
//}
//
//
//// UART0 Interrupt Handler - Handles received data for UART0
//void Uart0Handler(void *CallBackRef, u32 Event, u32 EventData, XUartPs *UartInst) {
//    XUartPs *UartPtr = (XUartPs *)CallBackRef;
//    char RecvChar;
//
//    if (Event == XUARTPS_EVENT_RECV_DATA) {
//        XUartPs_Recv(UartPtr, (u8 *)&RecvChar, 1);
//		if (RecvChar == '\r') {
//			char newline[] = "\r\n";
//			XUartPs_Send(UartInst, (u8 *)newline, 2);
//		} else {
//			XUartPs_Send(UartInst, (u8 *)&RecvChar, 1);
//		}
//        printf("[UART0] Received: %c\n", RecvChar);
//    }
//}
//
//// UART1 Interrupt Handler - Handles received data for UART1
//void Uart1Handler(void *CallBackRef, u32 Event, u32 EventData, XUartPs *UartInst) {
//    XUartPs *UartPtr = (XUartPs *)CallBackRef;
//    char RecvChar;
//
//    if (Event == XUARTPS_EVENT_RECV_DATA) {
//        XUartPs_Recv(UartPtr, (u8 *)&RecvChar, 1);
//        if (RecvChar == '\r') {
//			char newline[] = "\r\n";
//			XUartPs_Send(UartInst, (u8 *)newline, 2);
//		} else {
//			XUartPs_Send(UartInst, (u8 *)&RecvChar, 1);
//		}
//        printf("[UART1] Received: %c\n", RecvChar);
//    }
//}
