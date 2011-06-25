//************************************************************************
//
//				LCD_driver.c: Interface for Nokia LCD
//
//************************************************************************
//************************************************************************
//*	Edit History
//*		<MLS>	= Mark Sproul msproul -at- jove.rutgers.edu
//*		<ytsuboi> = Yoshihiro TSUBOI
//************************************************************************
//*	Apr  2,	2010	<MLS> I received my Color LCD Shield sku: LCD-09363 from sparkfun
//*	Apr  2,	2010	<MLS> The code was written for WinAVR, I modified it to compile under Arduino
//*	Apr  3,	2010	<MLS> Changed LCDSetPixel to make it "RIGHT" side up
//*	Apr  3,	2010	<MLS> Made LCDSetPixel public
//*	Apr  3,	2010	<MLS> Working on MEGA, pin defs in nokia_tester.h
//*	Apr  4,	2010	<MLS> Removed delays from LCDCommand & LCDCommand, increased speed by 75%
//* June 25, 2011	<ytsuboi> Added LCDContrast
//************************************************************************
#include "nokia_tester.h"
#include "LCD_driver.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#ifdef _USE_ARDUINO_FOR_NOKIEA_
	#include	"WProgram.h"
#endif

//************************************************************************
//
//				"Private" Function Definitions
//
//************************************************************************


//************************************************************************
//Usage: LCDClear(black);
//Inputs: char color: 8-bit color to be sent to the screen.
//Outputs: None
//Description: This function will clear the screen with "color" by writing the
//				color to each location in the RAM of the LCD.
//************************************************************************
void LCDClear(int color)
{
unsigned int i;

	#ifdef EPSON
		LCDCommand(PASET);
		LCDData(0);
		LCDData(131);
	
		LCDCommand(CASET);
		LCDData(0);
		LCDData(131);

		LCDCommand(RAMWR);
	#endif
	#ifdef	PHILLIPS
		LCDCommand(PASETP);
		LCDData(0);
		LCDData(131);
	
		LCDCommand(CASETP);
		LCDData(0);
		LCDData(131);

		LCDCommand(RAMWRP);
	#endif
	
	for (i=0; i < (131*131)/2; i++)
	{
		LCDData((color >> 4) & 0x00FF);
		LCDData(((color & 0x0F) << 4) | (color >> 8));
		LCDData(color & 0x0FF);		// nop(EPSON)
	}
	
//	x_offset = 0;
//	y_offset = 0;
}


//************************************************************************
void	delay_us(int microSecs)
{
	delayMicroseconds(microSecs);
}

//************************************************************************
void	delay_ms(int millisecs)
{
//	delay(millisecs);
//	delayMicroseconds(millisecs * 1000);
}


//************************************************************************
//Usage: LCDCommand(RAMWR);
//Inputs: char data - character command to be sent to the LCD
//Outputs: None
//************************************************************************
void LCDCommand(unsigned char data)
{
char jj;
	cbi(LCD_PORT_CS, CS);		// enable chip, p0.20 goes low
	//delay_us(1);
	cbi(LCD_PORT_DIO, DIO);		// output low on data out (9th bit low = command), p0.19
	//delay_us(1);

	cbi(LCD_PORT_SCK, SCK);		// send clock pulse
	delay_us(1);
	sbi(LCD_PORT_SCK, SCK);
	//delay_us(1);

	for (jj = 0; jj < 8; jj++)
	{
		if ((data & 0x80) == 0x80)
		{
			sbi(LCD_PORT_DIO, DIO);
		}
		else
		{
			cbi(LCD_PORT_DIO, DIO);
		}
		//delay_us(1);

		cbi(LCD_PORT_SCK, SCK);		// send clock pulse
	//+	delay_us(1);
		sbi(LCD_PORT_SCK, SCK);

		data <<= 1;
	}

	sbi(LCD_PORT_CS, CS);			// disable
}

//************************************************************************
//Usage: LCDData(RAMWR);
//Inputs: char data - character data to be sent to the LCD
//Outputs: None
//************************************************************************
void LCDData(unsigned char data)
{
char j;

	cbi(LCD_PORT_CS, CS);			// enable chip, p0.20 goes low
	//delay_us(1);
	sbi(LCD_PORT_DIO, DIO);			// output high on data out (9th bit high = data), p0.19
	//delay_us(1);
	
	cbi(LCD_PORT_SCK, SCK);			// send clock pulse
//+	delay_us(1);
	sbi(LCD_PORT_SCK, SCK);			// send clock pulse
	//delay_us(1);

	for (j = 0; j < 8; j++)
	{
		if ((data & 0x80) == 0x80)
		{
			sbi(LCD_PORT_DIO, DIO);
		}
		else
		{
			cbi(LCD_PORT_DIO, DIO);
		}
		//delay_us(1);
		
		cbi(LCD_PORT_SCK, SCK);		// send clock pulse
//+		delay_us(1);
		sbi(LCD_PORT_SCK, SCK);

		data <<= 1;
	}

	LCD_PORT_CS	|=	(1<<CS);		// disable
}

