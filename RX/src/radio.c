/*
 * radio.c  —  TX-only radio driver.
 * Based on CMOSTEK AN237 official reference source.
 *
 * PURE C file — no C++ allowed here.
 * Debug output uses printf() which on ESP32 Arduino routes to USB Serial.
 * Do NOT add Serial.print / #include <Arduino.h> in this file.
 */
#include "radio.h"
#include "cmt2310a.h"
#include "cmt2310a_hal.h"
#include "cmt2310a_params.h"
#include <stdio.h>
#include <string.h>

/* ---- internal state ---- */
static EnumRFStatus g_state = RF_STATE_IDLE;
static u8 *g_rx_buf = NULL;
static u16 g_rx_len = 0;
static u32 g_rx_timeout = INFINITE;
static u32 g_rx_tick_start = 0;

/* DIV8 → crystal wait ~6624 µs, safest for 20 MHz XTAL */
#define RADIO_CGU_DIV8 2

/* ================================================================
 *  radio_init()
 *  Exact CMOSTEK structure:  GPIO init → chip soft-reset → rf_config
 * ============================================================== */
void radio_init(void) {
  cmt2310a_init_gpio();
  cmt2310a_init();
  rf_config();
}

/* ================================================================
 *  rf_config()
 *  AN237 correct boot sequence:
 *   1. Load page regs          (firmware dormant, regs writable)
 *   2. vRadioXoWaitCfg(DIV8)   (set Xtal wait BEFORE boot)
 *   3. powerup_boot()          (start internal firmware)
 *   4. go_ready()
 *   5. DC-offset cal (0x02)    (blocks until chip confirms)
 *   6. IR cal (0x01)           (blocks until chip confirms)
 *   7. GPIO / interrupt config
 * ============================================================== */
void rf_config(void) {
  /* 1. page registers */
  cmt2310a_config_page_regs(0, g_cmt2310a_page0, CMT2310A_PAGE0_SIZE);
  cmt2310a_config_page_regs(1, g_cmt2310a_page1, CMT2310A_PAGE1_SIZE);

  /* 2. Xtal wait — THIS WAS THE MISSING CALL causing silent PLL fail */
  vRadioXoWaitCfg(RADIO_CGU_DIV8);

  /* 3. Boot */
  cmt2310a_powerup_boot();
  cmt2310a_delay_ms(5);

  /* 4. READY */
  cmt2310a_go_ready();
  cmt2310a_delay_ms(2);

  /* 5. DC-offset calibration */
  cmt2310a_api_command_and_wait(0x02);
  cmt2310a_delay_ms(2);

  /* 6. IR calibration */
  cmt2310a_api_command_and_wait(0x01);

  /* 7. GPIO0 = INT1 (TX_DONE),  GPIO1 = INT2 (PKT_DONE) */
  cmt2310a_config_gpio0(GPIO0_SEL_INT1);
  cmt2310a_config_gpio1(GPIO1_SEL_INT2);

  cmt2310a_config_interrupt(INT_SRC_TX_DONE, INT_SRC_PKT_DONE);
  cmt2310a_set_interrupt_polar(0, 0); /* active-high for both */

  cmt2310a_enable_interrupt_source_0(M_TX_DONE_EN | M_PREAM_PASS_EN |
                                     M_SYNC_PASS_EN | M_ADDR_PASS_EN |
                                     M_CRC_PASS_EN | M_PKT_DONE_EN);

  cmt2310a_enable_interrupt_source_fifo(
      M_RX_FIFO_FULL_RX_EN | M_RX_FIFO_NMTY_RX_EN | M_RX_FIFO_TH_RX_EN |
      M_RX_FIFO_OVF_EN | M_TX_FIFO_FULL_EN | M_TX_FIFO_NMTY_EN |
      M_TX_FIFO_TH_EN);

  cmt2310a_select_pa_mode(1); /* 1 = differential PA for +20dBm */

  printf("[rf_config] done  state=0x%02X\n", cmt2310a_get_chip_state());
}

/* ================================================================
 *  rf_start_rx()
 *  Store params, arm state machine. Hardware work in rf_process().
 * ============================================================== */
void rf_start_rx(u8 buf[], u16 len, u32 timeout_ms) {
  g_rx_buf = buf;
  g_rx_len = len;
  g_rx_timeout = timeout_ms;
  g_state = RF_STATE_RX_START;
}

/* ================================================================
 *  rf_process()  —  call every loop() iteration
 * ============================================================== */
EnumRFResult rf_process(void) {
  EnumRFResult result = RF_BUSY;

  switch (g_state) {
  /* ---- idle ---- */
  case RF_STATE_IDLE:
    result = RF_IDLE;
    break;

  /* ----------------------------------------------------------------
   *  RX_START
   * -------------------------------------------------------------- */
  case RF_STATE_RX_START: {
    cmt2310a_go_ready();
    cmt2310a_delay_ms(2);

    cmt2310a_clear_interrupt_flag_25(M_PKT_DONE_CLR);
    cmt2310a_clear_rx_fifo();

    if (FALSE == cmt2310a_go_rx()) {
      printf("[RX_START] go_rx() failed  state=0x%02X\n",
             cmt2310a_get_chip_state());
      g_state = RF_STATE_ERROR;
    } else {
      g_state = RF_STATE_RX_WAIT;
      g_rx_tick_start = cmt2310a_get_tick_count();
    }
    break;
  }

  /* ----------------------------------------------------------------
   *  RX_WAIT
   *  Poll PKT_DONE flag via SPI
   * -------------------------------------------------------------- */
  case RF_STATE_RX_WAIT: {
    if (M_PKT_DONE_FLG & cmt2310a_get_interrupt_flag_26()) {
      g_state = RF_STATE_RX_DONE;
      break;
    }
    if ((INFINITE != g_rx_timeout) &&
        ((cmt2310a_get_tick_count() - g_rx_tick_start) > g_rx_timeout)) {
      u8 st = cmt2310a_get_chip_state();
      printf("[RX_WAIT] timeout  state=0x%02X\n", st);
      g_state = RF_STATE_ERROR;
    }
    break;
  }

  /* ---- RX_DONE ---- */
  case RF_STATE_RX_DONE:
    /* Read exact length from FIFO */
    cmt2310a_read_fifo(g_rx_buf, g_rx_len);
    cmt2310a_go_ready();
    cmt2310a_delay_ms(2);
    g_state = RF_STATE_IDLE;
    result = RF_RX_DONE;
    break;

  /* ---- ERROR — re-init and recover ---- */
  case RF_STATE_ERROR:
    printf("[RF_ERROR] re-initialising\n");
    radio_init();
    g_state = RF_STATE_IDLE;
    result = RF_ERROR;
    break;

  default:
    break;
  }

  return result;
}