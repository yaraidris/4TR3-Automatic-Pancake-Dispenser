#include "Plotter.h"
#include "HMI.h"
#include "pins.h"
#include "cloud.h"

// Calculates the distance between the points for batching using the radius of the griddle
static long calculateHexRadius(long panRadius, long edgeMargin, long siteSpacing)
{
  const float usableR = (float)(panRadius - edgeMargin);
  float rNeed = (float)siteSpacing;
  if (rNeed > usableR) rNeed = usableR;
  if (rNeed < 0)       rNeed = 0;
  return (long)rNeed;
}
//Generates the amount of pancakes to dispense depending on button presses from HMI
static uint8_t generateHexPoints(uint8_t n, const Point &center, long radius, float startAngleRad, Point out[6])
{
  if (n < 1) n = 1;
  if (n > 6) n = 6;

  for (uint8_t i = 0; i < n; ++i)
  {
    float angle = startAngleRad + i * (2.0f * PI / 6.0f);
    out[i].x = center.x + (long)(radius * cos(angle));
    out[i].y = center.y + (long)(radius * sin(angle));
  }
  return n;
}

// ---------- Plotter implementation ----------
Plotter::Plotter(AccelStepper &x, AccelStepper &y, AccelStepper &z, AccelStepper &val, MultiStepper &group) : x_(x), y_(y), z_(z), val_(val), group_(group){}

void Plotter::begin()
{
  // Step pulse width
  x_.setMinPulseWidth(STEP_PULSE_US);
  y_.setMinPulseWidth(STEP_PULSE_US);
  z_.setMinPulseWidth(STEP_PULSE_US);
  val_.setMinPulseWidth(STEP_PULSE_US);

  // Max speeds
  x_.setMaxSpeed(XY_MAX_SPEED);
  y_.setMaxSpeed(XY_MAX_SPEED);
  z_.setMaxSpeed(Z_MAX_SPEED);
  val_.setMaxSpeed(VAL_MAX_SPEED);

  // DIR inversion
  x_.setPinsInverted(false, X_DIR_INVERTED, false);
  y_.setPinsInverted(false, Y_DIR_INVERTED, false);
  z_.setPinsInverted(false, Z_DIR_INVERTED, false);
  val_.setPinsInverted(false, VAL_DIR_INVERTED, false);

  // Shared enable (optional)
  if (ENABLE_PIN != 255)
  {
    pinMode(ENABLE_PIN, OUTPUT);
    digitalWrite(ENABLE_PIN, LOW);
  }

  // Per-axis enable (optional)
  if (X_EN != 255) { pinMode(X_EN, OUTPUT); digitalWrite(X_EN, LOW); }
  if (Y_EN != 255) { pinMode(Y_EN, OUTPUT); digitalWrite(Y_EN, LOW); }
  if (Z_EN != 255) { pinMode(Z_EN, OUTPUT); digitalWrite(Z_EN, LOW); }
  if (VAL_EN != 255) { pinMode(VAL_EN, OUTPUT); digitalWrite(VAL_EN, LOW); }

  // Register with MultiStepper (order matters)
  group_.addStepper(x_);
  group_.addStepper(y_);
  group_.addStepper(z_);
  group_.addStepper(val_);
}

void Plotter::enableDrivers(bool on)
{
  if (ENABLE_PIN == 255) return;
  digitalWrite(ENABLE_PIN, on ? LOW : HIGH);
}

// ---------- Movement helpers ----------

void Plotter::moveToXY(const Point &p)
{
  long tgt[4] = { p.x, p.y, z_.targetPosition(), val_.targetPosition() };
  group_.moveTo(tgt);
  group_.runSpeedToPosition();
}

void Plotter::moveTower(long zSteps)
{
  long tgt[4] = { x_.targetPosition(), y_.targetPosition(), zSteps, val_.targetPosition() };
  group_.moveTo(tgt);
  group_.runSpeedToPosition();
}

void Plotter::moveValve(long vSteps)
{
  long tgt[4] = { x_.targetPosition(), y_.targetPosition(), z_.targetPosition(), vSteps };
  group_.moveTo(tgt);
  group_.runSpeedToPosition();
}

void Plotter::moveToAll(long xs, long ys, long zs, long vs)
{
  long tgt[4] = { xs, ys, zs, vs };
  group_.moveTo(tgt);
  group_.runSpeedToPosition();
}

