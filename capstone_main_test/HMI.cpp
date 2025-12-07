#include <Arduino.h>

// Bind Nextion library to Serial2
#define nexSerial Serial2
#include <Nextion.h>

#include "HMI.h"

// ===== Nextion Buttons =====

// Batch operations page (page 2)
static NexButton b5(2, 2, "b5");   // start batch
static NexButton b7(2, 3, "b7");   // stop batch

// Square operations page (page 3)
static NexButton b8(3, 2, "b8");   // start square
static NexButton b10(3, 4, "b10"); // stop square

// Triangle operations page (page 4)
static NexButton b11(4, 2, "b11"); // start triangle
static NexButton b13(4, 3, "b13"); // stop triangle

// Batch quantity page (page 5)
static NexButton b14(5, 2, "b14"); // add
static NexButton b15(5, 3, "b15"); // minus

// Done / main menu page (page 10)
static NexButton b17(10, 2, "b17");

static NexButton b18(2, 8, "b18");

// List of components to listen to
static NexTouch *nex_listen_list[] = {
  &b5, &b7,
  &b8, &b10,
  &b11, &b13,
  &b14, &b15,
  &b17, &b18,
  nullptr
};

// ===== HMI state =====
static bool   stopFlag       = false;
static bool   batchFlag      = false;
static bool   squareFlag     = false;
static bool   triangleFlag   = false;
static uint8_t batchNumber   = 0;   // 0..6

// ===== Internal helpers =====
//We need to send 3 empty lines after each button press to clear the serial
static void sendTerminator()
{
  Serial2.write(0xFF);
  Serial2.write(0xFF);
  Serial2.write(0xFF);
}

// ===== Callbacks =====

// Add button for batch
static void AddPushCallback(void *ptr)
{
  if (batchNumber < 6)
  {
    batchNumber++;
    Serial.println("HMI: b14 (Add) pressed, batch_number = " + String(batchNumber));
  }
  sendTerminator();
}

// Subtract button for batch
static void SubPushCallback(void *ptr)
{
  if (batchNumber > 0)
  {
    batchNumber--;
    Serial.println("HMI: b15 (Sub) pressed, batch_number = " + String(batchNumber));
  }
  sendTerminator();
}

// Start batch (page 2)
static void b5PushCallback(void *ptr)
{
  batchFlag = true;
  Serial.println("HMI: b5 (Batch start) pressed");
  sendTerminator();
}

// Back Button (page 2)
static void b18PushCallback(void *ptr)
{
  batchNumber = 0;
  Serial.println("HMI: b18 (Back) pressed");
  sendTerminator();
}

// Start square
static void b8PushCallback(void *ptr)
{
  squareFlag = true;
  Serial.println("HMI: b8 (Square start) pressed");
  sendTerminator();
}

// Start triangle
static void b11PushCallback(void *ptr)
{
  triangleFlag = true;
  Serial.println("HMI: b11 (Triangle start) pressed");
  sendTerminator();
}

// Any stop button
static void StopPushCallback(void *ptr)
{
  stopFlag = true;
  Serial.println("HMI: Stop button pressed");
  sendTerminator();
}

// Initializing all HMI buttons and Serial

void HMI_init()
{
  Serial2.begin(9600);   // Nextion UART (baud must match HMI project)
  nexInit();

  // Attach callbacks
  b5.attachPush(b5PushCallback,   nullptr);
  b7.attachPush(StopPushCallback, nullptr);
  b8.attachPush(b8PushCallback,   nullptr);
  b10.attachPush(StopPushCallback, nullptr);
  b11.attachPush(b11PushCallback, nullptr);
  b13.attachPush(StopPushCallback, nullptr);
  b14.attachPush(AddPushCallback, nullptr);
  b15.attachPush(SubPushCallback, nullptr);
  b17.attachPush(StopPushCallback, nullptr);
  b18.attachPush(b18PushCallback, nullptr);
}

void HMI_poll()
{
  nexLoop(nex_listen_list);
}

bool HMI_batchRequested()   { return batchFlag;    }
bool HMI_squareRequested()  { return squareFlag;   }
bool HMI_triangleRequested(){ return triangleFlag; }
uint8_t HMI_getBatchNumber(){ return batchNumber;  }

bool HMI_stopRequested()    { return stopFlag;     }

void HMI_clearDesignRequests()
{
  batchFlag    = false;
  squareFlag   = false;
  triangleFlag = false;
}

void HMI_clearStop()
{
  stopFlag = false;
}