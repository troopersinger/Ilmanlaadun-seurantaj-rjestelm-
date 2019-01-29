#include <LiquidCrystal.h>
#include <dht.h>
#include <DS1307RTC.h>
#include <Time.h>
#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>
#include <HttpClient.h>
#include <Xively.h>

#define dht_dpin A0
#define dht_dpin1 A1
#define dustPin A2
// MAC-osoite
byte mac[] = { 0xDE, 0xAD, 0xBB, 0xEF, 0xFD, 0xED };

//Datastream nimet ja määrittelyt
char sensor1[] = "Jateilman_kosteus";
char sensor2[] = "Jateilman_lampotila";
char sensor3[] = "Hiukkaset";
char sensor4[] = "Tuloilman_kosteus";
char sensor5[] = "Tuloilman_lampotila";
XivelyDatastream datastreams[] = {
XivelyDatastream(sensor1, strlen(sensor1), DATASTREAM_FLOAT),
XivelyDatastream(sensor2, strlen(sensor2), DATASTREAM_FLOAT),
XivelyDatastream(sensor3, strlen(sensor3), DATASTREAM_FLOAT),
XivelyDatastream(sensor4, strlen(sensor4), DATASTREAM_FLOAT),
XivelyDatastream(sensor5, strlen(sensor5), DATASTREAM_FLOAT)};
 
 
// Datastreamien yhdistäminen feediin
XivelyFeed feed(3887585, datastreams, 5 /*Datastreamien määrä*/);

EthernetClient client;
XivelyClient xivelyclient(client);
dht DHT;
dht DHT_2;
char xivelyKey[] = "uu6F5QTHhgPZFzATG0r5bIvQd8CqRAgfSJRy46iSeH6Xuk82"; // Xively key
//Määrittelyt
float dustVal;
int ledPower=2;
int delayTime=280;
int delayTime2=40;
int apumuuttuja=0;
double kerroin=0.823172;
float offTime=9680;

LiquidCrystal lcd(12, 14, 11, 7, 8, 9, 10); //LCD-näytön määrittely

void setup(){
lcd.begin(20, 4); //LCD-näytön(merkit, rivit)
Serial.begin(9600);
pinMode(ledPower,OUTPUT);
pinMode(4, OUTPUT);
pinMode(3, OUTPUT);
delay(300); //DHT11 suositeltu 1s viive
Serial.println("Kosteus ja lämpötila\n\n");
delay(700); //Loput 1s viiveestä
lcd.setCursor(0,0);
lcd.print("Ohjelma kaynnistyy!"); //Tervetuloviesti
Ethernet.begin(mac); //Muodostetaan Ethernet-yhteys. DHCP määrittää IP:n automaattisesti.
  } 
void loop(){
    DHT.read11(dht_dpin); //Luetaan ensimmäisen DHT11 arvot
    DHT_2.read11(dht_dpin1); //Luetaan toisen DHT11 arvot
    tmElements_t tm; //Sallitaan pääsy aikakirjastoihin
    
Serial.print("Jateilman Kosteus = ");
Serial.print(DHT.humidity);
Serial.print("%");
Serial.print("Jateilman Lampotila = ");
Serial.print(DHT.temperature);
Serial.println("C");
Serial.print("Tuloilman Kosteus = ");
Serial.print(DHT_2.humidity);
Serial.print("%");
Serial.print("Tuloilman Lampotila = ");
Serial.print(DHT_2.temperature);
Serial.println("C");
Serial.println("AI2 arvo");
Serial.print(dustVal);
Serial.println("apumuuttuja");
Serial.print(apumuuttuja);
delay(1000);
digitalWrite(ledPower,LOW); // Sytytetään Led (PNP transistori)
delayMicroseconds(delayTime); //280 mikrosekunnin viive
dustVal=analogRead(dustPin); // Luetaan hiukkastieto AI2-portista
if(dustVal < 112.53)  
{
  dustVal=112.53;
}
delayMicroseconds(delayTime2); //40 mikrosekunnin viive
digitalWrite(ledPower,HIGH); // Sammutetaan Led
delayMicroseconds(offTime);
delay(500);
 lcd.setCursor(0,0); // Asetetaan kursorin sijainti
  lcd.print("Tulo=");
  lcd.print(DHT.humidity, 0);
  lcd.print("% ");
  lcd.print("Jate=");
  lcd.print(DHT_2.humidity, 0);
  lcd.print("%");
  lcd.setCursor(0,1); // Vaihdetaan riviä
  lcd.print("Lampotila=");
  lcd.print(DHT.temperature, 0);
  lcd.print("'C");
  lcd.setCursor(0,2);
  lcd.print("Hiukkaset=");
  lcd.print(((dustVal-112.53)*kerroin), 1);
  lcd.print("ug/m3 ");
  lcd.setCursor(0,3);
  if (RTC.read(tm))
  {
  if ((tm.Hour)<10)
  {
    lcd.print("0");
    lcd.print(tm.Hour);
  }
  else
  {
  lcd.print(tm.Hour);
  }
  lcd.print(":");
  if ((tm.Minute)<10)
  {
    lcd.print("0");
    lcd.print(tm.Minute);
  }
  else
  {
  lcd.print(tm.Minute);
  }
  lcd.print("/");
  lcd.print(tm.Day);
  lcd.print("/");
  lcd.print(tm.Month);
  lcd.print("/");
  lcd.print(tmYearToCalendar(tm.Year));
  }

  
  datastreams[0].setFloat(DHT.humidity);
  datastreams[1].setFloat(DHT.temperature);
  datastreams[2].setFloat(((dustVal-112.53)*kerroin));
  datastreams[3].setFloat(DHT_2.humidity);
  datastreams[4].setFloat(DHT_2.temperature);
  
  Serial.println("Ladataan Xivelyyn");
  int ret = xivelyclient.put(feed, xivelyKey);
  Serial.print("xivelyclient.put sai arvon ");
  Serial.println(ret);
  Serial.println();
  if (ret<0)
  {
    Serial.println("Ei yhteytta Xively-palveluun. yritetaan uudestaan...");
    Ethernet.begin(mac); 
  }  
  else
  {
    delay(55000);
}
    if (RTC.read(tm)) {
    Serial.print("Ok, Time = ");
    print2digits(tm.Hour);
    Serial.write(':');
    print2digits(tm.Minute);
    Serial.write(':');
    print2digits(tm.Second);
    Serial.print(", Date (D/M/Y) = ");
    Serial.print(tm.Day);
    Serial.write('/');
    Serial.print(tm.Month);
    Serial.write('/');
    Serial.print(tmYearToCalendar(tm.Year));
    Serial.println();
  } else {
    if (RTC.chipPresent()) {
      Serial.println("DS1307 on pysahtynyt. Aja SetTime esimerkki");
      Serial.println();
    } else {
      Serial.println("DS1307 lukeminen epaonnistui. Tarkista kytkenta");
      Serial.println();
    }

  }
 
if (DHT.humidity-DHT_2.humidity >= 7)
{
++apumuuttuja;
}
if (apumuuttuja==1)
{
delay(100);
digitalWrite(3,HIGH); // Signaali ilmanvaihdon tehostamiseksi
delay(10);
Serial.write("Tehostetaan ilmanvaihtoa");
digitalWrite(3,LOW);
++apumuuttuja;
}
if (DHT.humidity-DHT_2.humidity < 7)
apumuuttuja=0;
else if (apumuuttuja >=30)
apumuuttuja=0;
}
void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}

