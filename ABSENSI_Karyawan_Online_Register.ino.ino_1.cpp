#line 1 "F:\\Mobile & Game Technology\\LEC Mobile Embedded\\ABSENSI_Karyawan_Online_Register.ino\\ABSENSI_Karyawan_Online_Register.ino.ino"
/* -------------------------------------------------
 Program : Presensi Kehadiran Karyawan dg Google Sheet
 Input   : RFID Tag/ Key
 Output  : LCD, Buzzer, Google Sheet
 Chip    : NodeMCU ESP8266
 * -------------------------------------------------*/ 
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <HTTPSRedirect.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

//---------------------------------------------------------------------------------------------
// GANTI DG Google Script Deployment ID ANDA (Hanya ID-nya saja, bukan URL full):
const char *GScriptId = "AKfycbxuf5kqF6iTMVmv-P13H4yfkWgM_5ye4OitoGXg-SKnLcfjd-o3TzKYvn3F4FxA7Vah";
//----------------------------------------------------------------------------------------------
// GANTI DENGAN WIFI ANDA
const char* ssid     = "A96";
const char* password = "03731168"; 
//--------------------------------------------------------------------------------------------
const char* host        = "script.google.com";  
const int   httpsPort   = 443;
HTTPSRedirect* client = nullptr;

//------------------------------------------------------------
#define RST_PIN  0  // D3
#define SS_PIN   2  // D4
#define BUZZ     15 // D8
//------------------------------------------------------------
MFRC522 mfrc522(SS_PIN, RST_PIN);

//================================
#line 37 "F:\\Mobile & Game Technology\\LEC Mobile Embedded\\ABSENSI_Karyawan_Online_Register.ino\\ABSENSI_Karyawan_Online_Register.ino.ino"
void setup();
#line 122 "F:\\Mobile & Game Technology\\LEC Mobile Embedded\\ABSENSI_Karyawan_Online_Register.ino\\ABSENSI_Karyawan_Online_Register.ino.ino"
void loop();
#line 37 "F:\\Mobile & Game Technology\\LEC Mobile Embedded\\ABSENSI_Karyawan_Online_Register.ino\\ABSENSI_Karyawan_Online_Register.ino.ino"
void setup() {
  Serial.begin(115200);  
  pinMode(BUZZ,OUTPUT);      
  SPI.begin();
  
  lcd.begin(16,2);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("   Smart RFID   ");
  lcd.setCursor(0,1);
  lcd.print("Presensi Online");  
  delay(2000);
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Tunggu Koneksi");
  lcd.setCursor(0,1);
  lcd.print("WiFi...");  
  
  WiFi.begin(ssid, password);            
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");  
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  lcd.clear();
  lcd.print("WiFi tersambung");
  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  
  // Setup HTTPS Redirect Client
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  
  lcd.clear();  
  lcd.print("Tunggu Koneksi");
  lcd.setCursor(0,1);
  lcd.print("Google ... ");
  
  Serial.print("Connecting to ");
  Serial.println(host);
  
  // Mencoba connect ke Google
  bool flag = false;
  for(int i=0; i<5; i++){ 
    int retval = client->connect(host, httpsPort);
    if (retval == 1){
      flag = true;
      Serial.println("Connected. OK");
      lcd.clear();      
      lcd.print("Google ");
      lcd.setCursor(0,1);
      lcd.print("Tersambung");
      delay(2000);
      break;
    } else {
      Serial.println("Connection failed. Retrying...");
    }
  }
  
  if (!flag){
    lcd.clear();   
    lcd.print("Koneksi Gagal");
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    delay(5000);
    return;
  }
  
  delete client;    
  client = nullptr; 
  lcd.clear();
  mfrc522.PCD_Init();
}

//=========================================
void loop() {
  static bool flag = false;
  if (!flag){
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }
  
  if (client != nullptr){
    if (!client->connected()) {
      client->connect(host, httpsPort);
    }
  } else {
    Serial.println("Error creating client object!");
  }  
  
  lcd.setCursor(0,0);
  lcd.print("Silahkan Tap    ");  
  lcd.setCursor(0,1);
  lcd.print("Kartu Anda...   ");  
  
  // Menunggu kartu di-tap
  if ( ! mfrc522.PICC_IsNewCardPresent()) {return;} 
  if ( ! mfrc522.PICC_ReadCardSerial()) {return;} 
  
  Serial.println();
  Serial.println("Membaca UID dari RFID...");  
  
  // Mengambil UID Kartu
  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uid += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  
  // Bunyikan Buzzer 1x saat kartu terdeteksi
  digitalWrite(BUZZ, HIGH); delay(200); digitalWrite(BUZZ, LOW);
  
  lcd.clear();  
  lcd.print("UID: " + uid);
  lcd.setCursor(0,1);
  lcd.print("Memproses...");  
  
  // URL Endpoint GET untuk absensi ke Code.gs
  String url = String("/macros/s/") + GScriptId + "/exec?action=absen&uid=" + uid;
  
  // Mengirim HTTP GET ke Google Script
  if(client->GET(url, host)){     
    String response = client->getResponseBody();
    response.trim(); // Membersihkan spasi atau newline
    
    lcd.clear();
    lcd.setCursor(0,0);
    
    // Logika jika UID belum ada di Google Sheets (Karyawan Baru)
    if (response == "UID TDK TERDAFTAR") {
      lcd.print("KARTU BARU !");
      lcd.setCursor(0,1);
      lcd.print(uid); // Menampilkan UID agar bisa diketik di UI HTML
      
      // Bunyi Buzzer 3x cepat sebagai peringatan
      digitalWrite(BUZZ, HIGH); delay(100); digitalWrite(BUZZ, LOW); delay(100);
      digitalWrite(BUZZ, HIGH); delay(100); digitalWrite(BUZZ, LOW); delay(100);
      digitalWrite(BUZZ, HIGH); delay(100); digitalWrite(BUZZ, LOW);
      
      delay(5000); // Tahan layar 5 detik agar UID sempat dibaca dan dicatat di web
    } 
    // Logika jika Absen Masuk/Pulang Berhasil
    else {
      lcd.print(response); // Print "TAP IN BERHASIL" atau "TAP OUT BERHASIL"
      lcd.setCursor(0,1);
      lcd.print("Terima Kasih");
      
      // Bunyi Buzzer panjang 1x tanda sukses
      digitalWrite(BUZZ, HIGH); delay(500); digitalWrite(BUZZ, LOW);
      
      delay(3000);
    }
  } else {   
    Serial.println("Error while connecting");
    lcd.clear();    
    lcd.print("Gagal Sinkron");
    lcd.setCursor(0,1);
    lcd.print("Coba lagi");
    delay(3000);
  }
  
  // Hentikan komunikasi dengan kartu ini sementara agar tidak spam read
  mfrc522.PICC_HaltA();
  lcd.clear();
}
