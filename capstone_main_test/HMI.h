#pragma once
#include <Arduino.h>

// Initialize Nextion + buttons
void HMI_init();

// Poll for touch events (must be called frequently)
void HMI_poll();

// High-level requests from HMI
bool HMI_batchRequested();
bool HMI_squareRequested();
bool HMI_triangleRequested();
uint8_t HMI_getBatchNumber();

// Stop flag (any stop button)
bool HMI_stopRequested();

// Clear latched flags
void HMI_clearDesignRequests();
void HMI_clearStop();