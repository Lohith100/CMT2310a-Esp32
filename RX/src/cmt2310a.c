/*
 * cmt2310a.c  —  CMOSTEK official driver, adapted for Arduino / ESP32.
 * All register/macro names taken directly from cmt2310a_defs.h.
 */
#include "cmt2310a.h"

void cmt2310a_soft_reset(void) { cmt2310a_write_reg(CMT2310A_SOFT_RST, 0xFF); }

void cmt2310a_powerup_boot(void) {
  /* M_BOOT_MAIN | M_POWERUP = 0x02 | 0x01 */
  cmt2310a_write_reg(CMT2310A_CTL_REG_00, 0x03);
}

void cmt2310a_select_reg_page(u8 page_sel) {
  cmt2310a_set_reg_bits(CMT2310A_CTL_REG_126, (u8)(page_sel << CMT_BIT6),
                        M_HV_PAGE_SEL);
}

/*
 * Send internal API command and POLL until done.
 * Previous version did not wait — calibration never completed.
 */
BOOL cmt2310a_api_command_and_wait(u8 api_cmd) {
  u8 state;
  u32 deadline = cmt2310a_get_tick_count() + 2000;
  u8 expected = (u8)(M_API_CMD_FLAG | api_cmd);

  cmt2310a_write_reg(CMT2310A_CTL_REG_08, api_cmd);
  while (cmt2310a_get_tick_count() < deadline) {
    cmt2310a_delay_ms(2);
    state = cmt2310a_read_reg(CMT2310A_CTL_REG_09);
    if (state == expected)
      return TRUE;
  }
  return FALSE;
}

u8 cmt2310a_get_chip_state(void) {
  return cmt2310a_read_reg(CMT2310A_CTL_REG_10);
}

BOOL cmt2310a_switch_state_and_wait(u8 go_cmd, u8 want_state, u32 deadline) {
  cmt2310a_write_reg(CMT2310A_CTL_REG_01, go_cmd);
  while (cmt2310a_get_tick_count() < deadline) {
    cmt2310a_delay_us(20);
    if (cmt2310a_get_chip_state() == want_state)
      return TRUE;
  }
  return FALSE;
}

BOOL cmt2310a_auto_switch_state(u8 go_cmd) {
#ifdef ENABLE_AUTO_SWITCH_CHIP_STATUS
  u8 want;
  u32 deadline = cmt2310a_get_tick_count() + 10;
  switch (go_cmd) {
  case M_GO_RXFS:
    want = STATE_IS_RFS;
    break;
  case M_GO_TXFS:
    want = STATE_IS_TFS;
    break;
  case M_GO_RX:
    want = STATE_IS_RX;
    break;
  case M_GO_TX:
    want = STATE_IS_TX;
    break;
  case M_GO_READY:
    want = STATE_IS_READY;
    break;
  case M_GO_SLEEP:
    want = STATE_IS_SLEEP;
    break;
  default:
    want = STATE_IS_IDLE;
    break;
  }
  return cmt2310a_switch_state_and_wait(go_cmd, want, deadline);
#else
  cmt2310a_write_reg(CMT2310A_CTL_REG_01, go_cmd);
  return TRUE;
#endif
}

BOOL cmt2310a_go_sleep(void) { return cmt2310a_auto_switch_state(M_GO_SLEEP); }
BOOL cmt2310a_go_ready(void) { return cmt2310a_auto_switch_state(M_GO_READY); }
BOOL cmt2310a_go_txfs(void) { return cmt2310a_auto_switch_state(M_GO_TXFS); }
BOOL cmt2310a_go_rxfs(void) { return cmt2310a_auto_switch_state(M_GO_RXFS); }
BOOL cmt2310a_go_tx(void) { return cmt2310a_auto_switch_state(M_GO_TX); }
BOOL cmt2310a_go_rx(void) { return cmt2310a_auto_switch_state(M_GO_RX); }

void cmt2310a_config_gpio0(u8 gpio_sel) {
  cmt2310a_set_reg_bits(CMT2310A_CTL_REG_04, (u8)(gpio_sel << CMT_BIT0),
                        M_GPIO0_SEL);
}

void cmt2310a_config_gpio1(u8 gpio_sel) {
  cmt2310a_set_reg_bits(CMT2310A_CTL_REG_04, (u8)(gpio_sel << CMT_BIT3),
                        M_GPIO1_SEL);
}

