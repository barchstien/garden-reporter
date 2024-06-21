#pragma once

#include "epoch_time_t.h"

#define LOG(X) ({ \
  Serial.print(epoch_time_sync.now()); \
  Serial.print(" - "); \
  Serial.print(X); \
})