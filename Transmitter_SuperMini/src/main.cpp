#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>

// 1. CRITICAL STEP: Paste your N16R8 board's unique MAC address here!
// (Use the MAC address locator sketch we ran earlier to populate this array)
uint8_t receiverMacAddress[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};

// Hardware I2C pin layouts hardwired to your Super Mini tracker setup
#define I2C_SDA 5
#define I2C_SCL 6
#define MPU_ADDR 0x68

unsigned long lastTime;
float roll = 0, pitch = 0, yaw = 0;

// Binary packet structure MUST match your receiver side data structure perfectly
struct DataPacket {
  float r;
  float p;
  float y;
} myData;

esp_now_peer_info_t peerInfo;

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);

  // Wake up MPU6050 power management registers
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); Wire.write(0);    
  Wire.endTransmission();

  // Turn on Wi-Fi antenna circuitry in Station profile mode without network associations
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Bind and register the targeted Receiver board profiles
  memcpy(peerInfo.peer_addr, receiverMacAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  lastTime = millis();
}

void loop() {
  // Read raw register values from the MPU6050 sensor
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); 
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);

  int16_t ax = (Wire.read() << 8) | Wire.read();
  int16_t ay = (Wire.read() << 8) | Wire.read();
  int16_t az = (Wire.read() << 8) | Wire.read();
  Wire.read(); Wire.read(); // Skip temperature bytes
  int16_t gx = (Wire.read() << 8) | Wire.read();
  int16_t gy = (Wire.read() << 8) | Wire.read();
  int16_t gz = (Wire.read() << 8) | Wire.read();

  unsigned long currentTime = millis();
  float dt = (currentTime - lastTime) / 1000.0;
  lastTime = currentTime;

  // Convert raw values to degrees/second and G forces
  float accRoll  = atan2(ay, az) * 180.0 / PI;
  float accPitch = atan2(-ax, sqrt((float)ay * ay + (float)az * az)) * 180.0 / PI;

  // High-performance Complementary Filter processing loop
  roll  = 0.96 * (roll + (gx / 131.0) * dt) + 0.04 * accRoll;
  pitch = 0.96 * (pitch + (gy / 131.0) * dt) + 0.04 * accPitch;
  yaw   = yaw + (gz / 131.0) * dt; 

  // Store variables safely into the target binary structure
  myData.r = roll;
  myData.p = pitch;
  myData.y = yaw;

  // Fire packet cleanly into the room atmosphere spectrum
  esp_now_send(receiverMacAddress, (uint8_t *) &myData, sizeof(myData));

  delay(12); // Rest window matching local processing clock configurations
}