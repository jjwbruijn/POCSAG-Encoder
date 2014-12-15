/*
 * Serial.c
 *
 * Created: 25-11-2014 22:28:48
 *  Author: Jelmer Bruijn
 */ 

#include "Serial.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <stdlib.h>
#include "Pocsag.h"
#include "Encoder.h"
#include "Settings.h"
#include "SI4432.h"

#define XMIT_START 0
#define XMIT_CRLF 1
#define XMIT_END 3
#define XMIT_COMPLETE 4
volatile uint8_t serialxmitstate;
volatile uint8_t escapenext;



void InitSerial(void){
	UBRR0 = 8;
	serialxmitstate = XMIT_COMPLETE;
	escapenext = 0;
	UCSR0B = (1<<RXCIE0)|(1<<TXCIE0)|(1<<RXEN0)|(1<<TXEN0);
	strcpy_P(message,PSTR("POCSAG Encoder by Jelmer Bruijn 2014\0"));
	SerialXmitStart();
}

ISR(USART_TX_vect){
	UCSR0A|=(1<<TXC0);
	switch(serialxmitstate){
		case XMIT_START:
			if(*bufferp==0x00){
				serialxmitstate = XMIT_CRLF;
				UDR0 = 13;
			} else {
				UDR0 = *bufferp;
				bufferp++;
			}
			break;
		case XMIT_CRLF:
			UDR0 = 10;
			serialxmitstate = XMIT_END;
			break;
		case XMIT_END:
			serialxmitstate = XMIT_COMPLETE;
			bufferp = &(message[0]);
			SerialRXEnable();
			break;
		case XMIT_COMPLETE:
			break;
		}

}

ISR(USART_RX_vect){
	*bufferp = UDR0;
	if(escapenext){
		switch(*bufferp){
			case 0x6e: // newline
				*bufferp = 0x0A;
				break;
		}
		escapenext = 0;

		bufferp++;
	} else {
		if(*bufferp==0x5c){
			escapenext = 1;
		} else {
				bufferp++;
			if(*(bufferp-1)==13){
			*(bufferp-1)=0x00;
			} else if(*(bufferp-1)==10){
			*(bufferp-1)=0x00;
			//*bufferp=0x00;
				ProcessSerialData();
			}
		}

	}
	if(bufferp>(&(message[MSG_BUFFER_LEN])-2))
	bufferp--;

}

void SerialRXDisable(){
	UCSR0B&=~(1<<RXCIE0);
}

void SerialRXEnable(){
	UCSR0B|=(1<<RXCIE0);
}

