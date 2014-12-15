/*
 * SI4432.c
 *
 * Created: 24-9-2014 15:12:27
 *  Author: Jelmer Bruijn
 */ 

#include <avr/io.h>
#include "SI4432.h"
#define F_CPU 16000000UL
#include <util/delay.h>

volatile uint8_t spiret;

void SelectSI(){
	SPIPORT&=~(1<<SPI_SS);
}

void DeselectSI(){
	SPIPORT|=(1<<SPI_SS);
}


void WriteSI(uint8_t registerno, uint8_t val){
	SelectSI();
	SPDR=(0x80|registerno);
	while(!(SPSR & (1<<SPIF)));
	SPDR=val;
	while(!(SPSR & (1<<SPIF)));
	DeselectSI();
}

uint8_t ReadSI(uint8_t registerno){
	SelectSI();
	SPDR=registerno;
	while(!(SPSR & (1<<SPIF)));
	SPDR=0x00;
	while(!(SPSR & (1<<SPIF)));
	DeselectSI();
	return SPDR;
}

void SetupFreq(){
	uint16_t offsettwo = 0;
	WriteSI(0x75,(uint8_t)((xmitfreq>>16)&0xFF));
	WriteSI(0x76,(uint8_t)((xmitfreq>>8)&0xFF));
	WriteSI(0x77,(uint8_t)((xmitfreq)&0xFF));
	offsettwo+=offset;
	WriteSI(0x74,(uint8_t)((offsettwo>>8)&0xFF));
	WriteSI(0x73,(uint8_t)((offsettwo)&0xFF));
}
void SetupModule(){
	WriteSI(0x6D,0x17);
	WriteSI(0x6E,0x09);
	WriteSI(0x6F,0xD5);
	WriteSI(0x70,0x24);
	WriteSI(0x71,0x0A);
	WriteSI(0x72,0x07);
	WriteSI(0x0B,0x10);
	WriteSI(0x0C,0x0F); // set gpio1 to data clock out;
	WriteSI(0x0D,0x14); // gpio2 data out;
}

void SIXmit(){
	WriteSI(0x07,0x0B);
}

void SIRX(){
	WriteSI(0x07,0x07);
}

void SIXmitStop(){
	WriteSI(0x07,0x01);
}
void SetupSPI(void){
	SPIPORT_DDR|=(1<<SPI_SS)|(1<<SPI_MOSI)|(1<<SPI_CLK);
	SPCR|=(1<<SPE)|(1<<MSTR);
	//DDRA|=(1<<PORTA7);
	GPIODDR|=(1<<GPIO_DATA_CLK)|(1<<GPIO_DATA_IN);
	DeselectSI();
	SetupModule();
}



void WaitForSignal(){
	uint32_t temp;
	uint16_t count;
	SIRX();
	while(1){
		for(count=0;count<1000;count++){
			temp+=ReadSI(0x26);
		}
		spiret = temp/1000;
		temp=0;
		if(spiret>34){
			spiret=0;
		}
	}
}