/*
 * module6.c ---
 *
 *
 */
#include <stdio.h>
#include "xil_printf.h"
#include "xparameters.h"
#include "servo.h"
#include "adc.h"
#include "led.h"
#include "io.h"
#include "gic.h"
#include "ttc.h"
//#include "substation.c"

// Hardware Constants
/* led colors */
#define RED 5
#define BLUE 6
#define GREEN 7
#define YELLOW 8
#define PED_LED   4
#define PING 1
#define UPDATE 2

#define MIN ((double)5.5)
#define MAX ((double)10.25)
#define MID ((double)7.5)
#define MIN_GREEN_TICKS 100
#define YELLOW_TICKS    30
#define PED_RED_TICKS   100
#define RED_LIGHT_TICKS 30

#define POLLING_INTERVAL 100000 // 100 milliseconds (10 times per second)
#define MAX_CROSSINGS 10

// FSM States
typedef enum {
    RED_LIGHT,
    YELLOW_LIGHT1,
    GREEN_LIGHT,
    YELLOW_LIGHT2,
    TRAIN_CLOSING,
    TRAIN_CLOSED,
    TRAIN_OPENING,
    TRAIN_WAIT_PED,
    MAINTENANCE
} SystemState;

typedef enum {
    REQUEST_UPDATE,
    REQUEST_ENABLE_MAINTENANCE,
    REQUEST_DISABLE_MAINTENANCE
} SubstationRequestType;


static const char *state_names[] = {
    "RED_LIGHT", "YELLOW_LIGHT1", "GREEN_LIGHT",
    "YELLOW_LIGHT2", "TRAIN_CLOSING", "TRAIN_CLOSED",
    "TRAIN_OPENING", "TRAIN_WAIT_PED", "MAINTENANCE"
};

typedef struct {
    int id;
    int status; // 0 = inactive, 1 = active
} CrossingStatus;

typedef struct {
    CrossingStatus crossings[MAX_CROSSINGS];
    bool maintenance_mode;
} Substation;

typedef struct{
	int type;
	int id;
} ping_t;

typedef struct {
int type; /* must be assigned to UPDATE */
int id;
/* must be assigned to your id */
int value; /* must be assigned to some value */
} update_request_t;

typedef struct {
int type;
int id;
int average;
int values[30];
} update_response_t;



// Global Variables
static volatile SystemState current_state = RED_LIGHT;
static volatile bool pedestrian_request = false;
static volatile bool train_arriving = false;
static volatile bool maintenance_active = false;
static volatile unsigned int fsm_tick_count = 0;
static double current_servo_duty = 7.5;
static bool done = false;


#define UART1_INT_ID  	XPAR_XUARTPS_1_INTR
#define UART0_INT_ID  	XPAR_XUARTPS_0_INTR
#define BUTTON3_MASK 	0x08  // Assuming Button 3 corresponds to bit 3

//#define CONFIGURE 0
//#define PING 1
//#define UPDATE 2
//#define OFF 3

static XUartPs UartInst1;  // UART1 (Receiving)
static XUartPs UartInst0;  // UART0 (WiFly module - Forwarding)
static bool done;
int Status;

XUartPs_Config *Config;



//void send_request(SubstationRequestType type) {
//    switch (type) {
//        case REQUEST_UPDATE:
//            printf("Sending UPDATE request...\n");
//            break;
//        case REQUEST_ENABLE_MAINTENANCE:
//            printf("Sending ENABLE MAINTENANCE request...\n");
//            substation.maintenance_mode = true;
//            break;
//        case REQUEST_DISABLE_MAINTENANCE:
//            printf("Sending DISABLE MAINTENANCE request...\n");
//            substation.maintenance_mode = false;
//            break;
//    }
//}
//
//void process_response(Substation *substation) {
//    for (int i = 0; i < MAX_CROSSINGS; i++) {
//        substation->crossings[i].id = i;
//        substation->crossings[i].status = rand() % 2; // Simulate active/inactive status
//    }
//}
//
//void *poll_substation_status(void *arg) {
//    while (1) {
//        send_request(REQUEST_UPDATE);
//        process_response(&substation);
//        print_crossing_statuses();
//        usleep(POLLING_INTERVAL);
//    }
//}


