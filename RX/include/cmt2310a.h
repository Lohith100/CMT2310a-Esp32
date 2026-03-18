#ifndef __CMT2310A_H
#define __CMT2310A_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cmt2310a_defs.h"
#include "cmt2310a_hal.h"
#include "typedefs.h"


/* Auto-poll state transitions — keep enabled */
#define ENABLE_AUTO_SWITCH_CHIP_STATUS

/* Boot / state */
void cmt2310a_soft_reset(void);
void cmt2310a_powerup_boot(void);
void cmt2310a_select_reg_page(u8 page_sel);
BOOL cmt2310a_api_command_and_wait(u8 api_cmd);
u8 cmt2310a_get_chip_state(void);
BOOL cmt2310a_auto_switch_state(u8 go_cmd);
BOOL cmt2310a_go_sleep(void);
BOOL cmt2310a_go_ready(void);
BOOL cmt2310a_go_txfs(void);
BOOL cmt2310a_go_rxfs(void);
BOOL cmt2310a_go_tx(void);
BOOL cmt2310a_go_rx(void);

/* GPIO / interrupts */
void cmt2310a_config_gpio0(u8 gpio_sel);
void cmt2310a_config_gpio1(u8 gpio_sel);
void cmt2310a_config_gpio5(u8 gpio_sel);
void cmt2310a_config_interrupt(u8 int1_sel, u8 int2_sel);
void cmt2310a_set_interrupt_polar(u8 int1_polar, u8 int2_polar);
void cmt2310a_enable_interrupt_source_0(u8 en);
void cmt2310a_enable_interrupt_source_fifo(u8 en);
void cmt2310a_clear_interrupt_flag_0(u8 clr);
void cmt2310a_clear_interrupt_flag_25(u8 clr);
u8 cmt2310a_get_interrupt_flag_0(void);
u8 cmt2310a_get_interrupt_flag_26(void);
u8 cmt2310a_get_fifo_flag(void);

/* FIFO */
void cmt2310a_clear_tx_fifo(void);
void cmt2310a_clear_rx_fifo(void);

/* General */
void cmt2310a_select_pa_mode(u8 mode);
void cmt2310a_init(void);
void cmt2310a_config_page_regs(u8 page_sel, const u8 page_regs[], u8 len);

/* Xtal wait — must call before powerup_boot() */
void bRadioSetReg(u8 addr, u8 set_bits, u8 mask_bits);
void vRadioXoWaitCfg(u8 div_sel);

#ifdef __cplusplus
}
#endif

#endif /* __CMT2310A_H */