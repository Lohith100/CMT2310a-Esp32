#include <Arduino.h>
#include <string.h>

extern "C" {
#include "radio.h"
}

static uint32_t rxPktCount = 0;
static uint8_t rxMsg[64];

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("====================================");
  Serial.println(" CMT2310A RX Test  |  868.000 MHz  ");
  Serial.println("====================================");

  /* GPIO init -> soft-reset -> rf_config() */
  radio_init();

  Serial.println("[SETUP] Ready. Listening for packets...");

  // Start listening indefinitely
  rf_start_rx(rxMsg, sizeof(rxMsg), INFINITE);
}

void loop() {
  // Process the radio state machine continuously
  EnumRFResult res = rf_process();

  if (res == RF_RX_DONE) {
    rxPktCount++;
    // Null terminate just in case
    rxMsg[sizeof(rxMsg) - 1] = '\0';
    Serial.printf("[RX] Packet %lu Received: \"%s\"\n", rxPktCount,
                  (char *)rxMsg);

    // Restart listening
    rf_start_rx(rxMsg, sizeof(rxMsg), INFINITE);
  }

  // We only print on timeout and error to avoid making the serial monitor crash
  if (res == RF_ERROR) {
    Serial.println("[ERR] RF_ERROR — driver re-initialised");
    // Restart listening
    rf_start_rx(rxMsg, sizeof(rxMsg), INFINITE);
  }
}