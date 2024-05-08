#include <BlynkEdgent.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include "HX711.h"
#include <Servo.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <LineNotify.h> // ไลบรารีสำหรับส่งข้อความไปยัง LINE
#include <TimeLib.h>

#define WIFI_SSID "SIM"
#define WIFI_PASSWORD "simmyjung110"
#define LINE_TOKEN "YourLineAccessToken" //รอใส่

#define LoadCell  1 //รอใส่ค่าจริง
#define ServoPin    2 //รอใส่ค่าจริง

#define DOUT_PIN 23
#define CLK_PIN 22

HX711 scale(DOUT_PIN, CLK_PIN);
Servo myServo;

WiFiClientSecure client;
LineNotify lineNotify(client, LINE_TOKEN);

char auth[] = "TMPL6xcI0PxKX";
char ssid[] = "SIM";
char pass[] = "simmyjung110";

BlynkTimer timer;

void myTimeEvent(){
  if (hour() == 6 || hour() == 12 || hour() == 18 || hour() == 0) { // ตรวจสอบเวลา 6:00, 12:00, 18:00, และ 00:00
    int currentMinute = minute();
    int currentSecond = second();
    
    if (currentMinute == 0 && currentSecond < 10) { // ตรวจสอบวินาที 10 วินาทีแรกของนาทีที่ 0
      rotateServo(30);
      delay(10000); // หมุน servo ในเวลาประมาณ 10 วินาที
    }
  }
}

void rotateServo(int angle) {
  myServo.write(angle); // หมุน servo ไปที่มุมที่กำหนด
  delay(15); // รอให้ servo หมุนสมบูรณ์
}

void myLine(){
  if (lineNotify.available()) {
    String message = lineNotify.getMessage();

    // ตรวจสอบว่าเป็นข้อความเกี่ยวกับน้ำหนักหรือไม่
    if (message.startsWith("Weight:")) {
      // ดึงค่าน้ำหนักออกมาจากข้อความ
      float weightIndex = message.indexOf(":") + 1;
      float weight = message.substring(weightIndex).toFloat();

      // ใช้ค่าน้ำหนักในการควบคุม LoadCell
      controlLoadCell(weight);
    }
  }
}

void controlLoadCell(float weight) {
  scale.set_scale(); //ตั้งค่าขาต่อ LoadCell
  scale.tare(); //รีเซ็ตค่าต้นฉบับ
  for (int pos = 0; pos <= 90; pos+= 90) {
    myServo.write(pos);
    delay(1000); // หน่วงเวลา 1 วินาที
    
    // อ่านค่าน้ำหนักจากเซ็นเซอร์ HX711
    int weightValue = scale.get_units();
    
    // ตรวจสอบว่าค่าน้ำหนักมากกว่าหรือเท่ากับ 200 กรัมหรือไม่
    if (weightValue >= weight) {
      // หยุดหมุน Servo Motor เมื่อค่าน้ำหนักมากกว่าหรือเท่ากับ น้ำหนักที่กำหนด
      break;
    }
  }
}

//เอาไว้เชื่อมต่อ WiFi
void connectWiFi() {
  Serial.println("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }

  Serial.println("WiFi connected");
}

void setup()
{
  Serial.begin(115200);
  delay(100);
  Blynk.begin(auth, ssid, pass);
  pinMode(LoadCell, INPUT);
  myServo.attach(ServoPin); // ตั้งค่าขาที่เชื่อมกับ servo

  connectWiFi();
  BlynkEdgent.begin();
  timer.setInterval(1000L, myTimeEvent); // เรียกใช้ฟังก์ชัน myTimeEvent() ทุก 1 วินาที
}

void loop() {
  BlynkEdgent.run();
  Blynk.run();
  timer.run();
  myLine();
}
