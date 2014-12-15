/*
 * PocsagEncoder.c
 *
 * Created: 16-9-2014 20:58:19
 *  Author: Jelmer Bruijn
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#include <util/delay.h>
#include <stdlib.h>

#include "Encoder.h"
#include "Pocsag.h"
#include "SI4432.h"
#include "Serial.h"
#include "Settings.h"

int main(void){
	DeviceState=STATE_NORMAL;
	SetupADC();
	SetupSecTimer();
	InitSerial();
	SetupSPI();
	sei();
	LoadSettings();
	while(1){
		
	}
}


void SetupSecTimer(){
	TCCR0B|=(1<<CS00)|(1<<CS02);
	TIMSK0|=(1<<TOIE0);
	seconds = 0;
}

void SetupADC(){
	ADCSRA|=(1<<ADEN)|(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2);
	ADMUX|=(1<<REFS0)|(1<<REFS1);
	ADCSRA|=(1<<ADSC);
}

void EverySecond(){
	ADCval = (uint8_t)(ADC>>2)&0xFF;
	ADCSRA|=(1<<ADSC);
	if((DeviceState==STATE_NORMAL)||(DeviceState==STATE_SERVER_FAIL_SENT)){
		if(ADCval<=ADCThreshold){
			if(PowerLossMsgSet()){
				sei();
				while(xmitState!=STATE_OFF)
				;
				LoadSettings();
				LoadPowerLossMsg();
				DeviceState=STATE_POWER_LOSS_SENT;
				StartXmit();
			}
		}
		if((DeviceState==STATE_NORMAL)&&(seconds>WATCHDOG_MAX)){
			if(ServerFailMsgSet()){
				sei();
				while(xmitState!=STATE_OFF)
				;
				LoadSettings();
				LoadServerFailMsg();
				DeviceState=STATE_SERVER_FAIL_SENT;
				StartXmit();
			}
		}


	} else if(DeviceState==STATE_POWER_LOSS_SENT){
		if(ADCval>ADCThreshold){
			DeviceState=STATE_NORMAL;
		}
	}
}
ISR(TIMER0_OVF_vect){
	subseconds++;
	if(subseconds==62){
		subseconds=0;
		seconds++;
		sei();
		EverySecond();
	}
}