// UART0 Interrupt Handler - Forwards received data to UART1
void Uart0Handler(void *CallBackRef, u32 Event, u32 EventData) {
	XUartPs *UartPtr = (XUartPs *)CallBackRef;
	char RecvChar;

//	if (Event == XUARTPS_EVENT_RECV_DATA && current_mode == CONFIGURE) {
//    	XUartPs_Recv(UartPtr, (u8 *)&RecvChar, 1);
//    	// If Enter is pressed, send both \r and \n
//    	if (RecvChar == '\r') {
//        	char newline[] = "\r\n";
//        	XUartPs_Send(&UartInst1, (u8 *)newline, 2);
//    	} else {
//        	XUartPs_Send(&UartInst1, (u8 *)&RecvChar, 1);
//    	}
//	}
}


// UART1 Interrupt Handler - Forwards received data to UART0
void Uart1Handler(void *CallBackRef, u32 Event, u32 EventData) {
	XUartPs *UartPtr = (XUartPs *)CallBackRef;
	char RecvChar;

//	if (Event == XUARTPS_EVENT_RECV_DATA && current_mode == CONFIGURE) {
//    	XUartPs_Recv(UartPtr, (u8 *)&RecvChar, 1);
//    	// If Enter is pressed, send both \r and \n
//    	if (RecvChar == '\r') {
//        	char newline[] = "\r";
//        	XUartPs_Send(&UartInst0, (u8 *)newline, 2);
//    	} else {
//        	XUartPs_Send(&UartInst0, (u8 *)&RecvChar, 1);
//    	}
//	}
 }


// TTC Callback (10Hz = 100ms ticks)
void fsm_ttc_callback(void) {
    fsm_tick_count++;
}

// Button Callback
void btn_callback(unsigned int btn) {
    if (btn & (1 << 0)) {
               pedestrian_request = true;
               printf("\n\r[INPUT] Pedestrian request\n\r");
   }
           if (btn & (1 << 2)) {
               pedestrian_request = true;
               printf("\n\r[INPUT] Pedestrian request\n\r");
           }
           if (btn & (1 << 3)) {
               done = true;
           }
}

// Switch Callback
void sw_callback(unsigned int sw) {
    // SW0 (bit 0) controls train arrival/clear
    bool new_train = (sw & 0x1);
    // SW1 (bit 1) controls maintenance mode
    bool new_maintenance = (sw & 0x2);

    if(new_train != train_arriving || new_maintenance != maintenance_active) {
        printf("\n\r[INPUT] Train: %s | Maintenance: %s\n\r",
               train_arriving ? "ARRIVING" : "CLEAR",
               maintenance_active ? "ON" : "OFF");
    }
    if (sw & (1 << 0)) {
           train_arriving = !train_arriving;
        }
    if (sw & (1 << 1)) {
           maintenance_active = !maintenance_active;
        }
}

// LED Control
void setTrafficLED(u32 color) {
    led_set(color, LED_ON);
}

// Hardware Initialization
void hardware_init() {
    gic_init();
    led_init();
    servo_init();
    adc_init();
    io_btn_init(btn_callback);
    io_sw_init(sw_callback);
    ttc_init(10, fsm_ttc_callback);
}

// Status Display
void update_display() {
    printf("\r%-15s | Gate: %-6s | Train: %-8s | Ped: %s \n",
           state_names[current_state],
           (current_servo_duty > 7.5) ? "OPEN" : "CLOSED",
           train_arriving ? "ARRIVING" : "CLEAR",
           ((current_state == RED_LIGHT && pedestrian_request) ||
            (current_state >= TRAIN_CLOSING && current_state <= TRAIN_WAIT_PED) ||
            (current_state == MAINTENANCE)) ? "WALK" : "STOP",
           fsm_tick_count);
    fflush(stdout);
}

