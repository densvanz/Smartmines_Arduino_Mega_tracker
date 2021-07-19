#include <SPI.h>
#include <MFRC522.h>
//#include <SoftwareSerial.h>    //Libraries required for the Serial communication and DHT
//#include <TinyGPS++.h>
#include <DFRobot_sim808.h>


#define BT_Serial Serial1
//----- ---- -------GPS Section------ ---- ---- ---

// Create a TinyGPS++ object
//TinyGPSPlus gps;
  // Use Serial2 port called "gpsSerial"
#define gpsSerial Serial2
DFRobot_SIM808 sim808(&Serial2);//Connect RX,TX,PWR,

//SoftwareSerial gps_Serial(RXPin, TXPin); // RX,TX for Arduino and for the module it's TXD RXD, they should be inverted
  //---- ------ -----End of GPS section--- ---- -----

/* Define the DIO used for the SDA (SS) and RST (reset) pins. */
//Arduino Mega SPI Pins - 50(MISO), 51(MOSI), 52(SCK),
#define SS_PIN 49 //SDA
#define RST_PIN 48
#define esp32_Serial Serial3
MFRC522 rfid(SS_PIN, RST_PIN);
// Init array that will store new NUID
byte nuidPICC[4];
String RFID_String;
String character;
String App_Data;
String Tracking_Data;
String GPS_Loc;
String ESP_Data="";

void setup() {
 
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
  gpsSerial.begin(9600);
  BT_Serial.begin(9600);
  esp32_Serial.begin(9600);
    
}

void loop() {

  RFID_String=readRFID();
  
  if(RFID_String!=""){
    //Write to BT
    BT_Serial.println(RFID_String);
    Serial.println(RFID_String);
  }
  RFID_String="";
    
  while(BT_Serial.available()>0) {
    character = (char)BT_Serial.read();
    delay(1); //wait for the next byte, if after this nothing has arrived it means the text was not part of the same stream entered by the user
    App_Data+=character;
  }

  
  if(App_Data.length()>10){
    //Serial.println(App_Data);   
    //Get GPS Data
   /*
    int retry=0;  
    // - - - - - - -- GPS - - - - - - - 
    while(retry<5){
      if(!sim808.attachGPS()){
        Serial.println("Open the GPS power failure");
        retry++;
      }
      else{
        retry=999;
      }
    }
    if (sim808.getGPS()){
      //Serial.println(sim808.GPSdata.lat,6);
      //Serial.println(sim808.GPSdata.lon,6);
      //Serial.println(sim808.GPSdata.speed_kph);
      //Serial.println(sim808.GPSdata.altitude);
      GPS_Loc=String(sim808.GPSdata.lat,6)+"|"+String(sim808.GPSdata.lon,6)+"|"+String(sim808.GPSdata.speed_kph);
      sim808.detachGPS();
    }
    else{
      
    }
    // - - - - - - --End of GPS - - - - - - - 
*/
    //Write to Server
    String FL="80";
    GPS_Loc="";
    Tracking_Data=App_Data+"|"+String(FL);
    App_Data="";
    //String Tracking_Data2 = App_Data + "A1B2C3D4" + "|" + "A1B2C3D4" + "|" + "3:10PM" + "|" + "3:10PM" + "|" + "Status" + "|" + "Lat"+ "|" + "Long"+ "|" + "Speed" + "|" + FL;
    esp32_Serial.print(Tracking_Data);
    esp32_Serial.println();
    //BT_Serial.println(Tracking_Data);
    Serial.println(Tracking_Data);
  }
  while(esp32_Serial.available()>0){
 
    character = (char)esp32_Serial.read();
    delay(1); //wait for the next byte, if after this nothing has arrived it means the text was not part of the same stream entered by the user
    ESP_Data+=character;
  }
  if(ESP_Data!="")
    BT_Serial.print(ESP_Data);
  ESP_Data="";
  App_Data="";
  Tracking_Data="";

}

String readRFID(){

  String RFID_string_local="";
  // Look for new card
  if ( ! rfid.PICC_IsNewCardPresent()) 
    return RFID_string_local;
  
    // Verify if the NUID has been readed
  if (  !rfid.PICC_ReadCardSerial())
    return RFID_string_local;
  
  /*if (rfid.uid.uidByte[0] != nuidPICC[0] || 
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] )*/ {
    //Serial.println(F("A new card has been detected."));
  
  // Store NUID into nuidPICC array
  for (byte i = 0; i < 4; i++) {
    nuidPICC[i] = rfid.uid.uidByte[i];
  }
   
  //Serial.print(F("RFID tag in HEX: "));
  RFID_string_local=printDec(rfid.uid.uidByte, rfid.uid.size);
  //Serial.println();
  }
   
  // Halt PICC
  rfid.PICC_HaltA();
  
  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
  
  return RFID_string_local;

}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
String printDec(byte *buffer, byte bufferSize){
  String RFID_string_local;
  for (byte i = 0; i < bufferSize; i++){
    //Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    //Serial.print(buffer[i], DEC);
    
    RFID_string_local.concat(String(buffer[i] < 0x10 ? " 0" : ""));
    RFID_string_local.concat(String(buffer[i], HEX));
  }
  RFID_string_local.toUpperCase();
  
  if(RFID_string_local.length()==8){
    return RFID_string_local;
  }  
  else
    RFID_string_local="";
}


String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
