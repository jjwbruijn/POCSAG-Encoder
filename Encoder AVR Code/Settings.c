/*
 * Settings.c
 *
 * Created: 27-11-2014 12:11:58
 *  Author: Jelmer Bruijn
 */ 

#include <avr/eeprom.h>
#include "Encoder.h"
#include "Settings.h"
#include "Pocsag.h"
#include "SI4432.h"

uint32_t EEMEM EERic = 10238;
uint8_t EEMEM EEFunc = 0;
uint8_t EEMEM EEType = 3;
uint16_t EEMEM EEBaud = 1200;
uint32_t EEMEM EEFreq = 0x5616F8;
int16_t EEMEM EEOffset = 0x34;
char EEMEM EEPowerLossMsg[161] ="POWER LOSS!\0";
char EEMEM EEServerFailMsg[161] ="SERVER FAIL!\0";
char EEMEM EEServerResumeMsg[161] = "SERVER RESUME\0";
uint8_t EEMEM EEPowerLossThreshold;
uint16_t EEMEM EETotalMessages;

void SaveSettings(void){
	eeprom_update_dword(&(EERic),pagerric);
	eeprom_update_byte(&(EEFunc),functiontype);
	eeprom_update_byte(&(EEType),messagetype);
	eeprom_update_word(&(EEBaud),baudrate);
	eeprom_update_dword(&(EEFreq),xmitfreq);
	eeprom_update_word(&(EEOffset),offset);
}

void SaveThreshold(){
	eeprom_update_byte(&(EEPowerLossThreshold),ADCThreshold);
}

void LoadSettings(void){
	pagerric = eeprom_read_dword(&(EERic));
	functiontype = eeprom_read_byte(&(EEFunc));
	messagetype = eeprom_read_byte(&(EEType));
	baudrate = eeprom_read_word(&(EEBaud));
	xmitfreq = eeprom_read_dword(&(EEFreq));
	offset = eeprom_read_word(&(EEOffset));
	ADCThreshold = eeprom_read_byte(&(EEPowerLossThreshold));
	SetupFreq();
}

void SavePowerLossMsg(void){
	message[160]=0;
	eeprom_update_block((void *)&(message),(void *)&(EEPowerLossMsg),161);
}

void LoadPowerLossMsg(void){
	eeprom_read_block(message,(void *)&(EEPowerLossMsg),161);
}

uint8_t PowerLossMsgSet(void){
	return eeprom_read_byte((uint8_t *)&(EEPowerLossMsg));
}

void SaveServerFailMsg(void){
	message[160]=0;
	eeprom_update_block((void *)&(message),(void *)&(EEServerFailMsg),161);
}
void LoadServerFailMsg(void){
	eeprom_read_block(message,(void *)&(EEServerFailMsg),161);
}

uint8_t ServerFailMsgSet(void){
	return eeprom_read_byte((uint8_t *)&(EEServerFailMsg));
}

void SaveServerResumeMsg(void){
	message[160]=0;
	eeprom_update_block((void *)&(message),(void *)&(EEServerResumeMsg),161);	
}

void LoadServerResumeMsg(void){
	eeprom_read_block(message,(void *)&(EEServerResumeMsg),161);
}

uint8_t ServerResumeMsgSet(void){
	return eeprom_read_byte((uint8_t *)&(EEServerResumeMsg));
}