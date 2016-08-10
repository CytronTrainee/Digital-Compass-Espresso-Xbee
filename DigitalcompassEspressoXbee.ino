#include <ESPert.h>

#include <Wire.h>  //i2c library for the Digital Compass

ESPert espert;

const int hmc5883Address = 0x1E; //0011110b, I2C 7bit address for compass
const byte hmc5883ModeRegister = 0x02;
const byte hmcContinuousMode = 0x00;
const byte hmcDataOutputXMSBAddress = 0x03;

const char *host = "api.thingspeak.com"; 
String apiKey = "6N50TKJ0OBD19GUN"; //replace with your channel's thingspeak API key 

void setup()
{
 
  espert.init();  
  espert.oled.init(); 
  delay(2000);
  Wire.begin(); // Start the i2c communication

  espert.oled.clear(); 
  espert.oled.println(espert.info.getId()); 
  espert.oled.println();

  int mode = espert.wifi.init(); 
  
  if (mode == ESPERT_WIFI_MODE_CONNECT) 
  { 
    espert.println(">>> WiFi mode: connected."); 
    espert.oled.println("WiFi: connected."); 
    espert.oled.print("IP..: "); 
    espert.oled.println(espert.wifi.getLocalIP()); 
  } 
  else if (mode == ESPERT_WIFI_MODE_DISCONNECT) 
  { 
    espert.println(">>> WiFi mode: disconnected."); 
    espert.oled.println("WiFi: not connected."); 
  } 
  else if (mode == ESPERT_WIFI_MODE_SMARTCONFIG) 
  { 
    espert.println(">>> WiFi mode: smart config."); 
  } 
  else if (mode == ESPERT_WIFI_MODE_SETTINGAP) 
  { 
    espert.println(">>> WiFi mode: access point."); 
  } 
  delay(2000); 
  
  //Initialise the Digital Compass
  Wire.beginTransmission(hmc5883Address);  //Begin communication with compass
  Wire.write(hmc5883ModeRegister);  //select the mode register
  Wire.write(hmcContinuousMode); //continuous measurement mode
  Wire.endTransmission();

}

void loop()
{
  
  espert.loop();
  
  int16_t x, y, z;
  //Tell the HMC5883L where to begin reading the data
  Wire.beginTransmission(hmc5883Address);
  Wire.write(hmcDataOutputXMSBAddress);  //Select register 3, X MSB register
  Wire.endTransmission();

  //Read data from each axis
  Wire.requestFrom(hmc5883Address,6);
  if(6<=Wire.available())
  {
    x = Wire.read()<<8; //X msb
    x |= Wire.read();   //X lsb
    z = Wire.read()<<8; //Z msb
    z |= Wire.read();   //Z lsb
    y = Wire.read()<<8; //Y msb
    y |= Wire.read();   //Y lsb    
  }


  int angle = atan2(y,x)/M_PI*180;
  if (angle < 0)
  {
    angle = angle + 360;
  }

  String path = "/update?api_key=" + apiKey + "&field1=" + (String)(x) + "&field2=" + (String)(y) + "&field3=" + (String)(angle);
  String outString = "{\"x\":\"" + String(x) + "\", ";
  outString += "\"y\":\"" + String(y) + "\", ";
  outString += "\"Direction_angle(degree)\":\"" + String(angle) + "\", ";
  outString += "\"name\":\"" + String(espert.info.getId()) + "\"}";
  espert.println(outString); 
  espert.wifi.getHTTP(host, path.c_str()); 
    
  Serial.begin(9600);
}

