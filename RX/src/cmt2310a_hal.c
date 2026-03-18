/*
 * cmt2310a_hal.c  —  Arduino / ESP32 HAL implementation.
 * Only change from CMOSTEK original: cmt_spi4_init() (which does not exist
 * in bit-bang SPI) is replaced with explicit GPIO direction + idle-state setup.
 */
#include "cmt2310a_hal.h"
#include "cmt2310a_defs.h"

void cmt2310a_init_gpio(void)
{
    /* Interrupt inputs from chip */
    cmt2310a_set_gipo0_in();
    cmt2310a_set_gipo1_in();
    cmt2310a_set_nirq_in();

    /* SPI bus — set directions and safe idle levels */
    cmt_spi4_csb_out();  cmt_spi4_csb_1();   /* CS  idle HIGH */
    cmt_spi4_scl_out();  cmt_spi4_scl_0();   /* CLK idle LOW  */
    cmt_spi4_mosi_out(); cmt_spi4_mosi_1();  /* MOSI idle HIGH*/
    cmt_spi4_miso_in();
}

u8 cmt2310a_read_reg(u8 addr)
{
    u8 dat = 0xFF;
    cmt_spi4_read(addr, &dat, 1);
    return dat;
}

void cmt2310a_write_reg(u8 addr, u8 dat)
{
    cmt_spi4_write(addr, &dat, 1);
}

void cmt2310a_set_reg_bits(u8 addr, u8 dat, u8 bits_mask)
{
    u8 reg_val = cmt2310a_read_reg(addr);
    reg_val &= ~bits_mask;
    reg_val |= (dat & bits_mask);
    cmt_spi4_write(addr, &reg_val, 1);
}

void cmt2310a_read_regs(u8 addr, u8 buf[], u16 len)
{
    u16 i;
    for (i = 0; i < len; i++) { buf[i] = cmt2310a_read_reg(addr); addr++; }
}

void cmt2310a_write_regs(u8 addr, const u8 buf[], u16 len)
{
    u16 i;
    for (i = 0; i < len; i++) { cmt2310a_write_reg(addr, buf[i]); addr++; }
}

/* Bulk page load/dump via auto-increment CRW port */
void cmt2310a_batch_read_regs(u8 buf[], u16 len)
{
    cmt_spi4_read(CMT2310A_CRW_PORT, buf, len);
}

void cmt2310a_batch_write_regs(const u8 buf[], u16 len)
{
    cmt_spi4_write(CMT2310A_CRW_PORT, buf, len);
}

void cmt2310a_read_fifo(u8 buf[], u16 len)
{
    cmt_spi4_read(CMT2310A_FIFO_PORT, buf, len);
}

void cmt2310a_write_fifo(const u8 buf[], u16 len)
{
    cmt_spi4_write(CMT2310A_FIFO_PORT, buf, len);
}