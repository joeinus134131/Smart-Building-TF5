/*
 * Tubes Metode Pengukuran - TF
 * Author      : Made Agus Andi Gunawan
 * Company     : IDNMakerspace Algorithm Factory  
 * Last Edited : 6 Des 2020
 * Reference   : create.arduino.cc
 */

//library
#include <dht11.h>
#include <SoftwareSerial.h> 
#include <Wire.h>
#include <math.h>                                   

//koneksi ke wifi
String namawifi = "WIFI@ITERA";             
String passwordnya = "namasayamadeagusandi446";           

//variabel
int rxPin = 10;                                              
int txPin = 11;                                              
int ledpin1 = 6;
int ledpin2 = 5;
int pinfan = 12;
int dht11Pin = A0;
int pinsound = A1;
int ldrsensor = A2;
int BH1750address = 0x23;
float suhu, kelembaban;
byte buff[2];
String ip = "184.106.153.149";
int ambang = 540;                                

dht11 DHT11;
SoftwareSerial esp(rxPin, txPin);                             

void setup() 
{  
  Wire.begin();
  Serial.begin(9600);  
  Serial.println("Mulai");
  esp.begin(115200);                                        
  esp.println("AT");                                         
  Serial.println("AT  kirim ");
  pinMode(ledpin1, OUTPUT);
  pinMode(ledpin2, OUTPUT);
  pinMode(pinfan, OUTPUT);
  pinMode(pinsound, INPUT);
  pinMode(ldrsensor, INPUT);
  digitalWrite(ledpin1, HIGH);
  digitalWrite(ledpin2, HIGH);

  //cek esp
  while(!esp.find("OK")){                                   
    esp.println("AT");
    Serial.println("ESP8266 tidak ditemukan.");
  }
  Serial.println("OK Command diterima");
  esp.println("AT+CWMODE=1");                               
  while(!esp.find("OK")){                                     
    esp.println("AT+CWMODE=1");
    Serial.println("Proses ....");
  }
  Serial.println("Set as client");
  Serial.println("Menghubungkan ...");
  esp.println("AT+CWJAP=\""+namawifi+"\",\""+passwordnya+"\"");    
  while(!esp.find("OK"));                                     
  Serial.println("Terhubung");
  delay(1000);
}

void loop() 
{
  esp.println("AT+CIPSTART=\"TCP\",\""+ip+"\",80");         
  if(esp.find("gagal")){                                   
    Serial.println("AT+CIPSTART gagal");
  }

  //data dht11
  DHT11.read(dht11Pin);
  suhu = (float)DHT11.temperature;
  kelembaban = (float)DHT11.humidity;

  //data bh1750
  int i;
  uint16_t lux=0;
  BH1750_Init(BH1750address);
  delay(200);
 
  if(2==BH1750_Read(BH1750address))
  {
    lux=((buff[0]<<8)|buff[1])/1.2;
    Serial.print(lux,DEC);     
    Serial.println("lux"); 
  }

  //data sound sensor
  float outsensor = 0;
  outsensor = analogRead(pinsound);
  delay(100);

  //data ldr
  float cahaya = 0;
  cahaya = analogRead(ldrsensor);

  //kondisi cahaya
  if(cahaya < 50)
  {
    digitalWrite(ledpin1, HIGH);
    digitalWrite(ledpin2, HIGH);
    delay(120);
  }
  else if(cahaya>=50)
  {
    digitalWrite(ledpin1, LOW);
    digitalWrite(ledpin2, LOW);
    delay(120);
  }
  
  //kondisi lux sensor
  if(lux == 1000)
  {
    digitalWrite(ledpin1, 50);
    digitalWrite(ledpin2, 50);
    delay(120);
  }
  else if(lux >= 500 && lux<1000 )
  {
    digitalWrite(ledpin1, 100);
    digitalWrite(ledpin2, 100);
    delay(120);
  }
  else if(lux >= 100 && lux<500)
  {
    digitalWrite(ledpin1, 200);
    digitalWrite(ledpin2, 200);
    delay(120);
  }
  else if(lux >= 0 && lux<100)
  {
    digitalWrite(ledpin1, 255);
    digitalWrite(ledpin2, 255);
    delay(120);
  }

  //kondisi sensor suhu
  if(suhu > 28)
  {
    digitalWrite(pinfan, HIGH);
    delay(120);
  }
  else if(suhu<=28)
  {
    digitalWrite(pinfan, LOW);
    delay(120);
  }

  //kondisi sensor kebisingan
  if(outsensor > ambang)
  {
    Serial.print("Ribut");
    delay(100);
  }
  else if(outsensor > ambang)
  {
    Serial.print("Aman");
    delay(100);
  }
  delay(150);
  
  //mengirim data ke cloud
  String tautan = "GET https://api.thingspeak.com/update?api_key=4AZZPUAXNCKLWUQT";   
  tautan += String(suhu);
  tautan += "&field2=";
  tautan += String(kelembaban);
  tautan += "&field3=";
  tautan += String(lux);
  tautan += "&field4=";
  tautan += String(outsensor);
  tautan += "\r\n\r\n"; 
  esp.print("AT+CIPSEND=");                                   
  esp.println(tautan.length()+2);
  delay(2000);
  if(esp.find(">"))
  {                                         
    esp.print(tautan);                                       
    Serial.println(tautan);
    Serial.println("Data Terkirim");
    delay(1000);
  }
  Serial.println("Tutup koneksi");
  esp.println("AT+CIPCLOSE");                               
  delay(1000);                                               
}

//inisialisasi addres sensor bh1750
int BH1750_Read(int address) 
{
  int i=0;
  Wire.beginTransmission(address);
  Wire.requestFrom(address, 2);
  while(Wire.available()) 
  {
    buff[i] = Wire.read();  
    i++;
  }
  Wire.endTransmission();  
  return i;
}
 
void BH1750_Init(int address) 
{
  Wire.beginTransmission(address);
  Wire.write(0x10);
  Wire.endTransmission();
}
