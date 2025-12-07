#pragma once
#include <Arduino.h>
#include <AccelStepper.h>
#include <MultiStepper.h>

// ---------- Geometry types ----------
struct Point
{
  long x;
  long y;
};

// ---------- Pan & design parameters ----------
#ifndef PI
#define PI 3.14159265358979323846
#endif

// Pan / layout
constexpr long PAN_RADIUS_STEPS        = 8000;   // pan radius in steps
constexpr long PAN_CENTER_X            = 0;      // pan center X in steps
constexpr long PAN_CENTER_Y            = 0;      // pan center Y in steps
constexpr long PAN_EDGE_MARGIN_STEPS   = 300;    // keep away from rim
constexpr long SITE_SPACING_STEPS      = 6000;   // hex ring spacing
constexpr uint16_t DISPENSE_MS         = 4000;    // (not used directly now)
constexpr uint16_t FILL_MS             = 2000;   // time to fill at origin
constexpr float HEX_START_ANGLE_RAD    = 0.0f;

// Shapes
constexpr long SQUARE_SIZE_STEPS       = 6000;   // Square side
constexpr long TRI_SIDE_STEPS          = 5000;   // Triangle side

// Motion parameters
constexpr float XY_MAX_SPEED           = 1200.0f; // steps/s
constexpr float Z_MAX_SPEED            = 5000.0f; // steps/s
constexpr float VAL_MAX_SPEED          = 1200.0f; // steps/s

constexpr uint16_t STEP_PULSE_US       = 10;
constexpr uint16_t CORNER_PAUSE_MS     = 300;

// Positions
constexpr long Z_UP_POS                = 1000;   // Z axis high
constexpr long Z_DOWN_POS              = -1000;  // Z axis low



// Valve positions (labels only, we use incremental stepping now)
constexpr long VAL_OPEN_POS            = 800;
constexpr long VAL_CLOSED_POS          = 0;


class Plotter
{
public:
  Plotter(AccelStepper &x,
          AccelStepper &y,
          AccelStepper &z,
          AccelStepper &valve,
          MultiStepper &group);

  void begin();
  void enableDrivers(bool on);
  void goHome();

  // Basic motion (blocking, coordinated)
  void moveToXY(const Point &p);
  void moveTower(long zSteps);
  void moveValve(long vSteps);
  void moveToAll(long xs, long ys, long zs, long vs);

  // Convenience
  void liftTower();
  void lowerTower();
  void openValve();
  void closeValve();

  // Path operations
  void tracePath(const Point *pts, size_t count, int shape);

  // Single-site dispense
  void dispenseOnce(const Point &p, uint16_t dwellMs);

  // Design helpers
  void makeSquareShape(long side, Point out[4]);
  void makeTriangleShape(long side, Point out[3]);

  // Layout for batch (hex ring)
  uint8_t makeBatchLayout(uint8_t n,
                          const Point &panCenter,
                          long panRadius,
                          long siteSpacing,
                          long edgeMargin,
                          float startAngleRad,
                          Point out[6]);

  // Batch dispense
  void dispenseBatch(uint8_t n,
                     const Point &panCenter,
                     long panRadius,
                     long siteSpacing,
                     long edgeMargin,
                     uint16_t openMs,
                     float startAngleRad);

private:
  AccelStepper &x_;
  AccelStepper &y_;
  AccelStepper &z_;
  AccelStepper &val_;
  MultiStepper &group_;
};