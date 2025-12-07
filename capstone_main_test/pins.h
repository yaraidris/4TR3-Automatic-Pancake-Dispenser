#pragma once
#include <Arduino.h>

// Step/Dir/Enable pins
constexpr uint8_t X_STEP = 22, X_DIR = 23, X_EN = 24;       // X Axis Motor
constexpr uint8_t Y_STEP = 28, Y_DIR = 29, Y_EN = 30;       // Y Axis Motor
constexpr uint8_t Z_STEP = 34, Z_DIR = 35, Z_EN = 36;       // Z Axis Motor
constexpr uint8_t VAL_STEP = 40, VAL_DIR = 41, VAL_EN = 42; // Valve Motor

// Optional shared enable pin for all drivers (set to 255 to disable)
constexpr uint8_t ENABLE_PIN = 255;

// If any axis moves the wrong way, flip its DIR here
constexpr bool X_DIR_INVERTED   = false;
constexpr bool Y_DIR_INVERTED   = false;
constexpr bool Z_DIR_INVERTED   = false;
constexpr bool VAL_DIR_INVERTED = false;