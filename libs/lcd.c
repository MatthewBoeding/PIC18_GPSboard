
#include <xc.h>
#include <stdint.h>
#include "lcd_regs.h"
#include "lcd.h"

#define CD  LATCbits.LATC4
#define SS  LATCbits.LATC3

uint16_t cursor_x, cursor_y;
uint8_t textsize, rotation;
uint16_t textcolor, textbgcolor, _width, _height;

void setCursor(uint16_t x, uint16_t y){
	cursor_x = x;
	cursor_y = y;
}
// set text color
void setTextColor(uint16_t x, uint16_t y){
	textcolor =  x;
	textbgcolor = y;
}

// set text size
void setTextSize(uint8_t s){
	if (s > 8) return;
	textsize = (s>0) ? s : 1 ;
}

void write8(uint8_t data)
{
    SPI1TXB = data;
    __delay_us(10);
    return;
}

void writeRegister8(uint8_t reg, uint8_t data)
{
    SS = __ACTIVE__;
    CD = __CMD__;
    SPI1TXB = reg;
    CD = __DATA__;
    SPI1TXB = data;
    return;
}

void writeRegister16(uint16_t reg, uint16_t data)
{
    uint8_t hi, lo;
    hi= uint8_t(reg);
    lo = uint8_t(reg >> 8);
    SS = __ACTIVE__;
    CD = __CMD__;
    SPI1TXB = hi;
    SPI1TXB = lo;
    hi = uint8_t(data);
    lo = uint8_t(data >> 8);
    CD=__DATA__;
    SPI1TXB = hi;
    SPI1TXB = lo;
    return;
}

void setAddrWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    CD = __CMD__;
    write8(ILI9341_COLADDRSET);
    CD = __DATA__;
    write8(x1 >> 8);
    write8(x1);
    write8(x2 >> 8);
    write8(x2);
    CD = __CMD__;
    write8(ILI9341_PAGEADDRSET);
    CD = __DATA__;
    write8(y1 >> 8);
    write8(y1);
    write8(y2 >> 8);
    write8(y2);
    return;
}

void lcd_init(void)
{
	//char ID[5];
	///int id;
	_width = TFTWIDTH;
	_height = TFTHEIGHT;
	//IOM = 0;    
    writeRegister8(ILI9341_SOFTRESET, 0);
    __delay_ms(50);
    writeRegister8(ILI9341_DISPLAYOFF, 0);

    writeRegister8(ILI9341_POWERCONTROL1, 0x23);
    writeRegister8(ILI9341_POWERCONTROL2, 0x10);
    writeRegister16(ILI9341_VCOMCONTROL1, 0x2B2B);
    writeRegister8(ILI9341_VCOMCONTROL2, 0xC0);
    writeRegister8(ILI9341_MEMCONTROL, ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR);
    writeRegister8(ILI9341_PIXELFORMAT, 0x55);
    writeRegister16(ILI9341_FRAMECONTROL, 0x001B);
    
    writeRegister8(ILI9341_ENTRYMODE, 0x07);
    /* writeRegister32(ILI9341_DISPLAYFUNC, 0x0A822700);*/

    writeRegister8(ILI9341_SLEEPOUT, 0);
    __delay_ms(150);
    writeRegister8(ILI9341_DISPLAYON, 0);
    __delay_ms(500);
    setAddrWindow(0, 0, TFTWIDTH-1, TFTHEIGHT-1);
    return;
}

void drawPixel(uint16_t x3,uint16_t y3, uint16_t color1)
{	
    if ((x3 < 0) ||(x3 >= TFTWIDTH) || (y3 < 0) || (y3 >= TFTHEIGHT))
	{
		return;
	}
	setAddrWindow(x3,y3,x3+1,y3+1);

    CD = __CMD__; 
    write8(0x2C);
	
	CD = __DATA__;
	write8(color1>>8);
    write8(color1); 	 
    return;
} 

void drawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)
{
	int f = 1 - r;
    int ddF_x = 1;
    int ddF_y = -2 * r;
    int x = 0;
    int y = r;

    
    drawPixel(x0  , y0+r, color);
    drawPixel(x0  , y0-r, color);
    drawPixel(x0+r, y0  , color);
    drawPixel(x0-r, y0  , color);

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        drawPixel(x0 + x, y0 + y, color);
        drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 + x, y0 - y, color);
        drawPixel(x0 - x, y0 - y, color);
        drawPixel(x0 + y, y0 + x, color);
        drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 + y, y0 - x, color);
        drawPixel(x0 - y, y0 - x, color);
    }
    return;
}

void fillRect(uint16_t x, uint16_t y, uint16_t w,uint16_t h,uint16_t color)
{
	if ((x >= TFTWIDTH) || (y >= TFTHEIGHT))
	{
		return;
	}

	if ((x+w-1) >= TFTWIDTH)
	{
		w = TFTWIDTH-x;
	}

	if ((y+h-1) >= TFTHEIGHT)
	{
		h = TFTHEIGHT-y;
	}

	setAddrWindow(x, y, x+w-1, y+h-1);
    //IOM = 0;
	
    CD = __CMD__;
	write8(0x2C);
	//IOM = 1; IOM = 0;
	CD = __DATA__;
	for(y=h; y>0; y--) 
	{
		for(x=w; x>0; x--)
		{
			
			write8(color>>8); 
            write8(color);
		}
	}
	//IOM = 1;
}

void fillScreen(unsigned int Color)
{
	//unsigned char VH,VL;
	long len = (long)TFTWIDTH * (long)TFTHEIGHT;
	
	 int blocks;
	
   unsigned char  i, hi = Color >> 8,
              lo = Color;
	
    blocks = (uint16_t)(len / 64); // 64 pixels/block
	setAddrWindow(0,0,TFTWIDTH-1,TFTHEIGHT-1);
	
    CD = __CMD__;
	write8(0x2C);
    CD = __DATA__;
	write8(hi); 
    write8(lo);
	len--;
	while(blocks--) {
      i = 16; // 64 pixels/block / 4 pixels/pass
      do {
            write8(hi); 
            write8(lo);
            write8(hi); 
            write8(lo);
            write8(hi); 
            write8(lo);
            write8(hi);
            write8(lo);
      } while(--i);
    }
    for(i = (char)len & 63; i--; ) {
			
      write8(hi); 
      write8(lo);
			
    }
}	
void drawChar(uint16_t x, uint16_t y, uint8_t c, uint16_t color, uint16_t bg, uint8_t size)
{
	if ((x >=TFTWIDTH) || // Clip right
	    (y >=TFTHEIGHT)           || // Clip bottom
	    ((x + 6 * size - 1) < 0) || // Clip left
	    ((y + 8 * size - 1) < 0))   // Clip top
	{
		return;
	}

	for (char i=0; i<6; i++ )
	{
		uint8_t line;

		if (i == 5)
		{
			line = 0x0;
		}
		else 
		{
			line = font[(c*5)+i];
		}

		for (char j = 0; j<8; j++)
		{
			if (line & 0x1)
			{
				if (size == 1) // default size
				{
					drawPixel(x+i, y+j, color);
				}
				else {  // big size
					fillRect(x+(i*size), y+(j*size), size, size, color);
				} 
			} else if (bg != color)
			{
				if (size == 1) // default size
				{
					drawPixel(x+i, y+j, bg);
				}
				else 
				{  // big size
					fillRect(x+i*size, y+j*size, size, size, bg);
				}
			}
			line >>= 1;
		}
	}
}

void lcdWrite(uint8_t c)//write a character at setted coordinates after setting location and colour
{
	if (c == '\n')
	{
		cursor_y += textsize*8;
		cursor_x  = 0;
	}
	else if (c == '\r')
	{
		// skip em
	}
	else
	{
		drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
		cursor_x += textsize*6;
	}
}

void LCD_string_write(char *str)
{
	int i;
	for(i=0;str[i]!=0;i++)	/* Send each char of string till the NULL */
	{
		lcdWrite(str[i]);	/* Call transmit data function */
	}
}	