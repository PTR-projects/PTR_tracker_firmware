/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include <stdint.h>
#include "trackerHw.h"
#include "gps.h"
#include "telemetryPacket.h"
#include "config.h"

volatile char uartReceivedByte = '\0';
DataPackageRF_t telemetryPacket;

uint8_t vbat;

void read_analogSensors(uint8_t *voltage) {
	*voltage = (uint32_t)((HW_readADC(3)*1000 / 4095 * 33 * 3) / 1000); //Returns battery voltage * 10
}

void pack_data() {
	read_analogSensors(&vbat);
	telemetryPacket.vbat_10 = vbat;
	telemetryPacket.packet_id = 0x00AA; //We specify what type of frame we're sending, in this case the big 48 byte struct
	telemetryPacket.lat = GPS_lat;
	telemetryPacket.lon = GPS_lon;
	telemetryPacket.alti_gps = GPS_alt * 1000; //To mm
	telemetryPacket.sats_fix = ((GPS_fix & 3) << 6) | (GPS_sat_count & 0x3F);
}

void send_data_lora(uint8_t* data) {
	HW_writeLED(1);
	RADIO_sendPacketLoRa(data, sizeof(DataPackageRF_t), 500);
	HW_DelayMs(50);
	HW_writeLED(0);
}

int main(void)
{
	HW_trackerHwInit();
	RADIO_init();
	RADIO_modeLORA(TRACKER_FREQUENCY_0, TRACKER_TXPOWER_LOW);

	while(1) {
		if(1==1/*GPS_sat_count > 2*/) {
			pack_data();
			send_data_lora(&telemetryPacket);
			HW_DelayMs(TRACKER_TRANSMISSION_SPACING);
		}
	}
}

void USART1_IRQHandler(void) {
	if(USART1->ISR & USART_ISR_RXNE_RXFNE) {
		uartReceivedByte = USART1->RDR;
		GPS_parse(uartReceivedByte);
	}
}