void cmt2310a_config_gpio5(u8 gpio_sel) {
  cmt2310a_set_reg_bits(CMT2310A_CTL_REG_06, (u8)(gpio_sel << CMT_BIT3),
                        M_GPIO5_SEL);
}

void cmt2310a_config_interrupt(u8 int1_sel, u8 int2_sel) {
  cmt2310a_set_reg_bits(CMT2310A_CTL_REG_16, int1_sel, M_INT1_SEL);
  cmt2310a_set_reg_bits(CMT2310A_CTL_REG_17, int2_sel, M_INT2_SEL);
}

void cmt2310a_set_interrupt_polar(u8 int1_polar, u8 int2_polar) {
  cmt2310a_set_reg_bits(
      CMT2310A_CTL_REG_17,
      (u8)((int1_polar << CMT_BIT7) | (int2_polar << CMT_BIT6)),
      (u8)(M_INT1_POLAR | M_INT2_POLAR));
}

void cmt2310a_enable_interrupt_source_0(u8 en) {
  cmt2310a_write_reg(CMT2310A_CTL_REG_18, en);
}

void cmt2310a_enable_interrupt_source_fifo(u8 en) {
  cmt2310a_write_reg(CMT2310A_CTL_REG_14, en);
}

void cmt2310a_clear_interrupt_flag_0(u8 clr) {
  cmt2310a_write_reg(CMT2310A_CTL_REG_24, clr);
}

void cmt2310a_clear_interrupt_flag_25(u8 clr) {
  cmt2310a_write_reg(CMT2310A_CTL_REG_25, clr);
}

u8 cmt2310a_get_interrupt_flag_0(void) {
  return cmt2310a_read_reg(CMT2310A_CTL_REG_24);
}

u8 cmt2310a_get_interrupt_flag_26(void) {
  return cmt2310a_read_reg(CMT2310A_CTL_REG_26);
}

u8 cmt2310a_get_fifo_flag(void) {
  return cmt2310a_read_reg(CMT2310A_CTL_REG_28);
}

void cmt2310a_clear_tx_fifo(void) {
  cmt2310a_set_reg_bits(CMT2310A_CTL_REG_27, M_TX_FIFO_CLR, M_TX_FIFO_CLR);
}

void cmt2310a_clear_rx_fifo(void) {
  cmt2310a_set_reg_bits(CMT2310A_CTL_REG_27, M_RX_FIFO_CLR, M_RX_FIFO_CLR);
}

void cmt2310a_select_pa_mode(u8 mode) {
  cmt2310a_set_reg_bits(CMT2310A_CTL_REG_22, (u8)(mode << CMT_BIT3),
                        M_PA_DIFF_SEL);
}

void cmt2310a_init(void) {
  cmt2310a_soft_reset();
  cmt2310a_delay_ms(20);
}

void cmt2310a_config_page_regs(u8 page_sel, const u8 page_regs[], u8 len) {
  cmt2310a_select_reg_page(page_sel);
  cmt2310a_batch_write_regs(page_regs, len);
  cmt2310a_select_reg_page(0); /* always return to page 0 */
}

/* ---------------------------------------------------------------
 *  Xtal oscillator wait-time helper.
 *  MUST be called AFTER loading page regs, BEFORE powerup_boot().
 *  Without this, PLL silently fails to lock → no RF output.
 *
 *  div_sel 0=DIV1 (~1344 µs), 1=DIV4 (~3972 µs), 2=DIV8 (~6624 µs)
 * ------------------------------------------------------------- */
void bRadioSetReg(u8 addr, u8 set_bits, u8 mask_bits) {
  u8 tmp = cmt2310a_read_reg(addr);
  tmp &= ~mask_bits;
  tmp |= (set_bits & mask_bits);
  cmt2310a_write_reg(addr, tmp);
}

void vRadioXoWaitCfg(u8 div_sel) {
  u8 val = (u8)((div_sel & 0x03) << 6);
  cmt2310a_select_reg_page(0);
  bRadioSetReg(CMT2310A_CTL_REG_07, val, CMT2310A_CTL_REG_07_MASK);
  cmt2310a_select_reg_page(0);
}