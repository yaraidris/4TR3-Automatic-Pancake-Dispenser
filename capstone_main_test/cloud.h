#pragma once
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiEspAT.h>
#include "Adafruit_VL53L0X.h"

#ifndef HAVE_HWSERIAL1
#include <SoftwareSerial.h>
static SoftwareSerial Serial1(18, 19); // RX, TX
#endif

// ======== MQTT + WiFi Config ========
static const char *brokerAddress = "test.mosquitto.org";
static const char *mqtt_topic    = "4ID3_Group3/dispense";
static uint16_t    addressPort   = 1883;
static const char *clientID      = "ESP01";
static const char *mqtt_username = "";
static const char *mqtt_password = "";

static char ssid[] = "Learning Factory";
static char pass[] = "Factory2";

static int status           = WL_IDLE_STATUS;
static float totalDistance  = 150.0f;
static bool systemStatus    = false;

// ======== Hardware Objects ========
static WiFiClient wifiClient;
static PubSubClient  client(wifiClient);
static Adafruit_VL53L0X lox = Adafruit_VL53L0X();
void checkClient();
bool Connect();
void Cloud_init();
void Cloud_sendData();