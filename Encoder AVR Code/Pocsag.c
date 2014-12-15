/*
 * Pocsag.c
 *
 * Created: 24-9-2014 15:05:12
 *  Author: Jelmer Bruijn
 */ 

/*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This application is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include "Pocsag.h"
#include "Serial.h"
#include "SI4432.h"

uint32_t syncword = SYNCWORD;
uint32_t idleword = IDLEWORD;

char message[MSG_BUFFER_LEN] = "\0";

char bitswitch(char b){
	// a really cool way to reverse bit order. Not sure if it's the fastest, but it works!
	b = (b * 0x0202020202ULL & 0x010884422010ULL) % 1023;
	return (b>>1);
}
uint32_t createcrc(uint32_t in) {
	// I borrowed this routine from Kristoff. Credit goes to him!
	// https://github.com/on1arf/rf22_pocsag
	// (c) 2014 Kristoff Bonne (ON1ARF)
	// 
	// local vars
	uint32_t cw; // codeword
	uint32_t local_cw = 0;
	uint32_t parity = 0;
	// init cw
	cw=in;
	// move up 11 bits to make place for crc + parity
	local_cw=in; /* bch */
	// calculate crc
	for (int bit=1; bit<=21; bit++, cw <<= 1) {
		if (cw & 0x80000000) {
			cw ^= 0xED200000;
		}; // end if
	}; // end for
	local_cw |= (cw >> 21);
	// parity
	cw=local_cw;
	for (int bit=1; bit<=32; bit++, cw<<=1) {
		if (cw & 0x80000000) {
			parity++;
		}; // end if
	}; // end for
	// make even parity
	if (parity % 2) {
		local_cw++;
	}; // end if
	// done
	return(local_cw);
}
uint32_t MakeMessage(uint32_t message){
	// message frames have the MSB bit set. This routine makes a message frame from the supplied 4 byte word
	uint32_t tempframe;
	tempframe=0x80000000;
	tempframe|=(message<<11);
	tempframe=createcrc(tempframe);
	return tempframe;
}
uint8_t DecodeNumber(char c){
	// This is a lookup for the relevant numbers for alpha pagers.
	switch(c){
		case 0x30:
		return 0;
		case 0x31:
		return 0x08;
		case 0x32:
		return 0x04;
		case 0x33:
		return 0x0C;
		case 0x34:
		return 0x02;
		case 0x35:
		return 0x0A;
		case 0x36:
		return 0x06;
		case 0x37:
		return 0x0E;
		case 0x38:
		return 0x01;
		case 0x39:
		return 0x09;
		case 0x2F: //spare is slash
		return 0x05;
		case 0x55: // U is urgent?
		return 0x0D;
		case 0x20: // space
		return 0x03;
		case 0x2D: // --
		return 0x0B;
		case 0x5D: // [
		return 0x07;
		case 0x5B: // ]
		return 0x0F;
		default:
		return 0x05;
		break;
	}
}
void SetupTimer(void){
	// Sets up timer1 for use by the pocsag encoder, enables interrupts
	sei();
	OCR1A=(F_CPU/baudrate);
	TCCR1B|=(1<<3);
	TIMSK1|=(1<<OCIE1A);
}
void ClearBuffer(){
	// Loads the buffer with the IDLE codeword. Always a good starting point
	uint8_t count = 0;
	for(count=0;count<16;count++){
		frame[count]=idleword;
	}
}
void StartXmit(){
	// Commences transmit of pocsag frames. Sets up the timer
	SetupTimer();
	Prepare();
	// Begin transmitting. After turning on the transmitter, we wait for a little bit to allow the transmitter to stabilize
	SIXmit();
	_delay_ms(1);
	// We want the timer to start at 0!
	TCNT1 = 0x0000;
	// Set prescaler to start the timer.
	TCCR1B|=(1<<CS10);
	// Begin to send with a mark (1)
	SENDMARK;
	xmitState = STATE_PREAMBLE_MARK;
	preambleLeft = PREAMBLELENGTH;
}
void StopXmit(){
	// Turns off transmitter and stops the timer
	SIXmitStop();
	TCCR1B&=~(1<<CS10);
}
void ContinueXmit(){
	// Apparently we're done with this batch, prepare the new one.
	uint8_t charcount = 0;
	uint32_t tempframe = 0;
	uint8_t framecount = 0;
	uint8_t bitsleft = 0;
	ClearBuffer();
	switch(messagetype){
		case 0: // numeric page
		while((*messagep!=0)&&(framecount<=MAX_FRAME_COUNT)){ // loop for as long as we're not at the end of the message, and the frame isn't full yet
			charcount=0;
			tempframe=0;
			while((*messagep!=0)&&(charcount<5)){
				tempframe<<=4;
				tempframe|=DecodeNumber(*messagep);
				messagep++;
				charcount++;
			}
			while(charcount<5){
				tempframe<<=4;
				tempframe|=DecodeNumber(*messagep);
				charcount++;
			}
			frame[framecount]=MakeMessage(tempframe);
			framecount++;
		}
		break;
		case 3: // alphanumeric page
		while((*messagep!=0)&&(framecount<=MAX_FRAME_COUNT)){ // loop until end of message or and of batch
			// The alphanumeric frames are slightly more complicated as they're not aligned with the number of bits per character
			// We need to save the amount of bits we can stash in a frame, and put the rest of it in the next frame.
			// Very similar to the 'prepare' stage.
			charcount=0;
			tempframe=0;
			bitsleft=DATABITS_PER_FRAME;
			if(bitalign){
				tempframe|=bitswitch(((*messagep)&0x7F));
				bitsleft-=bitalign;
				bitalign=0;
				if(*messagep!=0){
					messagep++;
				}
			}
			while((*messagep!=0)&&(bitsleft>=7)){
				tempframe<<=7;
				tempframe|=bitswitch(((*messagep)&0x7F));
				messagep++;
				bitsleft-=7;
			}
			tempframe=(tempframe<<bitsleft);
			bitalign=7-bitsleft;
			tempframe|=(bitswitch(((*messagep)&0x7F))>>bitalign);
			frame[framecount]=MakeMessage(tempframe);
			framecount++;
		}
		
	}
}
uint32_t MakeAddressFrame(uint32_t ric, uint8_t type){
	uint32_t tempframe;
	// address is the most significant 18 bits of the RIC. Remaining 3 bits is determined by
	// the position in the batch. type is here function type, not the type of pager
	ric>>=3;
	ric<<=13;
	tempframe=0;
	tempframe|=ric;
	tempframe|=(((uint32_t)(type&0x03))<<11);
	return createcrc(tempframe);
}
uint8_t GetStartFrame(uint32_t ric){
	// The three MSB of the RIC determine the starting frame position within the batch
	return (uint8_t)(ric&0x07)*2;
}

