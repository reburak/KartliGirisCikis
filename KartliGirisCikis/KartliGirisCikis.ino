#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <SPI.h>
#include <MFRC522.h>
#define BUZZER_PIN D0
#define RST_PIN D4
#define SS_PIN D8
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

// Init array that will store new NUID
byte nuidPICC[4];
String HexId = "";
String DecId = "";

String EnterType = "20";  // Giris: 10 - Çıkış: 20
const char* ssid = "IsTech";
const char* password = "Tech2o23*";
const char* host = "ptsapi.isteknoloji.com.tr";
const int httpsPort = 443;

WiFiClient wClient;

void postData(String value) {
  WiFiClientSecure http;
  http.setInsecure();
  Serial.println("PostData: " + value);

  if (!http.connect(host, httpsPort)) {
    Serial.println("Connection failed");
    soundAlert(100, 5);
    return;
  }

  String url = "/api/Guest/SaveEnterLog/" + EnterType + "/" + value;

  http.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

  Serial.println("Veri gönderiliyor...");
  while (http.connected()) {
    String line = http.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("Veri gönderildi.");
      break;
    }
  }
  soundAlert(250, 1);

  String payload = http.readStringUntil('\n');
  http.stop();
}

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  Serial.begin(115200);
  SPI.begin();  //mfrc522 OKUCU Başlatıcı
  mfrc522.PCD_Init();

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);

  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Baglaniyor");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(250);
  }
  soundAlert(250, 2);

  Serial.println("");
  Serial.print("Baglanti Saglandi : ");
  Serial.println(ssid);
  Serial.print("IP adres: ");
  Serial.println(WiFi.localIP());

  Serial.println("Karti Simdi Okutabilirsiniz !");
  Serial.println("");
}

void soundAlert(int saniye, int tekrar) {
  for (int i = 0; i < tekrar; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(saniye);
    digitalWrite(BUZZER_PIN, LOW);
    delay(saniye);
  }
}

void loop() {

  String Id = "";
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!mfrc522.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if (!mfrc522.PICC_ReadCardSerial())
    return;

  printHex(mfrc522.uid.uidByte, mfrc522.uid.size);
  printDec(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();
  postData(HexId);
  // Halt PICC
  mfrc522.PICC_HaltA();

  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();

  delay(3000);
}
void printHex(byte* buffer, byte bufferSize) {
  HexId = "";
  for (byte i = 0; i < bufferSize; i++) {
    HexId += (buffer[i] < 0x10 ? "0" : "");
    HexId += String(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte* buffer, byte bufferSize) {
  DecId = "";
  for (byte i = 0; i < bufferSize; i++) {
    DecId += (buffer[i] < 0x10 ? "0" : "");
    DecId += String(buffer[i], DEC);

  }
}