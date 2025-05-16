#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

//sudo chmod a+rw /dev/ttyUSB0

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID1 "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID2 "beb5483e-36e1-4688-b7f5-ea07361b26a2"
#define BUTTON_PIN1 21  // GIOP21 pin connected to button
#define BUTTON_PIN2 19 

// Variables will change:
int moveLastState = HIGH;  // the previous state from the input pin
int moveCurrentState;     // the current reading from the input pin

int selectLastState = HIGH;  // the previous state from the input pin
int selectCurrentState;     // the current reading from the input pin


BLECharacteristic *pCharacteristicMoveButton;
BLECharacteristic *pCharacteristicSelectButton;


void setup() {
  Serial.begin(9600);
  //Serial.println("Starting BLE work!");
  pinMode(BUTTON_PIN1, INPUT_PULLUP);
  pinMode(BUTTON_PIN2, INPUT_PULLUP);

  BLEDevice::init("Hand Controller");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristicMoveButton = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID1,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pCharacteristicSelectButton = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID2,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  
  pCharacteristicMoveButton->setValue("Waiting");
  pCharacteristicSelectButton->setValue("Waiting");
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {

  moveCurrentState = digitalRead(BUTTON_PIN1);

  selectCurrentState = digitalRead(BUTTON_PIN2);

  if(moveLastState == LOW && moveCurrentState == HIGH)
  {
    pCharacteristicMoveButton->setValue("Down");
    pCharacteristicMoveButton->notify();
    Serial.println("Down");
    
  }

  if(selectLastState == LOW && selectCurrentState == HIGH)
  {
    pCharacteristicSelectButton->setValue("Select");
    pCharacteristicSelectButton->notify();
    Serial.println("Selected");
  }
  moveLastState = moveCurrentState;
  selectLastState = selectCurrentState;
  delay(10);
}
