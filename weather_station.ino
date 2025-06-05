#include <Adafruit_BME680.h>
#include <WiFi.h>
#include "ThingSpeak.h"

#define uS_TO_S_FACTOR 1000000ULL                                     //Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  300                                         //Time ESP32 will go to sleep (in seconds) - 9.5 minutes / 590 seconds в секундах (1200 - 20 минут)

Adafruit_BME680 bme;                                               

WiFiClient client;                                                  
char ssid[] = "zarus2";                                                //WiFi Name
char pass[] = "170300asd";                                             //WiFi Password

unsigned long myChannelNumber = 1736315;                               //Thingspeak channel number
const char * myWriteAPIKey = "0SJ1P721NYA24UFF";                       //Thingspeak API write key

const int Analog_channel_pin= 4;                                      // Пин для измерения напряжения батареи

int gas = 0;
float temp = 0;
int humid = 0;
int pressure = 0;

int ADC_value = 0;
float vol_bat = 0;

void setup()                                                        
{
  Serial.begin(9600);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);     //активация параметров режима сна
  unsigned status;
  delay(200);                                                        //Allow BME sensor to startup
  status = bme.begin();                                              //подключение к BME
  if (!status)
  {
    Serial.println("Could not find a valid BME280 sensor");
  }

bme.setGasHeater(320, 150);

  readGas();
  recTempHumid ();                                                   //запуск фунций чтения данных с BME 
  recPress ();                                                        

  Serial.print("Temp: ");
  Serial.println(temp);
  Serial.print("Humidity: ");
  Serial.println(humid);
  Serial.print("Pressure: ");
  Serial.println(pressure);
  Serial.print("gas: ");
  Serial.println(gas);
  Serial.print("vol_bat: ");
  Serial.println(vol_bat);
 // Serial.print("ADC_value: ");
  //Serial.println(ADC_value);


  WiFi.begin(ssid, pass);                                            //подключение к WiFi
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED  && timeout < 20)
  {
    delay(500);
    Serial.print(".");
    timeout++;
  }
  if(WiFi.status() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.println("WiFi connected");
    ThingSpeak.begin(client);                                        //активация ThingSpeak
    updateThingSpeak ();                                             //обновление Thingspeak
  }
  else
  {
    Serial.println("");
    Serial.println("WiFi connection failed");
  }
  Serial.println("Going to sleep now");
  Serial.flush();
  esp_deep_sleep_start();                                            //режим сна
}

void loop()                                                          //функция не используется
{

}

void recTempHumid ()                                                 //чтение температуры и давления
{
  temp = bme.readTemperature();
  humid = bme.readHumidity();
  ADC_value = analogRead(Analog_channel_pin);                       // чтение данных с пина ADC
vol_bat = (ADC_value * 7.45 ) / (4095);                             // пересчитывание в вольты (7.38 для напряжения аккамулятора 4.17, подобрано на обум)

}

void recPress ()                                                     //чтение давления
{
  pressure = (bme.readPressure()/100)*0.75;
}

void readGas ()                                                     //чтение газа
{
  gas = bme.readGas()/1000;
}

void updateThingSpeak ()                                             //Отправка поста на Thingspeak
{
  ThingSpeak.setField(1, gas);
  ThingSpeak.setField(2, temp);
  ThingSpeak.setField(3, humid);
  ThingSpeak.setField(4, pressure);
  ThingSpeak.setField(5, vol_bat);

  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200)
  {
    Serial.println("Channel update successful.");
  }
  else
  {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
}