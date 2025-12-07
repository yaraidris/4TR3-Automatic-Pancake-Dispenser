#include <Arduino.h>
#include <AccelStepper.h>
#include <MultiStepper.h>

#include "pins.h"
#include "Plotter.h"
#include "HMI.h"
#include "cloud.h"

// ===== Stepper Motors =====
AccelStepper X(AccelStepper::DRIVER, X_STEP,   X_DIR);
AccelStepper Y(AccelStepper::DRIVER, Y_STEP,   Y_DIR);
AccelStepper Z(AccelStepper::DRIVER, Z_STEP,   Z_DIR);
AccelStepper VAL(AccelStepper::DRIVER, VAL_STEP, VAL_DIR);
MultiStepper steppers;

unsigned long previousMillis = 0;
unsigned long interval = 5000; // mil

// ===== Plotter Object =====
Plotter plotter(X, Y, Z, VAL, steppers);

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);

  // ===== Initialize Subsystems =====
  HMI_init();
  Cloud_init();

   // Initialize the Plotter
  plotter.begin(); 
  plotter.enableDrivers(true);
  plotter.goHome();
}

void loop()
{
  HMI_poll(); // Poll the HMI
  checkClient(); // Check the Client
  //Publishes to the cloud at specified interval (every 5 seconds in this case)
  if (millis() -  previousMillis > interval){
    previousMillis += interval;
    Cloud_sendData();
  }

  // ===== Handle Design Start Requests =====
  if (HMI_batchRequested()) // Batching
  {
    HMI_clearDesignRequests();

    uint8_t n = HMI_getBatchNumber();
    if (n == 0) n = 1; // at least 1 pancake

    Serial.println("Dispensing batch...");
    Point panCenter = { PAN_CENTER_X, PAN_CENTER_Y };

    plotter.dispenseBatch(
      n,                   // number of hex points
      panCenter,           // pan center
      PAN_RADIUS_STEPS,    // radius
      SITE_SPACING_STEPS,  // spacing between points
      PAN_EDGE_MARGIN_STEPS,
      DISPENSE_MS,                // valve open time per site (ms)
      HEX_START_ANGLE_RAD  // starting angle
    );
    plotter.closeValve();
  }

  if (HMI_squareRequested()) // Square
  {
    HMI_clearDesignRequests();

    Serial.println("Drawing square...");
    Point square[4];
    plotter.makeSquareShape(SQUARE_SIZE_STEPS, square);
    plotter.tracePath(square, 4, 0);
    plotter.closeValve();
  }

  if (HMI_triangleRequested()) // Triangle
  {
    HMI_clearDesignRequests();

    Serial.println("Drawing triangle...");
    Point triangle[3];
    plotter.makeTriangleShape(TRI_SIDE_STEPS, triangle);
    plotter.tracePath(triangle, 3, 1);
    plotter.closeValve();
  }

   // ===== Handle STOP from any page =====

  if (HMI_stopRequested()) // Stopped Pressed
  {
    Serial.println("STOP requested – homing and stopping motors.");
    plotter.goHome();

    HMI_clearStop();
    HMI_clearDesignRequests();
  } 
;}