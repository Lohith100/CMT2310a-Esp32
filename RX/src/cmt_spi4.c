/*
 * cmt_spi4.c  —  Bit-bang SPI for CMT2310A on Arduino / ESP32
 * CPOL=0, CPHA=0, MSB first, ~500 kHz (well within 10 MHz max)
 */
#include "cmt_spi4.h"

/* one clock half-period */
static inline void spi_delay(void) { delayMicroseconds(1); }

/* send one byte, MSB first */
static void spi_send_byte(u8 d)
{
    u8 i;
    for (i = 0; i < 8; i++) {
        cmt_spi4_scl_0();
        if (d & 0x80) cmt_spi4_mosi_1(); else cmt_spi4_mosi_0();
        spi_delay();
        cmt_spi4_scl_1();
        spi_delay();
        d <<= 1;
    }
    cmt_spi4_scl_0();
}

/* receive one byte, MSB first */
static u8 spi_recv_byte(void)
{
    u8 i, d = 0;
    for (i = 0; i < 8; i++) {
        cmt_spi4_scl_0();
        spi_delay();
        d <<= 1;
        cmt_spi4_scl_1();
        if (cmt_spi4_miso_read()) d |= 0x01;
        spi_delay();
    }
    cmt_spi4_scl_0();
    return d;
}

/*
 * cmt_spi4_write — write len bytes to addr
 * addr bit7 = 0 means write on CMT2310A
 */
void cmt_spi4_write(u8 addr, const u8 *buf, u16 len)
{
    u16 i;
    noInterrupts();
    cmt_spi4_csb_0();
    spi_delay();
    spi_send_byte(addr & 0x7F);   /* bit7=0 → write */
    for (i = 0; i < len; i++) spi_send_byte(buf[i]);
    cmt_spi4_csb_1();
    cmt_spi4_mosi_1();
    interrupts();
}

/*
 * cmt_spi4_read — read len bytes from addr
 * addr bit7 = 1 means read on CMT2310A
 */
void cmt_spi4_read(u8 addr, u8 *buf, u16 len)
{
    u16 i;
    noInterrupts();
    cmt_spi4_csb_0();
    spi_delay();
    spi_send_byte(addr | 0x80);   /* bit7=1 → read */
    for (i = 0; i < len; i++) buf[i] = spi_recv_byte();
    cmt_spi4_csb_1();
    cmt_spi4_mosi_1();
    interrupts();
}