// Main FSM
void run_fsm() {
	static unsigned int flash_timer = 0;
    static bool blue_led_state = false;
    static SystemState prev_state = MAINTENANCE;
    float pot_val = adc_get_pot();


    if(prev_state != current_state) {
        printf("\n\r\n\r");
        prev_state = current_state;
    }

    if(maintenance_active && current_state != MAINTENANCE) {
        current_state = MAINTENANCE;
        fsm_tick_count = 0;
        current_servo_duty = MIN;
        servo_set(MIN);
    }
    else if(train_arriving && current_state < TRAIN_CLOSING) {
        current_state = TRAIN_CLOSING;
        fsm_tick_count = 0;
    }

    switch(current_state) {
        case RED_LIGHT: {
            unsigned int red_duration = pedestrian_request ? PED_RED_TICKS : RED_LIGHT_TICKS;

            setTrafficLED(RED);
            current_servo_duty = MAX; // Gate open
            servo_set(MAX);
            led_set(PED_LED, pedestrian_request);

            if(fsm_tick_count >= red_duration) {
                if(pedestrian_request) {
                    pedestrian_request = false;
                    led_set(PED_LED, false);
                }
                current_state = YELLOW_LIGHT1;
                fsm_tick_count = 0;
            }
            break;
        }

        case YELLOW_LIGHT1:
            setTrafficLED(YELLOW);
            if(fsm_tick_count >= YELLOW_TICKS) {
                current_state = GREEN_LIGHT;
                fsm_tick_count = 0;
            }
            break;

        case GREEN_LIGHT:
            setTrafficLED(GREEN);
            current_servo_duty = MAX;
            servo_set(MAX);

            if(fsm_tick_count >= MIN_GREEN_TICKS) {
                current_state = YELLOW_LIGHT2;
                fsm_tick_count = 0;
            }
            break;

        case YELLOW_LIGHT2:
            setTrafficLED(YELLOW);
            if(fsm_tick_count >= YELLOW_TICKS) {
                current_state = RED_LIGHT;
                fsm_tick_count = 0;
            }
            break;

        case TRAIN_CLOSING:
            setTrafficLED(RED);
            current_servo_duty = MIN; // Close gate
            servo_set(MIN);
            current_state = TRAIN_CLOSED;
            fsm_tick_count = 0;
            break;

        case TRAIN_CLOSED:
            led_set(PED_LED, true);
            if(!train_arriving) {
                current_state = TRAIN_OPENING;
                fsm_tick_count = 0;
            }
            break;

        case TRAIN_OPENING:
            current_servo_duty = MAX; // Open gate
            servo_set(MAX);
            current_state = TRAIN_WAIT_PED;
            fsm_tick_count = 0;
            break;

        case TRAIN_WAIT_PED:
            led_set(PED_LED, true);
            if(fsm_tick_count >= PED_RED_TICKS) {
                current_state = YELLOW_LIGHT1;
                led_set(PED_LED, false);
                fsm_tick_count = 0;
            }
            break;

        case MAINTENANCE:
            led_set(ALL, LED_OFF);
            led_set(PED_LED, true);

            // First handle forced close on entry
            if(fsm_tick_count == 0) {
                current_servo_duty = MIN;
                servo_set(MIN);
            }
            // Then allow manual control
            else {
                current_servo_duty = (pot_val * (MAX - MIN)) + MIN;
                servo_set(current_servo_duty);
            }

            if(!maintenance_active) {
                current_state = RED_LIGHT;
                led_set(PED_LED, false);
                fsm_tick_count = 0;
            }
            break;
    }
    if(maintenance_active && flash_timer % 10 == 0) {
        led_set(ALL, LED_OFF);
        led_set(RED, LED_OFF);
        led_set(GREEN, LED_OFF);
        led_set(BLUE, blue_led_state);
        blue_led_state = !blue_led_state;
        flash_timer = 0;
     }
    update_display();
    flash_timer++;
}

//void send_update_request() {
//    printf("Sending UPDATE request with id = 0...\n");
//    // Simulate sending an UPDATE message to the remote substation database
//}
//
//void process_update_response() {
//    // Simulate response with random crossing statuses
//    for (int i = 0; i < MAX_CROSSINGS; i++) {
//        substation.crossings[i].id = i;
//        substation.crossings[i].status = rand() % 2; // Simulating active/inactive status
//    }
//}
//
//void print_crossing_statuses() {
//    printf("Crossing Statuses:\n");
//    for (int i = 0; i < MAX_CROSSINGS; i++) {
//        printf("Crossing %d: %s\n", substation.crossings[i].id, substation.crossings[i].status ? "Active" : "Inactive");
//    }
//}
//
//void enable_maintenance_mode(int enable) {
//    substation.maintenance_mode = enable;
//    printf("Maintenance mode %s\n", enable ? "ENABLED" : "DISABLED");
//}

//void poll_substation_status() {
//    while (1) {
//        send_update_request();
//        process_update_response();
//        print_crossing_statuses();
//        usleep(POLLING_INTERVAL); // Sleep for 100ms before the next poll
//    }
//}

//void set_maintenance_mode(bool enable) {
//    send_request(enable ? REQUEST_ENABLE_MAINTENANCE : REQUEST_DISABLE_MAINTENANCE);
//}