void Prepare(void){
	// Loads the buffer for initial transmit.
	uint8_t charcount = 0;
	uint8_t framecount = 0;
	uint32_t tempframe = 0;
	uint8_t bitsleft;
	bitalign = 0;
	messagep = &(message[0]);
	ClearBuffer(); // load the buffer with the IDLE codeword
	framecount = GetStartFrame(pagerric);
	// Load the address frame in the correct position, with the selected function type
	frame[framecount]=MakeAddressFrame(pagerric,functiontype);
	
	// What happens next is largely dependent on the type of page. tone-only pages only have
	// an address frame, but numeric and alphanumeric pages have a message attached.
	switch(messagetype){
		case 0: // numeric page
		// Numeric pages are pretty straightforward, as a they contain a fixed number of characters per frame
		while((*messagep!=0)&&(framecount<MAX_FRAME_COUNT)){ // loop for as long 
			framecount++;
			charcount=0;
			tempframe=0;
			while((*messagep!=0)&&(charcount<5)){ // check if the pointer doesnt point to the 0 character, and load characters as long as it doesnt.
				tempframe<<=4;
				tempframe|=DecodeNumber(*messagep);
				messagep++;
				charcount++;
			}
			while(charcount<5){ // if we enter this block, we ran out of characters before the frame was full. Fill it up with 0-pointers
				tempframe<<=4;
				tempframe|=DecodeNumber(*messagep);
				charcount++;
			}
			// calculate CRC for the frame
			frame[framecount]=MakeMessage(tempframe);
		}
		break;
		case 3: // alpha page
		while((*messagep!=0)&&(framecount<MAX_FRAME_COUNT)){
			framecount++;
			charcount=0;
			tempframe=0;
			bitsleft=DATABITS_PER_FRAME;
			if(bitalign){ // if bitalign is not 0, there were bits for a previous character left. Shift them in!
				tempframe|=bitswitch(((*messagep)&0x7F));
				bitsleft-=bitalign;
				bitalign=0;
				if(*messagep!=0){
					messagep++;
				}
			}
			// Now lets see how many full characters we can fit in there (usually 1 or 2)
			while((*messagep!=0)&&(bitsleft>=7)){
				tempframe<<=7;
				tempframe|=bitswitch(((*messagep)&0x7F));
				messagep++;
				bitsleft-=7;
			}
			// Now 'OR' the first bits of the next character onto the frame, and set how many bits we've fit
			// in there in bitalign
			tempframe=(tempframe<<bitsleft);
			bitalign=7-bitsleft;
			tempframe|=(bitswitch(((*messagep)&0x7F))>>bitalign);
			frame[framecount]=MakeMessage(tempframe);
		}
		
		
	}
}
ISR(TIMER1_COMPA_vect){
	if(timercorrection==2){
		timercorrection=0;
		OCR1A=(F_CPU/baudrate)+1;
	} else {
		OCR1A=(F_CPU/baudrate);
		timercorrection++;
	}
	TIFR1|=(1<<OCF1A);
	switch(xmitState){
		case STATE_PREAMBLE_MARK:
		SENDMARK;
		xmitState = STATE_PREAMBLE_SPACE;
		preambleLeft--;
		if(!(preambleLeft)){
			xmitState = STATE_SYNCWORD;
			codeWordBit=CODEWORDSIZE;
		}
		break;
		case STATE_PREAMBLE_SPACE:
		SENDSPACE;
		xmitState = STATE_PREAMBLE_MARK;
		preambleLeft--;
		if(!(preambleLeft)){
			xmitState = STATE_SYNCWORD;
			codeWordBit=CODEWORDSIZE;
		}
		break;
		case STATE_SYNCWORD:
		codeWordBit--;
		if((syncword>>codeWordBit)&0x00000001){
			SENDMARK;
			} else {
			SENDSPACE;
		}
		if(!(codeWordBit)){
			xmitState = STATE_FRAME;
			frameNo = 0;
			codeWordBit=CODEWORDSIZE;
		}
		break;
		case STATE_FRAME:
		codeWordBit--;
		if(((frame[frameNo])>>codeWordBit)&0x00000001){
			SENDMARK;
			} else {
			SENDSPACE;
		}
		if(!(codeWordBit)){
			if(frameNo<MAX_FRAME_COUNT){
				frameNo++;
				codeWordBit=CODEWORDSIZE;
				} else {
				xmitState=STATE_SYNCWORD;
				codeWordBit=CODEWORDSIZE;
				if(frame[frameNo]==idleword){
					SENDSPACE;
					xmitState=STATE_OFF;
					StopXmit();
					PocsagTXComplete();
					} else {
					sei();
					ContinueXmit();
				}

			}

		}
		break;
	}
}