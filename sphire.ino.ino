#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <base64.h>

#define SERVICE_UUID        "afe84744-3652-4a4d-a5f3-ca3d85aa8207"
#define CONTROL_CHAR_UUID   "8c7d28d4-f658-4c67-9a27-37efb6c9c0f6"
#define SENSOR_CHAR_UUID    "beb82ea1-5c9a-4dfc-9d32-524ae3edc8da"

BLECharacteristic* pControlCharacteristic;
BLECharacteristic* pSensorCharacteristic;

bool shouldMeasure = false;
const int mq3Pin = 2;
const int measurementDuration = 5000; // ms
const int sampleInterval = 100;       // ms


// BLE write handler
class CommandCallback : public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic* characteristic) override {

    Serial.println("Starting measurement...");

    
    int avg = performMeasurement();

    pSensorCharacteristic->setValue( String(avg));
    pSensorCharacteristic->notify();

    Serial.println("Measurement sent via BLE.");

    delay(1000);
  }

  int performMeasurement() {
    int sum = 0;
    int samples = 0;
    unsigned long start = millis();

    while (millis() - start < measurementDuration) {
      int value = analogRead(mq3Pin);
      sum += value;
      samples++;
      delay(sampleInterval);
    }

    int average = samples > 0 ? (sum / samples) : 0;
    Serial.print("Average sensor value: ");
    Serial.println(average);
    return average;
  }

};

// BLE connection handler
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    Serial.println("Central connected. ");
  }

  void onDisconnect(BLEServer* pServer) override {
    Serial.println("Central disconnected.");
  }
};

void setup() {
  Serial.begin(115200);
  pinMode(mq3Pin, INPUT);

  BLEDevice::init("Sphire");
  BLEServer* server = BLEDevice::createServer();
  server->setCallbacks(new MyServerCallbacks());

  BLEService* service = server->createService(SERVICE_UUID);

  pControlCharacteristic = service->createCharacteristic(
    CONTROL_CHAR_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  //pControlCharacteristic->setCallbacks(new CommandCallback());

  pSensorCharacteristic = service->createCharacteristic(
    SENSOR_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pSensorCharacteristic->setCallbacks(new CommandCallback());

  service->start();

  BLEAdvertising* advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->start();

  Serial.println("BLE ready. Waiting for connection...");
}

void loop() {
}

// Function to perform sensor measurement and return the average value