void Plotter::liftTower()
{
  moveTower(Z_UP_POS);
}

void Plotter::lowerTower()
{
  moveTower(Z_DOWN_POS);
}

static const long VALVE_STEP_AMOUNT = 800; 
static long currentValvePos = 0;
bool valveOpen = false;

void Plotter::openValve()
{
  if (!valveOpen)
  {
    valveOpen = !valveOpen;
    currentValvePos += VALVE_STEP_AMOUNT; // always rotate forward due to how we designed our valve attachment
    moveValve(currentValvePos);
  }
  
}

void Plotter::closeValve()
{
  if (valveOpen)
  {
    valveOpen = !valveOpen;
    currentValvePos += VALVE_STEP_AMOUNT; // always rotate forward due to how we designed our valve attachment
    moveValve(currentValvePos);
  }
}

void Plotter::goHome()
{
  if(valveOpen)
  {
    moveToAll(0, 0, Z_UP_POS, (currentValvePos + VALVE_STEP_AMOUNT));
  }
  else
  {
    moveToAll(0, 0, Z_UP_POS, (currentValvePos));
  }
}

// ---------- Design helpers ----------

void Plotter::makeSquareShape(long side, Point out[4])
{
  Serial.println("Square Making");
  out[0] = { side, 0 };
  out[1] = { side, side };
  out[2] = { 0,    side };
  out[3] = { 0,    0    };
}

void Plotter::makeTriangleShape(long side, Point out[3])
{
  Serial.println("Triangle Making");
  const float L = (float)side;
  out[0] = { 0, 0 };
  out[1] = { (long)L, 0 };
  out[2] = { (long)(L * 0.5f), (long)(L * 0.86602540378f) };
}

uint8_t Plotter::makeBatchLayout(uint8_t n, const Point &panCenter, long panRadius, long siteSpacing, long edgeMargin, float startAngleRad, Point out[6])
{
  Serial.println("Batch Making");
  const long R = calculateHexRadius(panRadius, edgeMargin, siteSpacing);
  return generateHexPoints(n, panCenter, R, startAngleRad, out);
}

// ---------- Trace & batch ----------

void Plotter::tracePath(const Point *pts, size_t count, int shape)
{
  if (!pts || count == 0) return;

  // Ensure valve is closed & start at origin
  closeValve();
  liftTower();
  moveToXY({0, 0});
  if(HMI_stopRequested()) { return; }

  // Move to first point and start dispensing
  moveToXY(pts[0]);
  lowerTower();
  openValve();

  // Follow remaining points
  for (size_t i = 1; i < count; ++i)
  {
    checkClient();
    HMI_poll();
    if(HMI_stopRequested()) { return; }
    moveToXY(pts[i]);
  }
  if(HMI_stopRequested()) { return; }
  moveToXY(pts[0]);

  // Return to origin and fill
  if (shape == 0)
  {
    moveToXY({SQUARE_SIZE_STEPS/2, SQUARE_SIZE_STEPS/2});
  }
  else if (shape == 1)
  {
    moveToXY({TRI_SIDE_STEPS/2, TRI_SIDE_STEPS/3});
  }
  else 
  {
    goHome();
  }
  
  lowerTower();
  openValve();
  delay(FILL_MS);
  closeValve();
  liftTower();
  goHome();
}

void Plotter::dispenseOnce(const Point &p, uint16_t dwellMs)
{
  liftTower();
  moveToXY(p);
  lowerTower();
  openValve();
  delay(dwellMs);
  closeValve();
  liftTower();
}

void Plotter::dispenseBatch(uint8_t n, const Point &panCenter, long panRadius, long siteSpacing, long edgeMargin, uint16_t openMs, float startAngleRad)
{
  Point pts[6];
  const uint8_t m = makeBatchLayout(n, panCenter, panRadius, siteSpacing, edgeMargin, startAngleRad, pts);

  goHome();

  for (uint8_t i = 0; i < m; ++i)
  {
    HMI_poll();
    checkClient();
    if(HMI_stopRequested()) { return; }

    liftTower();
    moveToXY(pts[i]);
    lowerTower();

    openValve();
    delay(openMs);
    closeValve();
  }

  moveToXY({0, 0});
  closeValve();
}