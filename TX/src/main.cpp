
#include <Arduino.h>
#include <string.h>

extern "C" {
#include "radio.h"
}

static uint32_t pktCount = 0;
static uint8_t txMsg[32];

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("====================================");
  Serial.println(" CMT2310A TX Test  |  868.000 MHz  ");
  Serial.println("====================================");

  /* GPIO init -> soft-reset -> rf_config() */
  radio_init();

  /* Queue the very first packet immediately */
  pktCount++;
  snprintf((char *)txMsg, sizeof(txMsg), "HACKRF_TEST:868MHZ:%lu", pktCount);
  Serial.printf("[TX] Queuing: \"%s\" (%d bytes)\n", txMsg, sizeof(txMsg));
  rf_start_tx(txMsg, sizeof(txMsg), 500);
}

void loop() {
  static EnumRFResult lastRes = RF_IDLE;

  // Process the radio state machine continuously
  EnumRFResult res = rf_process();

  // Only react on the RISING EDGE of RF_TX_DONE to avoid double-counting
  // (rf_process can return RF_TX_DONE for multiple consecutive iterations)
  if (res == RF_TX_DONE && lastRes != RF_TX_DONE) {
    pktCount++;
    snprintf((char *)txMsg, sizeof(txMsg), "HACKRF_TEST:868MHZ:%lu", pktCount);
    Serial.printf("[TX] #%lu sent — queuing next\n", pktCount - 1);
    rf_start_tx(txMsg, sizeof(txMsg), 500);
  } else if (res == RF_TX_TIMEOUT) {
    Serial.println("[ERR] TX Timeout! Retrying...");
    rf_start_tx(txMsg, sizeof(txMsg), 500); // retry same packet
  } else if (res == RF_ERROR) {
    Serial.println("[ERR] RF_ERROR — driver re-initialised");
  }

  lastRes = res;
}