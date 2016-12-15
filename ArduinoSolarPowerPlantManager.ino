#include <TracerSolarChargeController.h>
#include <SakuraIO.h>

#define LED_PIN 10
#define PANEL_AND_HEATER_PIN 4
#define PANEL_AND_BATTERY_PIN 5

static float HEATER_ON_VOLT = 24.8;
static float HEATER_OFF_VOLT = 24.3;
static float BATTERY_OFF_VOLT = 25.5;

TracerSolarChargeController chargeCon(&Serial1);
SakuraIO_I2C sakuraio;

int   sumCount            = 0;
float sumPanelVolt        = 0;
float sumBatteryChargeAmp = 0;
float sumBatteryVolt      = 0;
float sumChargeAmp        = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("setup");
  pinMode(LED_PIN, OUTPUT);
  pinMode(PANEL_AND_HEATER_PIN, OUTPUT);
  pinMode(PANEL_AND_BATTERY_PIN, OUTPUT);
  panelAndHeater(false);
  panelAndBattery(true);
  chargeCon.begin();
  Serial.println("finished setup");
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  Serial.println("start a loop");

  if (chargeCon.update()) {
    chargeCon.printInfo(&Serial);
    sumPanelVolt   += chargeCon.panelVolt;
    sumBatteryVolt += chargeCon.batteryVolt;
    sumChargeAmp   += chargeCon.chargeAmp;
    sumCount ++;
  }
  //sumPanelVolt += 30;
  //sumBatteryVolt += 22;
  //sumChargeAmp += 1.25;
  //sumCount ++;

  if (sumCount >= 6) {
    Serial.println("send to sakura");
    float panelVolt   = sumPanelVolt / sumCount;
    float batteryVolt = sumBatteryVolt / sumCount;
    float chargeAmp   = sumChargeAmp / sumCount;
    float chargeWatt  = batteryVolt * chargeAmp;

    updateRelays(batteryVolt);

    sakuraioSendSolarPowerInfo(panelVolt,
                               batteryVolt,
                               chargeAmp,
                               chargeWatt);

    sumPanelVolt   = 0;
    sumBatteryVolt = 0;
    sumChargeAmp   = 0;
    sumCount       = 0;
  }

  delay(10000);
}

void updateRelays(float batteryVolt) {
  if (batteryVolt >= HEATER_ON_VOLT) {
    Serial.println("heater on");
    panelAndHeater(true);
  } else if (batteryVolt <= HEATER_OFF_VOLT) {
    Serial.println("heater off");
    panelAndHeater(false);
  }
  if (batteryVolt > BATTERY_OFF_VOLT) {
    panelAndBattery(false);
  } else {
    panelAndBattery(true);
  }
}

void sakuraioSendSolarPowerInfo(float panelVolt,
                                float batteryVolt,
                                float chargeAmp,
                                float chargeWatt) {
  sakuraio.enqueueTx((uint8_t)0, (float) panelVolt);
  sakuraio.enqueueTx((uint8_t)1, (float) chargeAmp);
  sakuraio.enqueueTx((uint8_t)2, (float) batteryVolt);
  sakuraio.enqueueTx((uint8_t)3, (float) chargeWatt);
  sakuraio.send();
}

void panelAndHeater(bool connect) {
  // Normaly open
  if (connect) {
    digitalWrite(PANEL_AND_HEATER_PIN, HIGH);
  } else {
    digitalWrite(PANEL_AND_HEATER_PIN, LOW);
  }
}

void panelAndBattery(bool connect) {
  // Normaly close
  if (connect) {
    digitalWrite(PANEL_AND_BATTERY_PIN, LOW);
  } else {
    digitalWrite(PANEL_AND_BATTERY_PIN, HIGH);
  }
}

