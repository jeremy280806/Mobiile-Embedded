#line 1 "F:\\Mobile & Game Technology\\LEC Mobile Embedded\\ABSENSI_Karyawan_Online\\ABSENSI_Karyawan_Online.ino"
/* -------------------------------------------------
 Program : Presensi Kehadiran Karyawan dg Google Sheet
 Input   : RFID Tag/ Key
 Output  : LCD, Google Sheet
 Chip    : NodeMCU V3
 Data    : Tanggal + Jam Datang + Jam Pulang + NIP + Nama Karyawan + Jabatan
 www.ardutech.com
 * -------------------------------------------------*/ 
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <HTTPSRedirect.h>

#include<Wire.h>
#include<LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
//---------------------------------------------------------------------------------------------
// GANTI DG Google Script Deployment ID ANDA:
const char *GScriptId = "AKfycbw86AdtF-YbczKdOIBSNeguPW8ZRfv5QlHA-g--Dm5ibC3qmF-rktR-PB6pqr07NHPl";
//----------------------------------------------------------------------------------------------
// GANTI DENGAN WIFI ANDA
const char* ssid     = "Tedy aryanto";
const char* password = "tedy280806";
//--------------------------------------------------------------------------------------------
String payload_base =  "{\"command\": \"insert_row\", \"sheet_name\": \"Sheet1\", \"values\": ";
String payload = "";
//-------------------------------------------------------------------------------------------
const char* host        = "script.google.com";
const int   httpsPort   = 443;
const char* fingerprint = "";
String url = String("/macros/s/") + GScriptId + "/exec";
HTTPSRedirect* client = nullptr;
//------------------------------------------------------------
String nip,nama_awal,nama_akhir,jabatan;
//------------------------------------------------------------
int blocks[] = {4,5,6,8};
#define total_blocks  (sizeof(blocks) / sizeof(blocks[0]))
//------------------------------------------------------------
#define RST_PIN  0  //D3
#define SS_PIN   2  //D4
#define BUZZ 15 //D8
//------------------------------------------------------------
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;  
MFRC522::StatusCode status;
//------------------------------------------------------------
int blockNum = 2;  
byte bufferLen = 18;
byte readBlockData[18];
//================================
#line 52 "F:\\Mobile & Game Technology\\LEC Mobile Embedded\\ABSENSI_Karyawan_Online\\ABSENSI_Karyawan_Online.ino"
void setup();
#line 137 "F:\\Mobile & Game Technology\\LEC Mobile Embedded\\ABSENSI_Karyawan_Online\\ABSENSI_Karyawan_Online.ino"
void loop();
#line 225 "F:\\Mobile & Game Technology\\LEC Mobile Embedded\\ABSENSI_Karyawan_Online\\ABSENSI_Karyawan_Online.ino"
void ReadDataFromBlock(int blockNum, byte readBlockData[]);
#line 52 "F:\\Mobile & Game Technology\\LEC Mobile Embedded\\ABSENSI_Karyawan_Online\\ABSENSI_Karyawan_Online.ino"
void setup() {
  Serial.begin(115200);  
  pinMode(BUZZ,OUTPUT);      
  SPI.begin();
  lcd.begin(16,2);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("  Smart RFID   ");
  lcd.setCursor(0,1);
  lcd.print("Presensi Online");  
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Tunggu Koneksi");
  lcd.setCursor(0,1);
  lcd.print("WiFi");  
  WiFi.begin(ssid, password);             
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.print(".");
  }
  lcd.clear();
  lcd.print("WiFi tersambung");
  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  //----------------------------------------------------------
  lcd.clear();  
  lcd.print("Tunggu Koneksi");
  lcd.setCursor(0,1); //col=0 row=0
  lcd.print("Google ... ");
  delay(5000);
  //----------------------------------------------------------
  Serial.print("Connecting to ");
  Serial.println(host);
  //----------------------------------------------------------
  // Try to connect for a maximum of 5 times
  bool flag = false;
  for(int i=0; i<5; i++){ 
    int retval = client->connect(host, httpsPort);
    //*************************************************
    if (retval == 1){
      flag = true;
      String msg = "Connected. OK";
      Serial.println(msg);
      lcd.clear();      
      lcd.print("Google ");
      lcd.setCursor(0,1);
      lcd.print("Tersambung");
      delay(2000);
      break;
    }
    //*************************************************
    else
      Serial.println("Connection failed. Retrying...");
    //*************************************************
    
  }
  //----------------------------------------------------------
  if (!flag){
    //____________________________________________
    lcd.clear();   
    lcd.print("Koneksi Gagal");
    //____________________________________________
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    delay(5000);
    return;
    //____________________________________________
  }
  delete client;    
  client = nullptr; 
  lcd.clear();
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
    if (!client->connected())
      {client->connect(host, httpsPort);}
  }
  else{Serial.println("Error creating client object!");}  
  lcd.setCursor(0,0); //col=0 row=0
  lcd.print("Tap RFID anda..");  
  mfrc522.PCD_Init();
  if ( ! mfrc522.PICC_IsNewCardPresent()) {return;} 
  if ( ! mfrc522.PICC_ReadCardSerial()) {return;} 
  Serial.println();
  Serial.println(F("Reading last data from RFID..."));  
  //----------------------------------------------------------------
  String values = "", data;    
  for (byte i = 0; i < total_blocks; i++) {
    ReadDataFromBlock(blocks[i], readBlockData);
    //*************************************************
    if(i == 0){
      data = String((char*)readBlockData);
      data.trim();     
      values = "\"" + data + ",";     
    }
    //*************************************************
    else if(i == total_blocks-1){
      data = String((char*)readBlockData);
      data.trim();
      values += data + "\"}";      
    }
    //*************************************************
    else{
      data = String((char*)readBlockData);
      data.trim();
      values += data + ",";
      //---
       if(i==1){
         nama_awal = data;      
         }
       else if(i==2){
         nama_akhir = data;      
         }
    }
  }
  payload = payload_base + values;
  digitalWrite(BUZZ,HIGH);
  delay(200);  
  digitalWrite(BUZZ,LOW);
  delay(200); 
  digitalWrite(BUZZ,HIGH);
  delay(200);  
  digitalWrite(BUZZ,LOW);
  delay(200); 
  //----------------------------------------------------------------
  lcd.clear();  
  lcd.print("Mengirim Data");
  lcd.setCursor(0,1); //col=0 row=0
  lcd.print("Mohon tunggu...");  
  //----------------------------------------------------------------
   if(client->POST(url, host, payload)){     
    lcd.clear();
    lcd.print(nama_awal);
    lcd.print(" ");
    lcd.print(nama_akhir);
    lcd.setCursor(0,1); //col=0 row=0
    lcd.print("Data tersimpan ");
  }
  //----------------------------------------------------------------
  else{   
    Serial.println("Error while connecting");
    lcd.clear();    
    lcd.print("Gagal.");
    lcd.setCursor(0,1); //col=0 row=0
    lcd.print("Coba lagi");
  }
  //----------------------------------------------------------------      
  delay(3000);
  lcd.clear();
}

//=====================================================
void ReadDataFromBlock(int blockNum, byte readBlockData[]) 
{   
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  //----------------------------------------------------------------------------  
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  //----------------------------------------------------------------------------s
  if (status != MFRC522::STATUS_OK){
     Serial.print("Authentication failed for Read: ");
     Serial.println(mfrc522.GetStatusCodeName(status));
     return;
  }
  //----------------------------------------------------------------------------
  else {
    Serial.println("Authentication success");
  }
  //----------------------------------------------------------------------------
  /* Reading data from the Block */
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  //----------------------------------------------------------------------------
  else {
    readBlockData[16] = ' ';
    readBlockData[17] = ' ';
    Serial.println("Block was read successfully");  
  }
  //----------------------------------------------------------------------------
}
