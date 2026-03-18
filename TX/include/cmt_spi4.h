#ifndef __CMT_SPI4_H
#define __CMT_SPI4_H

/* Arduino.h brings in C++ headers — must be outside extern "C" */
#include <Arduino.h>
#include "typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 *  Pin assignments — change these to match YOUR wiring
 * ============================================================== */
#define CMT_CSB_GPIO    7    /* SPI Chip Select  (active LOW) */
#define CMT_SCLK_GPIO   6    /* SPI Clock                    */
#define CMT_MOSI_GPIO   10   /* SPI MOSI                     */
#define CMT_MISO_GPIO   2    /* SPI MISO                     */
#define CMT_GPIO0_GPIO  3    /* INT1 output from chip        */
#define CMT_GPIO1_GPIO  4    /* INT2 output from chip        */
#define CMT_NIRQ_GPIO   5    /* NIRQ — tie to 3V3 if unused  */

/* ---- Generic GPIO helpers ---- */
#define SET_GPIO_OUT(pin)    pinMode((pin), OUTPUT)
#define SET_GPIO_IN(pin)     pinMode((pin), INPUT)
#define READ_GPIO_PIN(pin)   digitalRead((pin))

/* ---- SPI bus control ---- */
#define cmt_spi4_csb_out()   SET_GPIO_OUT(CMT_CSB_GPIO)
#define cmt_spi4_scl_out()   SET_GPIO_OUT(CMT_SCLK_GPIO)
#define cmt_spi4_mosi_out()  SET_GPIO_OUT(CMT_MOSI_GPIO)
#define cmt_spi4_miso_in()   SET_GPIO_IN(CMT_MISO_GPIO)

#define cmt_spi4_csb_1()     digitalWrite(CMT_CSB_GPIO,  HIGH)
#define cmt_spi4_csb_0()     digitalWrite(CMT_CSB_GPIO,  LOW)
#define cmt_spi4_scl_1()     digitalWrite(CMT_SCLK_GPIO, HIGH)
#define cmt_spi4_scl_0()     digitalWrite(CMT_SCLK_GPIO, LOW)
#define cmt_spi4_mosi_1()    digitalWrite(CMT_MOSI_GPIO, HIGH)
#define cmt_spi4_mosi_0()    digitalWrite(CMT_MOSI_GPIO, LOW)
#define cmt_spi4_miso_read() READ_GPIO_PIN(CMT_MISO_GPIO)

/* ---- Function prototypes ---- */
void cmt_spi4_init (void);                            /* init hardware SPI */
void cmt_spi4_write(u8 addr, const u8 *buf, u16 len);
void cmt_spi4_read (u8 addr,       u8 *buf, u16 len);

#ifdef __cplusplus
}
#endif

#endif /* __CMT_SPI4_H */