int main() {
    hardware_init();
    ttc_start();
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("\n\r[initialized]\n\r");
    printf("Switch 0: Train Control | Switch 1: Maintenance Mode\n\r");
    printf("Normal sequence: GREEN (10s) → YELLOW (3s) → RED (3s/10s)\n\r");
    // Lookup UART1 (Receiving)
    	Config = XUartPs_LookupConfig(XPAR_PS7_UART_1_DEVICE_ID);
    	if (!Config) {
        	printf("UART1 Lookup Failed: Config is NULL\n");
        	return XST_FAILURE;
    	}

    	Status = XUartPs_CfgInitialize(&UartInst1, Config, Config->BaseAddress);
    	if (Status != XST_SUCCESS) {
        	printf("UART1 Initialization Failed! Status: %d\n", Status);
        	return XST_FAILURE;
    	}

    	// Lookup UART0 (Forwarding to WiFly)
    	Config = XUartPs_LookupConfig(XPAR_PS7_UART_0_DEVICE_ID);
    	if (!Config) {
        	printf("UART0 Lookup Failed: Config is NULL\n");
        	return XST_FAILURE;
    	}

    	Status = XUartPs_CfgInitialize(&UartInst0, Config, Config->BaseAddress);
    	if (Status != XST_SUCCESS) {
        	printf("UART0 Initialization Failed! Status: %d\n", Status);
        	return XST_FAILURE;
    	}

    	// Set baud rates
    	Status = XUartPs_SetBaudRate(&UartInst1, 115200);
    	if (Status != XST_SUCCESS) {
        	printf("Setting Baud Rate for UART1 Failed! Status: %d\n", Status);
        	return XST_FAILURE;
    	}

    	Status = XUartPs_SetBaudRate(&UartInst0, 9600);
    	if (Status != XST_SUCCESS) {
        	printf("Setting Baud Rate for UART0 Failed! Status: %d\n", Status);
        	return XST_FAILURE;
    	}
    	printf("[hello]\n");

    	// Enable UART1 Interrupts
    	XUartPs_SetInterruptMask(&UartInst1, XUARTPS_IXR_RXOVR);
    	printf("UART1 Interrupt Mask Set\n");

    	XUartPs_SetFifoThreshold(&UartInst1, 1);
    	printf("UART1 FIFO Threshold Set\n");

    	XUartPs_SetHandler(&UartInst1, Uart1Handler, (void *)&UartInst1);
    	printf("UART1 Handler Set\n");

    	// Connect UART1 interrupt to GIC
    	gic_connect(UART1_INT_ID, (Xil_InterruptHandler)XUartPs_InterruptHandler, &UartInst1);
    	printf("UART1 Interrupt Connected\n");

    	// Enable UART0 Interrupts
    	XUartPs_SetInterruptMask(&UartInst0, XUARTPS_IXR_RXOVR);
    	printf("UART0 Interrupt Mask Set\n");

    	XUartPs_SetFifoThreshold(&UartInst0, 1);
    	printf("UART0 FIFO Threshold Set\n");

    	XUartPs_SetHandler(&UartInst0, Uart0Handler, (void *)&UartInst0);
    	printf("UART0 Handler Set\n");

    	// Connect UART0 interrupt to GIC
    	gic_connect(UART0_INT_ID, (Xil_InterruptHandler)XUartPs_InterruptHandler, &UartInst0);
    	printf("UART0 Interrupt Connected\n");


    
    while(!done) {

        run_fsm();
    	printf("\n[UPDATE]\n");


        	update_request_t update_msg;
        	update_msg.type = UPDATE;
        	update_msg.id = 0;
//        	update_msg.value = pot_percentage;

//        	printf("[UPDATE] Sending update message (ID: %d, Value: %d)\n",
//               	update_msg.id, update_msg.value);
        	XUartPs_Send(&UartInst0, (u8 *)&update_msg, sizeof(update_request_t));

        	usleep(50000);
        // Receive response
		update_response_t resp;
		int bytes_r = 0;
		while (bytes_r < sizeof(update_response_t)) {
			bytes_r += XUartPs_Recv(&UartInst0,
					(u8 *)&resp + bytes_r,
					sizeof(update_response_t) - bytes_r);
		}
		printf("response: %d,\n", resp.type);

		if (resp.type == UPDATE) {
			printf("[UPDATE] Received valid response from server:\n");
			printf("Last update values:\n");
			for (int j = 0; j < 30; j++) {
				printf("Device %d: %d\n", j, resp.values[j]);

			}
		} else {
			printf("[UPDATE] Invalid response received\n");
		}
		if(resp.values[27]==1){
			//send to maintenance mode
			maintenance_active = true;

		}
		else if(resp.values[27]==-1){
			//leave maintenance mode
			maintenance_active = false;
		}


        usleep(50000);


    }

    printf("\n\r[shutdown]\n\r");
    return 0;
}
