#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h>
#include <TFT_eSPI.h> 
#include "drone_model.h" // Links our clean 521-face data variables seamlessly

TFT_eSPI tft = TFT_eSPI(); 
TFT_eSprite img = TFT_eSprite(&tft); // PSRAM sprite object allocation buffer

float roll = 0, pitch = 0, yaw = 0;

struct DataPacket { float r; float p; float y; } incomingData;

int polyX[3], polyY[3]; 
float rotX[1200], rotY[1200], rotZ[1200]; 

void OnDataRecv(const esp_now_recv_info_t * recv_info, const uint8_t *incomingDataPtr, int len) {
  memcpy(&incomingData, incomingDataPtr, sizeof(incomingData));
  roll  = incomingData.r; pitch = incomingData.p; yaw   = incomingData.y;
}

void setup() {
  pinMode(9, OUTPUT); digitalWrite(9, HIGH); // Waveshare screen backlight wake trace
  tft.init(); tft.setRotation(1); tft.fillScreen(TFT_BLACK);

  // Initialize your high-speed memory double-buffer sprite
  img.setColorDepth(8); 
  img.createSprite(320, 240); 

  WiFi.mode(WIFI_STA);
  if (esp_now_init() == ESP_OK) { esp_now_register_recv_cb(OnDataRecv); }
}

void loop() {
  float radX = radians(pitch); float radY = radians(yaw); float radZ = radians(roll);

  for (int i = 0; i < TOTAL_VERTICES; i++) {
    float x = vertices[i].x * 2.0; float y = vertices[i].y * 2.0; float z = vertices[i].z * 2.0;
    float y1 = y * cos(radX) - z * sin(radX); float z1 = y * sin(radX) + z * cos(radX);
    float x2 = x * cos(radY) + z1 * sin(radY); float z2 = -x * sin(radY) + z1 * cos(radY);
    rotX[i] = x2 * cos(radZ) - y1 * sin(radZ); rotY[i] = x2 * sin(radZ) + y1 * cos(radZ);
    rotZ[i] = z2 + 250; 
  }

  img.fillSprite(TFT_BLACK); // Clear your background canvas layer in RAM

  for (int i = 0; i < TOTAL_FACES; i++) {
    int idx[4] = {faces[i].v1, faces[i].v2, faces[i].v3, faces[i].v4};
    for(int j = 0; j < 3; j++) { 
      polyX[j] = 160 + (int)(rotX[idx[j]] * 200 / rotZ[idx[j]]);
      polyY[j] = 120 + (int)(rotY[idx[j]] * 200 / rotZ[idx[j]]);
    }
    float vectorCalculation = (polyX[1] - polyX[0]) * (polyY[2] - polyY[0]) - (polyY[1] - polyY[0]) * (polyX[2] - polyX[0]);
    if (vectorCalculation > 0) { 
      img.fillTriangle(polyX[0], polyY[0], polyX[1], polyY[1], polyX[2], polyY[2], faces[i].baseColor);
    }
  }
  img.pushSprite(0, 0); // Blink the memory canvas data directly to the Waveshare panel
  delay(5); 
}