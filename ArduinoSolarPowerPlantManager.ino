#include <TracerSolarChargeController.h>
#include <SakuraIO.h>

#define LED_PIN 10

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
  chargeCon.begin();
  //for(;;){
  //  if( (sakuraio.getConnectionStatus() & 0x80) == 0x80 ) break;
  //  Serial.print(".");
  //  delay(1000);
  //}
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

