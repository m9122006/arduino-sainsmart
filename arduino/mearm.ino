#include <Servo.h>
#include "../calibration.hh"
#include "../controller.hh"
#include "../curve.hh"

class ServoCurve: public Curve
{
public:
  ServoCurve(void): Curve(0, 0.00015) {}
};

class Controller: public ControllerBase
{
public:
  Controller(void) {}
  void setup(void) {
    for (int drive=0; drive<DRIVES; drive++)
      m_servo[drive].attach(SERVOPIN[drive]);
  }
  float angleToPWMDrive(float angle, int drive) {
    return angleToPWM(angle, OFFSET[drive], RESOLUTION[drive], MIN[drive], MAX[drive]);
  }
  float limitArmDrive(int drive, float target) {
    return limitArm(drive, target, m_curve[SHOULDER].target(), m_curve[ELBOW].target());
  }
  void update(int dt) {
    for (int drive=0; drive<DRIVES; drive++) {
      float pwm = angleToPWMDrive(m_curve[drive].update(dt), drive);
      m_servo[drive].writeMicroseconds(round(pwm));
    };
  };
  void reportTime(void) {
    Serial.print(millis() * 0.001);
    Serial.write("\r\n");
  }
  void reportPosition(int drive) {
    Serial.print(m_curve[drive].pos());
    Serial.write("\r\n");
  }
  void reportPWM(int drive) {
    float pwm = angleToPWMDrive(m_curve[drive].pos(), drive);
    Serial.print(round(pwm));
    Serial.write("\r\n");
  }
  void targetAngle(int drive, float target) {
    float pwm = angleToPWMDrive(limitArmDrive(drive, target), drive);
    float angle = pwmToAngle(pwm, OFFSET[drive], RESOLUTION[drive]);
    m_curve[drive].retarget(angle);
  }
  void stopDrives(void)
  {
    for (int drive=0; drive<DRIVES; drive++)
      m_curve[drive].stop();
  }
protected:
  ServoCurve m_curve[4];
  Servo m_servo[4];
};

unsigned long t0;

Controller controller;

void setup() {
  controller.setup();
  Serial.begin(9600);
  t0 = millis();
}

void loop() {
  int dt = millis() - t0;
  if (Serial.available())
    controller.parseChar(Serial.read());
  controller.update(dt);
  t0 += dt;
}
