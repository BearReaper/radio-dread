#include <SPI.h>
#include <RH_RF95.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

// Singleton instance of the radio driver
RH_RF95 rf95;
float frequency = 915.0;  //frequency settings
TinyGPS gps;
SoftwareSerial ss(3, 4); // Arduino RX, TX to conenct to GPS module.

static void smartdelay(unsigned long ms);

unsigned int count = 1;        //For times count
char *node_id = "<12345>";  //From LG01 via web Local Channel settings on MQTT. This field determins  witch channel to update so keep in mind!

String datastring1="";        
String datastring2="";        
String datastring3="";
char datasent[90];     //Used to store GPS data for uploading

char gps_lon[20]={"\0"};  //Storage GPS info
char gps_lat[20]={"\0"}; //Storage latitude
char gps_alt[20]={"\0"}; //Storage altitude
float flat, flon,falt;

//static uint8_t mydata[] = "Hello, world!";      //For test using.

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  ss.begin(9600);  
  while(!Serial);
    Serial.println("LoRa GPS Example---- ");
  Serial.println("Connect to TTN");
  if (!rf95.init())
      Serial.println(F("init failed"));
  rf95.setFrequency(frequency);
  rf95.setTxPower(13);
}

void GPSRead()
{
  unsigned long age;
  gps.f_get_position(&flat, &flon, &age);
  falt=gps.f_altitude();  //get altitude       
  flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6;//save six decimal places 
  flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6;
  falt == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : falt, 2;//save two decimal places 
  //Serial.println("thats just weird");
}

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
    {
      gps.encode(ss.read());
    }
  }
  while (millis() - start < ms);
}

void GPSWrite()
{
    char data[90] = "\0";
    float rss;
    for(int i = 0; i < 90; i++)
    {
       data[i] = node_id[i];
    }
    dtostrf(flon, 0, 5, gps_lon);  
    dtostrf(flat, 0, 5, gps_lat);
    dtostrf(falt, 0, 2, gps_alt);
    
    //some var for storing the RSSI
    //rss=rf95.lastRssi();
    //dtostrf(rss,0,1,rssi_1);

    Serial.println((char *)data);
    strcat(data,"field4=");
    strcat(data,gps_lon);
    strcat(data,"&field5=");
    strcat(data,gps_lat);
    strcat(data,"&field6=");
    strcat(data,gps_alt);
    
    strcpy((char *)datasent,data);
     
    Serial.println((char *)datasent);
    Serial.println(sizeof datasent);
 
    smartdelay(1000);
  /*Convert GPS data to format*/
  /*
  datastring1 +=dtostrf(flat, 0, 6, gps_lat);   
  datastring2 +=dtostrf(flon, 0, 6, gps_lon);
  datastring3 +=dtostrf(falt, 0, 2, gps_alt);
  Serial.println("this is so random4546");
    
  if(flon!=1000.000000)
  {  
    Serial.println("this is so random6666");
    strcat(gps_lon,",");
    strcat(gps_lon,gps_lat); 
    strcat(gps_lon,","); 
    strcat(gps_lon,gps_alt);
    
    strcpy(datasend,gps_lon); //the format of datasend is longtitude,latitude,altitude
    Serial.print("###########    ");
    Serial.print("NO.");
    Serial.print(count);
    Serial.println("    ###########");
    Serial.println("The longtitude and latitude and altitude are:");
    Serial.print("[");
    Serial.print(datasend);
    Serial.print("]");
    Serial.println("");
    Serial.println("");
    count++;
  }
  smartdelay(1000);
  //delay(1000);*/
}

void SendData()
{
      //Serial.println(F("Sending data to LG01"));
      //Serial.println((char *)datasent);
           
   
      rf95.send((char *)datasent,sizeof(datasent));  
      rf95.waitPacketSent();  // Now wait for a reply
    
      uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
      uint8_t len = sizeof(buf);

  if (rf95.waitAvailableTimeout(3000))
  { 
        // Should be a reply message for us now   
        if (rf95.recv(buf, &len))
        {
        Serial.print("got reply from LG01: ");
        Serial.println((char*)buf);
        Serial.print("RSSI: ");
        Serial.println(rf95.lastRssi(), DEC);    
        }
    else
    {
      Serial.println("recv failed");
    }
  }
  else
  {
    Serial.println("No reply, is LoRa server running?");
  }
  delay(5000);
}

void loop() {
  
  // put your main code here, to run repeatedly:
  //Serial.print("###########    ");
  //Serial.print("COUNT=");
  //Serial.print(count);
  //Serial.println("    ###########");
  //count++;
  GPSRead();
  GPSWrite();
  SendData();

}
