/*
 * cmt2310a_hal.h  —  Arduino / ESP32 adaptation of the CMOSTEK HAL header.
 * Replaces gpio_defs.h, common.h, system.h (which are MCU-specific and do
 * not exist on Arduino) with Arduino built-in equivalents.
 */
#ifndef __CMT2310A_HAL_H
#define __CMT2310A_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "typedefs.h"
#include "cmt_spi4.h"

/* ---- Interrupt input pins ---- */
#define cmt2310a_set_gipo0_in()      SET_GPIO_IN(CMT_GPIO0_GPIO)
#define cmt2310a_set_gipo1_in()      SET_GPIO_IN(CMT_GPIO1_GPIO)
#define cmt2310a_set_nirq_in()       SET_GPIO_IN(CMT_NIRQ_GPIO)
#define cmt2310a_read_gpio0()        READ_GPIO_PIN(CMT_GPIO0_GPIO)
#define cmt2310a_read_gpio1()        READ_GPIO_PIN(CMT_GPIO1_GPIO)
#define cmt2310a_read_nirq()         READ_GPIO_PIN(CMT_NIRQ_GPIO)

/* ---- Timing ---- */
#define cmt2310a_delay_ms(ms)        delay(ms)
#define cmt2310a_delay_us(us)        delayMicroseconds(us)
#define cmt2310a_get_tick_count()    millis()

/* ---- Prototypes ---- */
void cmt2310a_init_gpio(void);
u8   cmt2310a_read_reg(u8 addr);
void cmt2310a_write_reg(u8 addr, u8 dat);
void cmt2310a_set_reg_bits(u8 addr, u8 dat, u8 bits_mask);
void cmt2310a_read_regs(u8 addr, u8 buf[], u16 len);
void cmt2310a_write_regs(u8 addr, const u8 buf[], u16 len);
void cmt2310a_batch_read_regs(u8 buf[], u16 len);
void cmt2310a_batch_write_regs(const u8 buf[], u16 len);
void cmt2310a_read_fifo(u8 buf[], u16 len);
void cmt2310a_write_fifo(const u8 buf[], u16 len);

#ifdef __cplusplus
}
#endif

#endif /* __CMT2310A_HAL_H */