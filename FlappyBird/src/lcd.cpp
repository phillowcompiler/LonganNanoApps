#include "lcd.h"

void spi_config(void)
{
    spi_parameter_struct spi_init_struct;
    /* deinitilize SPI and the parameters */
    //LCD_CS_Set();
    spi_struct_para_init(&spi_init_struct);

    /* SPI0 parameter config */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_8;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI0, &spi_init_struct);

	spi_crc_polynomial_set(SPI0,7);
	spi_enable(SPI0);
}


void write8(u8 dat)
{
	while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE));
        spi_i2s_data_transmit(SPI0, dat);
	while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE));
        spi_i2s_data_receive(SPI0);
}

void write16(uint16_t dat)
{
	write8(dat>>8);
	write8(dat);
}


void writecommand(u8 c)
{
	LCD_DC_Clr();
	write8(c);
	LCD_DC_Set();
}

void flood(u16 *d, u16 n)
{
	while(n--){write16(*d++);}
}

void ST7735::begin()
{
	_width = LCD_W;
	_height = LCD_H;
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOB);

//#if SPI0_CFG == 1
 	rcu_periph_clock_enable(RCU_AF);
	rcu_periph_clock_enable(RCU_SPI0);
	/* SPI0 GPIO config: SS/PB2, SCK/PA5, MOSI/PA7 */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5 |GPIO_PIN_6| GPIO_PIN_7);
	// TFT_CS Anytime Low 
	gpio_init(GPIOB, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ, GPIO_PIN_2);

	spi_config();

	// RS(D/C):PB0, RESET PB1
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0 | GPIO_PIN_1);
	gpio_bit_reset(GPIOB, GPIO_PIN_0 | GPIO_PIN_1);
	
	init();
}


void ST7735::init()
{
	LCD_RST_Clr();
	delay(200);
	LCD_RST_Set();
	delay(20);
	// Soft reset
	writecommand(0x11);
	delay(150);
	// Inv ON
	writecommand(0x21);
	// MY,MX,MV,ML,RGB,MH,0,0
	writecommand(0x36);
	write8(((0<<7)|(0<<6)|(0<<5)|(0<<4)|(1<<3)|(0<<2)));
	// 16-bit/pixel(RGB565)
	writecommand(0x3A);
	write8(0x5);
	// LCD ON
	writecommand(0x29);
}

void ST7735::setAddrWindow(u16 x1, u16 y1, u16 w, u16 h)
{
	u16  x2, y2;
	// Initial off-screen clipping
  	if( (w            <= 0     ) ||  (h             <= 0      ) ||
      (x1           >= _width) ||  (y1            >= _height) ||
     ((x2 = x1+w-1) <  0     ) || ((y2  = y1+h-1) <  0      )) return;
  	if(x1 < 0) { // Clip left
    	w += x1;
    	x1 = 0;
  	}
  	if(y1 < 0) { // Clip top
    	h += y1;
    	y1 = 0;
  	}
  	if(x2 >= _width) { // Clip right
    	x2 = _width - 1;
    	w  = x2 - x1 + 1;
  	}
  	if(y2 >= _height) { // Clip bottom
    	y2 = _height - 1;
    	h  = y2 - y1 + 1;
  	}
	
#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
	writecommand(0x2A);
	write16(x1+26);
	write16(x2+26);
	
	writecommand(0x2B);
	write16(y1+1);
	write16(y2+1);
#else
	writecommand(0x2A);
	write16(x1+1);
	write16(x2+1);
	
	writecommand(0x2B);
	write16(y1+26);
	write16(y2+26);
#endif
	writecommand(0x2C);
}

void ST7735::fillRect(u16 x1, u16 y1, u16 w, u16 h, u16 fillcolor)
{
	setAddrWindow(x1, y1, w, h);
  	u16 n = w * h;
  	while(n--){write16(fillcolor);}
}

void ST7735::fillScreen(u16 fillcolor)
{
	fillRect(0, 0, _width, _height, fillcolor);
}

void ST7735::drawBitmap(u16 x, u16 y, u16 w, u16 h, uint16_t *p)
{
	setAddrWindow(x, y, w, h);
  	u16 n;
  	for(n = 0; n < w*h; n++){write16(p[n]);}
}

void ST7735::drawFastHLine(u16 x, u16 y, u16 w, u16 c)
{
	setAddrWindow(x, y, w, 1);
	while(w--){write16(c);}
}


void ST7735::drawFastVLine(u16 x, u16 y, u16 h, u16 c)
{
	setAddrWindow(x, y, 1, h);
	while(h--){write16(c);}
}


void ST7735::drawString(u16 x, u16 y, const char *s, u16 fc, u16 bc)
{
	byte *p;
	u16 xx, yy, c;
	while(*s){
		p = (byte *)&_vga_font8x8[*s * 8];
		setAddrWindow(x, y, 8, 8);
		for(yy = 0; yy < 8; yy++){
		for(xx = 0; xx < 8; xx++){
			c = (*p & (0x80>>xx)) ? fc : bc;
			write16(c);
		}
		p++;
		}
		s++;
		x += 8;
	}
}

