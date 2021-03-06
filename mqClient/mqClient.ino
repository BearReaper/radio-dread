#include <dht.h>
#include <SPI.h>
#include <RH_RF95.h>

#define DHT11_PIN A0

// Singleton instance of the radio driver
RH_RF95 rf95;
float frequency = 915.0;  //frequency settings
dht DHT;

float temperature,humidity,temp,humi;
String datastring1="";
String datastring2="";
char tem_1[8]={"\0"},hum_1[8]={"\0"};
char *node_id = "<12345>";  //From LG01 via web Local Channel settings on MQTT. This field determins  witch channel to update so keep in mind!
uint8_t datasend[36];
unsigned int count = 1;

void setup()
{
      Serial.begin(9600); // console baud rate
      Serial.println(F("Start MQTT Example"));
          if (!rf95.init())
      Serial.println(F("init failed"));
         rf95.setFrequency(frequency);
         rf95.setTxPower(13);
}

void dhtTem()
{
       temperature = DHT.read11(DHT11_PIN);    //Read Tmperature data
       temp = DHT.temperature*1.0;      
       humidity = DHT.read11(DHT11_PIN);      //Read humidity data
       //Serial.print(humidity);
       humi = DHT.humidity;
                    
       Serial.println(F("The temperature and humidity:"));
       Serial.print("[");
       Serial.print(temp);
       Serial.print("℃");
       Serial.print(",");
       Serial.print(humi);
       Serial.print("%");
       Serial.print("]");
       Serial.println("");
       delay(2000);
}
void dhtWrite()
{
    char data[50] = "\0";
    for(int i = 0; i < 50; i++)
    {
       data[i] = node_id[i];
    }

    dtostrf(temp,0,1,tem_1);
    dtostrf(humi,0,1,hum_1);

     //Serial.println(hum_1);
     strcat(data,"field1=");
     strcat(data,tem_1);
     strcat(data,"&field2=");
     strcat(data,hum_1);
     strcpy((char *)datasend,data);
     
   //Serial.println((char *)datasend);
    //Serial.println(sizeof datasend);
      
}

void SendData()
{
      Serial.println(F("Sending data to LG01"));
           
   
      rf95.send((char *)datasend,sizeof(datasend));  
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
    
void loop()
{
    Serial.print("###########    ");
    Serial.print("COUNT=");
    Serial.print(count);
    Serial.println("    ###########");
     count++;
     dhtTem();
     dhtWrite();
     SendData();
}