//************************************************************************
//Usage: LCDInit();
//Inputs: None
//Outputs: None
//Description:  Initializes the LCD regardless of if the controlller is an EPSON or PHILLIPS.
//************************************************************************
void LCDInit(void)
{
	delay_ms(200);
							
	cbi(LCD_PORT_SCK, SCK);			//output_low (SPI_CLK);//output_low (SPI_DO);
	cbi(LCD_PORT_DIO, DIO);
	delay_us(10);
	sbi(LCD_PORT_CS, CS);			//output_high (LCD_CS);
	delay_us(10);
	cbi(LCD_PORT_RES, LCD_RES);		//output_low (LCD_RESET);
	delay_ms(200);
	sbi(LCD_PORT_RES, LCD_RES);		//output_high (LCD_RESET);
	delay_ms(200);
	sbi(LCD_PORT_SCK, SCK);
	sbi(LCD_PORT_DIO, DIO);
	delay_us(10);
	
	LCDCommand(DISCTL);		// display control(EPSON)
	LCDData(0x0C);			// 12 = 1100 - CL dividing ratio [don't divide] switching period 8H (default)
	LCDData(0x20);	
	//LCDData(0x02);
	LCDData(0x00);
	
	LCDData(0x01);
	
	LCDCommand(COMSCN);		// common scanning direction(EPSON)
	LCDData(0x01);
	
	LCDCommand(OSCON);		// internal oscialltor ON(EPSON)
	
	LCDCommand(SLPOUT);		// sleep out(EPSON)
	LCDCommand(P_SLEEPOUT);	//sleep out(PHILLIPS)
	
	LCDCommand(PWRCTR);		// power ctrl(EPSON)
	LCDData(0x0F);			//everything on, no external reference resistors
	LCDCommand(P_BSTRON);	//Booset On(PHILLIPS)
	
	LCDCommand(DISINV);		// invert display mode(EPSON)
	LCDCommand(P_INVON);	// invert display mode(PHILLIPS)
	
	LCDCommand(DATCTL);		// data control(EPSON)
	LCDData(0x03);			// correct for normal sin7
	LCDData(0x00);			// normal RGB arrangement
	//LCDData(0x01);		// 8-bit Grayscale
	LCDData(0x02);			// 16-bit Grayscale Type A
	
	LCDCommand(P_MADCTL);	//Memory Access Control(PHILLIPS)
	LCDData(0xC8);
	
	LCDCommand(P_COLMOD);	// Set Color Mode(PHILLIPS)
	LCDData(0x02);	
	
	LCDCommand(VOLCTR);		// electronic volume, this is the contrast/brightness(EPSON)
	//LCDData(0x18);		// volume (contrast) setting - fine tuning, original
	LCDData(0x24);			// volume (contrast) setting - fine tuning, original
	LCDData(0x03);			// internal resistor ratio - coarse adjustment
	LCDCommand(P_SETCON);	// Set Contrast(PHILLIPS)
	LCDData(0x30);	
	
	LCDCommand(NOP);		// nop(EPSON)
	LCDCommand(P_NOP);		// nop(PHILLIPS)
	
	delay_ms(200);

	LCDCommand(DISON);		// display on(EPSON)
	LCDCommand(P_DISPON);	// display on(PHILLIPS)
}


//************************************************************************
//Usage: LCDSetPixel(white, 0, 0);
//Inputs: unsigned char color - desired color of the pixel
//			unsigned char x - Page address of pixel to be colored
//			unsigned char y - column address of pixel to be colored
//Outputs: None
//Description: Sets the starting page(row) and column (x & y) coordinates in ram,
//				then writes the colour to display memory.	The ending x & y are left
//				maxed out so one can continue sending colour data bytes to the 'open'
//				RAMWR command to fill further memory.	issuing any red command
//				finishes RAMWR.
//**NOTE** Because this function is static, it is essentially a "private" function
//		and can only be used within this file!
//*	Apr  3,	2010	<MLS> Made LCDSetPixel public
//************************************************************************
void LCDSetPixel(int color, unsigned char x, unsigned char y)
{
int	myYYvalue;

//*	Apr  3,	2010	<MLS> This is to make it "RIGHT" side up
	
	myYYvalue	=	(COL_HEIGHT - 1) - y;

	#ifdef EPSON
		LCDCommand(PASET);	// page start/end ram
		LCDData(x);
		LCDData(ENDPAGE);
	
		LCDCommand(CASET);	// column start/end ram
		LCDData(myYYvalue);
		LCDData(ENDCOL);
	
		LCDCommand(RAMWR);	// write
		LCDData((color>>4)&0x00FF);
		LCDData(((color&0x0F)<<4)|(color>>8));
		LCDData(color&0x0FF);		// nop(EPSON)		
		//LCDData(color);
		//LCDData(NOP);
		//LCDData(NOP);
	#endif
	#ifdef	PHILLIPS
		LCDCommand(PASETP);	// page start/end ram
		LCDData(x);
		LCDData(ENDPAGE);
	
		LCDCommand(CASETP);	// column start/end ram
		LCDData(myYYvalue);
		LCDData(ENDCOL);
	
		LCDCommand(RAMWRP);	// write
		
		LCDData((unsigned char)((color>>4)&0x00FF));
		LCDData((unsigned char)(((color&0x0F)<<4)|0x00));
	#endif

}

void LCDContrast(char setting) {
     LCDCommand(VOLCTR);        // electronic volume, this is the contrast/brightness(EPSON)
     LCDData(setting);        // volume (contrast) setting - course adjustment,  -- original was 24
//     LCDData(0x03);            // internal resistor ratio - coarse adjustment
//     LCDData(0x30);   
     LCDCommand(NOP);        // nop(EPSON)
}

