/*
 * LCD_GFX.c
 *
 * Created: 9/20/2021 6:54:25 PM
 *  Author: You
 */ 

#include "LCD_GFX.h"
#include "ST7735.h"
#include <math.h>
#include <string.h>

/******************************************************************************
* Local Functions
******************************************************************************/



/******************************************************************************
* Global Functions
******************************************************************************/

/**************************************************************************//**
* @fn			uint16_t rgb565(uint8_t red, uint8_t green, uint8_t blue)
* @brief		Convert RGB888 value to RGB565 16-bit color data
* @note
*****************************************************************************/
uint16_t rgb565(uint8_t red, uint8_t green, uint8_t blue)
{
	return ((((31*(red+4))/255)<<11) | (((63*(green+2))/255)<<5) | ((31*(blue+4))/255));
}

/**************************************************************************//**
* @fn			void LCD_drawPixel(uint8_t x, uint8_t y, uint16_t color)
* @brief		Draw a single pixel of 16-bit rgb565 color to the x & y coordinate
* @note
*****************************************************************************/
void LCD_drawPixel(uint8_t x, uint8_t y, uint16_t color) {
	LCD_setAddr(x,y,x,y);
	SPI_ControllerTx_16bit(color);
}

/**************************************************************************//**
* @fn			void LCD_drawChar(uint8_t x, uint8_t y, uint16_t character, uint16_t fColor, uint16_t bColor)
* @brief		Draw a character starting at the point with foreground and background colors
* @note
*****************************************************************************/
void LCD_drawChar(uint8_t x, uint8_t y, uint16_t character, uint16_t fColor, uint16_t bColor){
	uint16_t row = character - 0x20;		//Determine row of ASCII table starting at space
	int i, j;
	if ((LCD_WIDTH-x>7)&&(LCD_HEIGHT-y>7)){
		for(i=0;i<5;i++){
			uint8_t pixels = ASCII[row][i]; //Go through the list of pixels
			for(j=0;j<8;j++){
				if ((pixels>>j)&1==1){
					LCD_drawPixel(x+i,y+j,fColor);
				}
				else {
					LCD_drawPixel(x+i,y+j,bColor);
				}
			}
		}
	}
}


/******************************************************************************
* LAB 4 TO DO. COMPLETE THE FUNCTIONS BELOW.
* You are free to create and add any additional files, libraries, and/or
*  helper function. All code must be authentically yours.
******************************************************************************/

/**************************************************************************//**
* @fn			void LCD_drawCircle(uint8_t x0, uint8_t y0, uint8_t radius,uint16_t color)
* @brief		Draw a colored circle of set radius at coordinates
* @note
*****************************************************************************/
void LCD_drawCircle(uint8_t x0, uint8_t y0, uint8_t radius,uint16_t color) 
{
	// assume that x0 and y0 is the center point
	
	// we are going to create an approx circle with rectangles:
	LCD_drawBlock((x0 - radius), (y0 - (radius/2)), (x0 + radius), (y0 + (radius/2)), color); 
	LCD_drawBlock((x0 - (radius/2)), (y0 - radius), (x0 + (radius/2)), (y0 + radius), color); 
	LCD_drawBlock((x0 - (3*radius/4)), (y0 - (3*radius/4)), (x0 + (3*radius/4)), (y0 + (3*radius/4)), color); 
}


/**************************************************************************//**
* @fn			void LCD_drawLine(short x0,short y0,short x1,short y1,uint16_t c)
* @brief		Draw a line from and to a point with a color
* @note
*****************************************************************************/
void LCD_drawLine(short x0,short y0,short x1,short y1,uint16_t c)
{
	uint8_t y_curr = 0;
	
	for (int x_curr = x0; x_curr <= x1; x_curr++) {
		y_curr = ((y1 - y0) / (x1 - x0)) * (x_curr - x0) + y1;
		LCD_drawPixel(x_curr, y_curr, c);	
	}
}


/**************************************************************************//**
* @fn			void LCD_drawBlock(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,uint16_t color)
* @brief		Draw a colored block at coordinates
* @note
*****************************************************************************/
void LCD_drawBlock(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,uint16_t color)
{
	uint8_t x_smaller = 0;
	uint8_t y_smaller = 0;
	uint8_t x_bigger = 0;
	uint8_t y_bigger = 0;
	
	if (sqrt(x0) + sqrt(y0) < sqrt(x1) + sqrt(y1)) {
		x_smaller = x0;
		y_smaller = y0;
		x_bigger = x1;
		y_bigger = y1;
	} else {
		x_smaller = x1;
		y_smaller = y1;
		x_bigger = x0;
		y_bigger = y0;
	}
	
	LCD_setAddr(x_smaller, y_smaller, x_bigger, y_bigger);
	
	for (int x_curr = x_smaller; x_curr <= x_bigger; x_curr++) {
		for (int y_curr = y_smaller; y_curr <= y_bigger; y_curr++) {
			SPI_ControllerTx_16bit(color);		
		}	
	}
}

/**************************************************************************//**
* @fn			void LCD_setScreen(uint16_t color)
* @brief		Draw the entire screen to a color
* @note
*****************************************************************************/
void LCD_setScreen(uint16_t color) 
{
	LCD_setAddr(0,0,LCD_WIDTH-1,LCD_HEIGHT-1);

	for (int x_curr = 0; x_curr < LCD_WIDTH; x_curr++) {
		for (int y_curr = 0; y_curr < LCD_HEIGHT; y_curr++) {
			SPI_ControllerTx_16bit(color);		
		}	
	}
}

/**************************************************************************//**
* @fn			void LCD_drawString(uint8_t x, uint8_t y, char* str, uint16_t fg, uint16_t bg)
* @brief		Draw a string starting at the point with foreground and background colors
* @note
*****************************************************************************/
void LCD_drawString(uint8_t x, uint8_t y, char* str, uint16_t fg, uint16_t bg)
{
	int num_letters = strlen(str);
	int x_curr = x;
	
	for (int pos = 0; pos < num_letters; pos++) { // go through each letter in str
		LCD_drawChar(x_curr, y, (uint16_t)str[pos], fg, bg);		
		x_curr+=8;
	}
}
