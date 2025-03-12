#include "adc.h"		/* types used by xilinx */

/*
 * initialize the adc module
 */
XAdcPs XAdcInst; // XADC instance

void adc_init(void){
	 XAdcPs_Config *ConfigPtr;
	      ConfigPtr = XAdcPs_LookupConfig(XPAR_XADCPS_0_DEVICE_ID);
	      XAdcPs_CfgInitialize(&XAdcInst, ConfigPtr, ConfigPtr->BaseAddress);
	      XAdcPs_SetSequencerMode(&XAdcInst, XADCPS_SEQ_MODE_CONTINPASS);
	      XAdcPs_SetSeqChEnables(&XAdcInst, XADCPS_SEQ_CH_AUX14 | XADCPS_SEQ_CH_TEMP | XADCPS_SEQ_CH_VCCINT);
	      XAdcPs_SetAlarmEnables(&XAdcInst, 0);
}

/*
 * get the internal temperature in degree's centigrade
 */
float adc_get_temp(void){
	 u32 raw_temp = XAdcPs_GetAdcData(&XAdcInst, XADCPS_CH_TEMP);
	 float temp = XAdcPs_RawToTemperature(raw_temp);
	 return temp;
}

/*
 * get the internal vcc voltage (should be ~1.0v)
 */
float adc_get_vccint(void){
	u32 raw_vccint = XAdcPs_GetAdcData(&XAdcInst, XADCPS_CH_VCCINT);
	float vccint = XAdcPs_RawToVoltage(raw_vccint);
	return vccint;

}

/*
 * get the **corrected** potentiometer voltage (should be between 0 and 1v)
 */
float adc_get_pot(void){
	u32 raw_pot = XAdcPs_GetAdcData(&XAdcInst, XADCPS_CH_AUX_MAX-1);
	float pot_value = XAdcPs_RawToVoltage(raw_pot);
	pot_value = pot_value/3;
	return pot_value;
}

