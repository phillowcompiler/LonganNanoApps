#ifndef __LCD_H
#define __LCD_H		

//#include <systick.h>
#include <arduino.h>
#include <stdlib.h>
#include <gd32vf103_gpio.h>
#include "font8x8.h"

#define USE_HORIZONTAL 0
#define HAS_BLK_CNTL    0

#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W 80
#define LCD_H 160
#else
#define LCD_W 160
#define LCD_H 80
#endif

typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long u32;    			


// #define LED_ON gpio_bit_reset(GPIOC,GPIO_PIN_13)
// #define LED_OFF gpio_bit_set(GPIOC,GPIO_PIN_13)

#define LED_ON 
#define LED_OFF 

#define SPI0_CFG 1  //hardware spi
// #define SPI0_CFG 2  //hardware spi dma
// #define SPI0_CFG 3  //software spi


//-----------------OLED端口定义---------------- 
#if SPI0_CFG == 1
#define LCD_SCLK_Clr() 
#define LCD_SCLK_Set() 

#define LCD_SDIN_Clr()
#define LCD_SDIN_Set()

#define LCD_CS_Clr() gpio_bit_reset(GPIOB,GPIO_PIN_2)     //CS PB2
#define LCD_CS_Set() gpio_bit_set(GPIOB,GPIO_PIN_2)
#elif SPI0_CFG == 2
#define LCD_SCLK_Clr() 
#define LCD_SCLK_Set() 

#define LCD_SDIN_Clr()
#define LCD_SDIN_Set()

#define LCD_CS_Clr()
#define LCD_CS_Set()
#else /* SPI0_CFG */
#define LCD_SCLK_Clr() gpio_bit_reset(GPIOA,GPIO_PIN_5)    //CLK PA5
#define LCD_SCLK_Set() gpio_bit_set(GPIOA,GPIO_PIN_5)

#define LCD_SDIN_Clr() gpio_bit_reset(GPIOA,GPIO_PIN_7)    //DIN PA7
#define LCD_SDIN_Set() gpio_bit_set(GPIOA,GPIO_PIN_7)

#define LCD_CS_Clr()  gpio_bit_reset(GPIOB,GPIO_PIN_2)     //CS PB2
#define LCD_CS_Set()  gpio_bit_set(GPIOB,GPIO_PIN_2)
#endif /* SPI0_CFG */

#define LCD_RST_Clr() gpio_bit_reset(GPIOB,GPIO_PIN_1)     //RES PB1
#define LCD_RST_Set() gpio_bit_set(GPIOB,GPIO_PIN_1)

#define LCD_DC_Clr() gpio_bit_reset(GPIOB,GPIO_PIN_0)      //DC PB0
#define LCD_DC_Set() gpio_bit_set(GPIOB,GPIO_PIN_0)


#if     HAS_BLK_CNTL
#define LCD_BLK_Clr()  gpio_bit_reset(GPIOA,GPIO_PIN_5)//BLK
#define LCD_BLK_Set()  gpio_bit_set(GPIOA,GPIO_PIN_5)
#else
#define LCD_BLK_Clr()
#define LCD_BLK_Set()
#endif

#define LCD_CMD  0	//写命令
#define LCD_DATA 1	//写数据

extern  u16 BACK_COLOR;   //背景色

#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE           	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40
#define BRRED 			 0XFC07
#define GRAY  			 0X8430
#define DARKBLUE      	 0X01CF
#define LIGHTBLUE      	 0X7D7C
#define GRAYBLUE       	 0X5458
#define LIGHTGREEN     	 0X841F
#define LGRAY 			 0XC618
#define LGRAYBLUE        0XA651
#define LBBLUE           0X2B12


class ST7735{
public:
	ST7735(){}
	void begin();
	void setAddrWindow(u16 x1, u16 y1, u16 w, u16 h);
	void fillRect(u16 x1, u16 y1, u16 w, u16 h, u16 fillcolor);
	void fillScreen(u16 fillcolor);
	void drawBitmap(u16 x, u16 y, u16 w, u16 h, uint16_t *p);
	void drawFastHLine(u16 x, u16 y, u16 w, u16 c);
	void drawFastVLine(u16 x, u16 y, u16 h, u16 c);
	void drawString(u16 x, u16 y, const char *s, u16 fc, u16 bc);
	
	private:
	u16 _width, _height;
	void init();
};
					  		 
#endif  
	 
	 



