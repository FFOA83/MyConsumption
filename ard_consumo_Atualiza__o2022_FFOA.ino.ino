#include <DS1307.h>                    //Biblioteca do relógio clck
#include <Wire.h>                      // Biblioteca para comunicação I2C/TWI SPA: Linha de daods e SCL: Linha de clock
#include <SD.h>                       //Biblioteca do Cartão de memória SD
#include <Adafruit_ADS1015.h>        //Biblioteca para incluir o módulo conversor de sinal analógico para digital
#include "RTClib.h"                 //Biblioteca para o datalogger
//#include <EEPROM.h>                // Biblioteca para gravação dos dados em memória
#include "EmonLib.h"              //Biblioteca para TP 
#include <LiquidCrystal.h>       //Biblioteca para o display de LED 16x2
#include <SPI.h>                //Biblioteca para comunicação SPI

////BOTÕES DO LCD////
//Select 720; left 478; right 0; Up 129; Down 305

#define E 9
#define RS 8
#define D4 4
#define D5 5
#define D6 6
#define D7 7
#define pinBackLight 10
#define pinButton A0

LiquidCrystal lcd(RS, E, D4, D5, D6, D7); //(RS, Enable, D4, D5, D6, D7)
EnergyMonitor emon1; 
Adafruit_ADS1115 ads(0x48);
const float SetPoint = 50;      //50A/1V
const float multiplicador = 0.095f;
//const int offset ;
//int addr = 0;     //endereço na eeprom para armazenar dado.
RTC_DS3231 rtc;
File dataFile;
DateTime now;
const int chipSelect = 53;
int tempo_inicial = millis();
float consumo;
char daysOfTheWeek[7][12]= {"domingo","segunda","terça","quarta","quinta","sexta","sábado"}; 
//////////////////////////
///////////SETUP//////////
//////////////////////////
void setup() {
  
###INCIALIZAÇÃO DOS MÓDULO###
  ads.begin();
  lcd.begin(16, 2);  
  Wire.begin();
  rtc.begin();  
  Serial.begin(9600);
  SD.begin();
  SPI.begin(); 

  pinMode(pinBackLight, OUTPUT);
  digitalWrite(pinBackLight, LOW);
  
  emon1.current(ads.readADC_Differential_0_1(), 111.1); // Current: input pin, calibration.
  emon1.voltage(8, 234.26, 1.7);   // Voltage: input pin, calibration, phase_shift
  
  ads.setGain(GAIN_TWO);        // ±2.048V  1 bit = 0.0848mV  
  
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }else{
    Serial.println("DataLogger OK!");
    delay(1000);
  }
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    return;
  }else{
    Serial.println("Cartao SD OK!");
    delay(3000);
  }
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    return;
  }else{
    lcd.print("Cartao SD OK!");
    delay(3000);
    //lcd.clear();
  }
  Serial.println("card initialized.");
  
###GRAVAÇÃO DOS CABEÇALHOS NO CARTÃO 'SD EM FORMATO .txt###
  DateTime now = rtc.now();
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  dataFile.print("Start logging on: ");
  dataFile.print(now.year(),DEC);
  dataFile.print('/');
  dataFile.print(now.month(),DEC);
  dataFile.print('/');
  dataFile.print(now.day(),DEC);
  dataFile.println(" ");
  dataFile.println("Data Hora Tensão Corrente Consumo(kwh)");
  dataFile.close();
  Serial.print(tempo_inicial);
  Serial.print("Start logging on: ");
  Serial.print(now.year(),DEC);
  Serial.print('/');
  Serial.print(now.month(),DEC);
  Serial.print('/');
  Serial.print(now.day(),DEC);
  Serial.println(" ");
  Serial.println("Data Hora Tensão Corrente Consumo(kwh)");
  
###MOSTRA NO DISPLAY QUE O DISPLAY ESTÁ OK###
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Display OK");
  delay(1000);
  lcd.clear();
  } 
/////////////////////////////
//////INÍCIO DO LOOP////////
///////////////////////////
void loop(){ 
  int selectButton = analogRead(pinButton);
  int TempoBackLight;
  int tempoI = millis();
  float tempo_final = millis() - tempo_inicial; 
  emon1.calcVI(20,2000);                           // Calculate all. No.of half wavelengths (crossings), time-out
  emon1.serialprint(); 
  float realPower  = emon1.realPower;            //extract Real Power into variable
  float apparentPower = emon1.apparentPower;    //extract Apparent Power into variable
  float FP = emon1.powerFactor;                //extract Power Factor into Variable
  float TensaoRMS = emon1.Vrms;               //extract Vrms into Variable
  float CorrenteRMS = emon1.Irms;            //extract Irms into Variable 
  
###REGISTRA EM MEMÓRIA 'SD OS VALORES INSTANTÂNEOS LIDOS NOS MÓDULOS###
  DateTime now = rtc.now();
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile){
     dataFile.print(now.day(),DEC);
     dataFile.print('/');
     dataFile.print(now.month(),DEC);
     dataFile.print('/');
     dataFile.print(now.year(),DEC);
     dataFile.print("        ");
     dataFile.print(now.hour(),DEC);
     dataFile.print(":");
     dataFile.print(now.minute(),DEC);
     dataFile.print(":");
     dataFile.print(now.second(),DEC);
     dataFile.print("        ");
     dataFile.print(TensaoRMS);
     dataFile.print("        ");
     dataFile.print(CorrenteRMS);
     dataFile.print("         ");
     dataFile.println(consumo, 4);   
     dataFile.close();

###TESTE DE VALORS###
     Serial.print(now.day(),DEC);
     Serial.print('/');
     Serial.print(now.month(),DEC);
     Serial.print('/');
     Serial.print(now.year(),DEC);
     Serial.print("        ");
     Serial.print(now.hour(),DEC);
     Serial.print(":");
     Serial.print(now.minute(),DEC);
     Serial.print(":");
     Serial.print(now.second(),DEC);
     Serial.print("        ");
     Serial.print(TensaoRMS);
     Serial.print("        ");
     Serial.print(CorrenteRMS); 
     Serial.print("           ");
     Serial.print(consumo, 2);  
     Serial.print("            ");
     Serial.print(now.unixtime());
     Serial.print("           ");
     Serial.println(tempo_final/1000);      
  }
  
###BOTÃO PARA ACIONAMENTO DA LUZ DE FUNDO DO DISPLAY### 
 if((selectButton < 800) && (selectButton >= 600)){
   digitalWrite(pinBackLight, HIGH);
   TempoBackLight = millis(); 
 }
 if((millis() - TempoBackLight) >= 5){
  digitalWrite(pinBackLight, LOW); 
 }
}
