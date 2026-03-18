#ifndef __RADIO_H
#define __RADIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cmt2310a.h"
#include "typedefs.h"

typedef enum {
  RF_STATE_IDLE = 0,
  RF_STATE_RX_START,
  RF_STATE_RX_WAIT,
  RF_STATE_RX_DONE,
  RF_STATE_ERROR,
} EnumRFStatus;

typedef enum {
  RF_IDLE = 0,
  RF_BUSY,
  RF_RX_DONE,
  RF_ERROR,
} EnumRFResult;

void radio_init(void);
void rf_config(void);
void rf_start_rx(u8 buf[], u16 len, u32 timeout_ms);
EnumRFResult rf_process(void);

#ifdef __cplusplus
}
#endif

#endif /* __RADIO_H */