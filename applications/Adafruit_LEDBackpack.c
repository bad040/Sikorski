/*************************************************** 
  This is a library for our I2C LED Backpacks

  Designed specifically to work with the Adafruit LED Matrix backpacks 
  ----> http://www.adafruit.com/products/
  ----> http://www.adafruit.com/products/

  These displays use I2C to communicate, 2 pins are required to 
  interface. There are multiple selectable I2C addresses. For backpacks
  with 2 Address Select pins: 0x70, 0x71, 0x72 or 0x73. For backpacks
  with 3 Address Select pins: 0x70 thru 0x77

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  MIT license, all text above must be included in any redistribution


Modifications Copyright Claroworks.

 ****************************************************/

#include "app.h"
#include "ch.h"
#include "hal.h"

#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

#ifndef _BV
  #define _BV(bit) (1<<(bit))
#endif

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif

uint16_t displaybuffer[8];

uint8_t rxbuf[2];
uint8_t txbuf[20];
msg_t status = MSG_OK;
systime_t tmo = MS2ST(5);
i2caddr_t i2caddr = 0x70;

void LED_begin(void) {

	hw_start_i2c();
	chThdSleepMilliseconds(10);

	txbuf[0] = 0x21;
	i2cAcquireBus(&HW_I2C_DEV);
	i2cMasterTransmitTimeout(&HW_I2C_DEV, i2caddr, txbuf, 1, rxbuf, 0, tmo);
	i2cReleaseBus(&HW_I2C_DEV);

	LED_blinkRate(HT16K33_BLINK_OFF);
	LED_setBrightness(15); // max brightness
}

void LED_setBrightness(uint8_t b) {
	if (b > 15)
		b = 15;

	txbuf[0] = HT16K33_CMD_BRIGHTNESS | b;
	i2cAcquireBus(&HW_I2C_DEV);
	i2cMasterTransmitTimeout(&HW_I2C_DEV, i2caddr, txbuf, 1, rxbuf, 0, tmo);
	i2cReleaseBus(&HW_I2C_DEV);
}

void LED_blinkRate(uint8_t b) {
	if (b > 3)
		b = 0;

	txbuf[0] = HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | (b << 1);
	i2cAcquireBus(&HW_I2C_DEV);
	i2cMasterTransmitTimeout(&HW_I2C_DEV, i2caddr, txbuf, 1, rxbuf, 0, tmo);
	i2cReleaseBus(&HW_I2C_DEV);
}

void LED_writeDisplay(void) {
	int j =0;
	txbuf[j++] = 0; // display addr 0
	for (uint8_t i=0; i<8; i++) {
		txbuf[j++] = displaybuffer[i] & 0xFF;
		txbuf[j++] = displaybuffer[i] >> 8;
	}

	i2cAcquireBus(&HW_I2C_DEV);
	i2cMasterTransmitTimeout(&HW_I2C_DEV, i2caddr, txbuf, j, rxbuf, 0, tmo);
	i2cReleaseBus(&HW_I2C_DEV);
}

void LED_clear(void) {
	for (uint8_t i=0; i<8; i++) {
		displaybuffer[i] = 0;
	}
}


/******************************* 8x8 MATRIX OBJECT */

void LED_drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((y < 0) || (y >= 8)) return;
  if ((x < 0) || (x >= 8)) return;

 // check rotation, move pixel around if necessary
  switch (GFX_getRotation()) {
  case 1:
    _swap_int16_t(x, y);
    x = 8 - x - 1;
    break;
  case 2:
    x = 8 - x - 1;
    y = 8 - y - 1;
    break;
  case 3:
    _swap_int16_t(x, y);
    y = 8 - y - 1;
    break;
  }

  // wrap around the x
  x += 7;
  x %= 8;


  if (color) {
    displaybuffer[y] |= 1 << x;
  } else {
    displaybuffer[y] &= ~(1 << x);
  }
}


