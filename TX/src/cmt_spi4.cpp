/*
 * cmt_spi4.c  —  Hardware SPI for CMT2310A on ESP32 (Arduino framework)
 * CPOL=0, CPHA=0  (SPI Mode 0), MSB first.
 * CMT2310A max SPI clock: 10 MHz  →  using 4 MHz for margin.
 *
 * Pin mapping (matches cmt_spi4.h):
 *   CS   = GPIO 7   (CMT_CSB_GPIO)
 *   SCLK = GPIO 6   (CMT_SCLK_GPIO)
 *   MOSI = GPIO 10  (CMT_MOSI_GPIO)
 *   MISO = GPIO 2   (CMT_MISO_GPIO)
 */

/* ============================================================
 * OLD BIT-BANG IMPLEMENTATION — commented out, kept for reference
 * ============================================================
 *
 * #include "cmt_spi4.h"
 *
 * // one clock half-period
 * static inline void spi_delay(void) { delayMicroseconds(1); }
 *
 * // send one byte, MSB first
 * static void spi_send_byte(u8 d)
 * {
 *     u8 i;
 *     for (i = 0; i < 8; i++) {
 *         cmt_spi4_scl_0();
 *         if (d & 0x80) cmt_spi4_mosi_1(); else cmt_spi4_mosi_0();
 *         spi_delay();
 *         cmt_spi4_scl_1();
 *         spi_delay();
 *         d <<= 1;
 *     }
 *     cmt_spi4_scl_0();
 * }
 *
 * // receive one byte, MSB first
 * static u8 spi_recv_byte(void)
 * {
 *     u8 i, d = 0;
 *     for (i = 0; i < 8; i++) {
 *         cmt_spi4_scl_0();
 *         spi_delay();
 *         d <<= 1;
 *         cmt_spi4_scl_1();
 *         if (cmt_spi4_miso_read()) d |= 0x01;
 *         spi_delay();
 *     }
 *     cmt_spi4_scl_0();
 *     return d;
 * }
 *
 * // cmt_spi4_write — write len bytes to addr
 * // addr bit7 = 0 means write on CMT2310A
 * void cmt_spi4_write(u8 addr, const u8 *buf, u16 len)
 * {
 *     u16 i;
 *     noInterrupts();
 *     cmt_spi4_csb_0();
 *     spi_delay();
 *     spi_send_byte(addr & 0x7F);   // bit7=0 → write
 *     for (i = 0; i < len; i++) spi_send_byte(buf[i]);
 *     cmt_spi4_csb_1();
 *     cmt_spi4_mosi_1();
 *     interrupts();
 * }
 *
 * // cmt_spi4_read — read len bytes from addr
 * // addr bit7 = 1 means read on CMT2310A
 * void cmt_spi4_read(u8 addr, u8 *buf, u16 len)
 * {
 *     u16 i;
 *     noInterrupts();
 *     cmt_spi4_csb_0();
 *     spi_delay();
 *     spi_send_byte(addr | 0x80);   // bit7=1 → read
 *     for (i = 0; i < len; i++) buf[i] = spi_recv_byte();
 *     cmt_spi4_csb_1();
 *     cmt_spi4_mosi_1();
 *     interrupts();
 * }
 * ============================================================ */

/* ============================================================
 * HARDWARE SPI IMPLEMENTATION
 * ============================================================ */
#include "cmt_spi4.h"
#include <SPI.h>

/* 4 MHz — well within 10 MHz CMT2310A max, and handles long wires */
#define CMT_SPI_FREQ 4000000UL
#define CMT_SPI_MODE SPI_MODE0 /* CPOL=0, CPHA=0 */

/*
 * cmt_spi4_init — call once from cmt2310a_init_gpio() after setting CS high.
 * Configures the hardware SPI peripheral on custom pins.
 * NOTE: ESP32-C3 has no VSPI/HSPI constants — use the global SPI object
 *       which maps to the single user-accessible SPI bus.
 */
void cmt_spi4_init(void) {
  /* ----- TEST: default SS pin -----
   * Using the framework's built-in SS constant instead of CMT_CSB_GPIO.
   * Old custom-pin init kept below as comment for easy revert. */
  // SPI.begin(CMT_SCLK_GPIO,   /* SCK  */
  //           CMT_MISO_GPIO,   /* MISO */
  //           CMT_MOSI_GPIO,   /* MOSI */
  //           SS);             /* SS   — default framework CS pin (TEST) */

  // OLD — custom CS pin:
  SPI.begin(CMT_SCLK_GPIO, CMT_MISO_GPIO, CMT_MOSI_GPIO, CMT_CSB_GPIO);
  pinMode(CMT_CSB_GPIO, OUTPUT);
  digitalWrite(CMT_CSB_GPIO, HIGH);

  /* Drive the default SS pin HIGH (idle) */
  // pinMode(SS, OUTPUT);
  // digitalWrite(SS, HIGH);
}

/* ---- internal helpers ---- */

/* TEST: use default SS pin for CS */
// static inline void cs_low(void) { digitalWrite(SS, LOW); }
// static inline void cs_high(void) { digitalWrite(SS, HIGH); }

// OLD — custom CS pin:
static inline void cs_low(void)  { digitalWrite(CMT_CSB_GPIO, LOW);  }
static inline void cs_high(void) { digitalWrite(CMT_CSB_GPIO, HIGH); }

/*
 * cmt_spi4_write — write len bytes to addr
 * CMT2310A protocol: first byte = addr with bit7 cleared (write)
 */
void cmt_spi4_write(u8 addr, const u8 *buf, u16 len) {
  u16 i;
  SPI.beginTransaction(SPISettings(CMT_SPI_FREQ, MSBFIRST, CMT_SPI_MODE));
  cs_low();
  SPI.transfer(addr & 0x7F); /* bit7 = 0 → write */
  for (i = 0; i < len; i++) {
    SPI.transfer(buf[i]);
  }
  cs_high();
  SPI.endTransaction();
}

/*
 * cmt_spi4_read — read len bytes from addr
 * CMT2310A protocol: first byte = addr with bit7 set (read)
 */
void cmt_spi4_read(u8 addr, u8 *buf, u16 len) {
  u16 i;
  SPI.beginTransaction(SPISettings(CMT_SPI_FREQ, MSBFIRST, CMT_SPI_MODE));
  cs_low();
  SPI.transfer(addr | 0x80); /* bit7 = 1 → read */
  for (i = 0; i < len; i++) {
    buf[i] = SPI.transfer(0x00);
  }
  cs_high();
  SPI.endTransaction();
}