void ProcessSerialData(){
	SerialRXDisable();
	seconds = 0;
	if(strncmp_P(message,PSTR("MSG:"),4)==0){
		strcpy(message,message+4);
		StartXmit();
	} else if(strncmp_P(message,PSTR("RIC:"),4)==0){
		pagerric = strtol(message+4,NULL,10);
		strcpy_P(message,PSTR("OK\0"));
		SerialXmitStart();
	} else if(strncmp_P(message,PSTR("FUNC:"),5)==0){
		functiontype = strtol(message+5,NULL,10);
		strcpy_P(message,PSTR("OK\0"));
		SerialXmitStart();
	} else if(strncmp_P(message,PSTR("TYPE:"),5)==0){
		messagetype = strtol(message+5,NULL,10);
		strcpy_P(message,PSTR("OK\0"));
		SerialXmitStart();
	} else if(strncmp_P(message,PSTR("BAUD:"),5)==0){
		baudrate = strtol(message+5,NULL,10);
		strcpy_P(message,PSTR("OK\0"));
		SerialXmitStart();
	} else if(strncmp_P(message,PSTR("OFFSET:"),7)==0){
		offset = strtol(message+7,NULL,10);
		SetupFreq();
		strcpy_P(message,PSTR("OK\0"));
		SerialXmitStart();
	} else if(strncmp_P(message,PSTR("PING"),4)==0){
		strcpy_P(message,PSTR("PONG\0"));
		SerialXmitStartWait();
		if((DeviceState==STATE_SERVER_FAIL_SENT)&&(ServerResumeMsgSet())){
			while(xmitState!=STATE_OFF)
			;
			LoadSettings();
			LoadServerResumeMsg();
			DeviceState=STATE_NORMAL;
			StartXmit();
		}
	} else if(strncmp_P(message,PSTR("FREQ:"),5)==0){
		xmitfreq = strtol(message+5,NULL,16);
		strcpy_P(message,PSTR("OK\0"));
		SetupFreq();
		SerialXmitStart();
	} else if(strncmp_P(message,PSTR("SAVE"),4)==0){
		SaveSettings();
		strcpy_P(message,PSTR("OK\0"));
		SerialXmitStart();
	} else if(strncmp_P(message,PSTR("GETVOLT"),7)==0){
		strcpy_P(message,PSTR("ADC VALUE=       \0"));
		ltoa(ADCval,&(message[10]),10);
		SerialXmitStart();
	} else if(strncmp_P(message,PSTR("SETVOLT:"),8)==0){
		ADCThreshold = strtol(message+8,NULL,10);
		SaveThreshold();
		strcpy_P(message,PSTR("OK\0"));
		SerialXmitStart();
	} else if(strncmp_P(message,PSTR("PWRLOSSMSG:"),11)==0){
		strcpy(message,(&message[11]));
		SavePowerLossMsg();
		strcpy_P(message,PSTR("OK\0"));
		SerialXmitStart();
	} else if(strncmp_P(message,PSTR("SERVERFAILMSG:"),14)==0){
		strcpy(message,(&message[14]));
		SaveServerFailMsg();
		strcpy_P(message,PSTR("OK\0"));
		SerialXmitStart();
	} else if(strncmp_P(message,PSTR("SERVERRESUMEMSG:"),16)==0){
		strcpy(message,(&message[16]));
		SaveServerResumeMsg();
		strcpy_P(message,PSTR("OK\0"));
		SerialXmitStart();
	} else if(strncmp_P(message,PSTR("TEST"),4)==0){
		switch(message[4]){
			case 0x31:
				LoadSettings();
				LoadPowerLossMsg();
				StartXmit();
				break;
			case 0x32:
				LoadSettings();
				LoadServerFailMsg();
				StartXmit();
				break;
			case 0x33:
				LoadSettings();
				LoadServerResumeMsg();
				StartXmit();
				break;
			default:
				strcpy_P(message,PSTR("???\0"));
				SerialXmitStart();
		}
		
	} else if(strncmp_P(message,PSTR("HELP"),4)==0){
		strcpy_P(message,PSTR("***\0"));
		SerialXmitStartWait();
		strcpy_P(message,PSTR("- PING Sends back a pong! Also resets the watchdog.\0"));
		SerialXmitStartWait();
		strcpy_P(message,PSTR("- MSG: Sends a message to configured RIC and settings\0"));
		SerialXmitStartWait();
		strcpy_P(message,PSTR("- RIC: Sets the RIC to be used for sending the message\0"));
		SerialXmitStartWait();
		strcpy_P(message,PSTR("- TYPE: Sets pager type. 0 for numeric, 3 for alpha. 1 and 2 are tone only.\0"));
		SerialXmitStartWait();
		strcpy_P(message,PSTR("- FUNC: Sets the function group. Sets tone cadence on some pagers, or mailbox.\0"));
		SerialXmitStartWait();
		strcpy_P(message,PSTR("- BAUD: Sets baudrate\0"));
		SerialXmitStartWait();
		strcpy_P(message,PSTR("- OFFSET: Sets TX offset\0"));
		SerialXmitStartWait();
		strcpy_P(message,PSTR("- FREQ: Sets frequency. Expects something like 0x5616F8 for 460,91875Mhz. Can be calculated using some SI excel sheet.\0"));
		SerialXmitStartWait();
		strcpy_P(message,PSTR("- PWRLOSSMSG: Sets the message that is sent to the configured pager during an outage. Set empty if you don't want a message to be sent. Max 160 chars\0"));
		SerialXmitStartWait();
		strcpy_P(message,PSTR("- SERVERFAILMSG: Sets the message that is sent to the pager if nothing is heard from the server for 5 minutes. Set empty if you don't want a message to be sent. Max 160 chars\0"));
		SerialXmitStartWait();
		strcpy_P(message,PSTR("- SERVERRESUMEMSG: Sets the message that is sent if the server resumes checking in. Set empty if you don't want a message to be sent. Max 160 chars\0"));
		SerialXmitStartWait();	
		strcpy_P(message,PSTR("- GETVOLT Displays the AD output for the supply voltage\0"));
		SerialXmitStartWait();
		strcpy_P(message,PSTR("- SETVOLT: Sets threshold value for power loss detection\0"));
		SerialXmitStartWait();
		strcpy_P(message,PSTR("- SAVE Saves current RIC, type, func, frequency and offset for sending automatic alerts\0"));
		SerialXmitStartWait();
		strcpy_P(message,PSTR("- TEST1 - TEST3 Tests the Power, server fail or server resume alerts\0"));
		SerialXmitStartWait();

	} else {
		strcpy_P(message,PSTR("???\0"));
		SerialXmitStart();
	}
}

void SerialXmitStart(){
	SerialRXDisable();
	bufferp = message;
	UCSR0A|= (1<<TXC0);
	UDR0 = *bufferp;
	serialxmitstate = XMIT_START;
	bufferp++;
}

void SerialXmitStartWait(){
	SerialXmitStart();
	sei();
	while(serialxmitstate!=XMIT_COMPLETE)
		;
}

void PocsagTXComplete(void){
	strcpy_P(message,PSTR("SENT\0"));
	SerialXmitStartWait();
}