/*
 * Just Barran Youtube
 * Just a Dehumidifier
 * www.youtube.com/c/justbarran
 * like,share,subscribe 
 */


#include <U8g2lib.h>
#include <Wire.h>
#include <MsTimer2.h>
#include "DHT.h"
#define WIRE Wire

//20*30
const unsigned char temp_bmp[] U8X8_PROGMEM = {0x00, 0x06, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x80, 0x19, 0x00, 0x80, 0x19, 0x00, 0xc0, 0x39, 0x00, 0xc0, 0x39, 0x00, 0xc0, 0x39, 0x00, 0xc0, 0x39, 0x00, 0xc0, 0x3f, 0x00, 0x80, 0x1f, 0x00, 0x80, 0x1f, 0x00, 0x00, 0x06, 0x00};
//20*20
const unsigned char hum_bmp[] U8X8_PROGMEM= {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09,0x00,0x80,0x10,0x00,0x00,0x00,0x00,0x40,0x20,0x00,0x20,0x40,0x00,0x20,0x40,0x00,0x10,0x80,0x00,0x10,0x80,0x00,0x08,0x00,0x01,0x08,0x00,0x01,0x08,0x00,0x01,0x08,0x00,0x01,0x08,0x00,0x01,0x08,0x00,0x01,0x10,0x80,0x00,0x00,0x00,0x00,0x40,0x20,0x00,0x80,0x10,0x00};

#define led 4
#define buzzer 5
#define button 6
#define fans 7
#define peltier 8

#define rotary A0
#define light A6

#define low_humid 45 
#define high_humid 50 

#define low_temp 15 
#define high_temp 40 

#define max_hrs 12 
#define display_update_ms 3000 
#define warm_up_ms 5000 
#define cool_down_ms 10000 

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R2, /* reset=*/U8X8_PIN_NONE);
DHT dht(3,DHT11);

#define CLICKS 100

int BuzzerFrequency = 300;
char MODE = 1;
char LongPress = false;
char PressCounter = 0;
char BlinkEnable = true;

byte power_flag = false;
byte power_flag_last = false;

byte started_flag = false;


// Variables will change:
int ledState = HIGH;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

unsigned long lastDisplayUpdate = 0;  

int temp = 0, humid = 0;

//================================================================
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(rotary, INPUT);
  pinMode(button, INPUT);
  pinMode(light, INPUT);
  
  pinMode(led, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(fans, OUTPUT);
  pinMode(peltier, OUTPUT);

  digitalWrite(buzzer, LOW);
  digitalWrite(fans, LOW);
  digitalWrite(peltier, LOW);  
  digitalWrite(led, LOW);
  u8g2.begin();
  Temp_show();
}

void loop() 
{
   if((millis() - lastDisplayUpdate)>display_update_ms)
    {
      Temp_show();
      lastDisplayUpdate = millis();
    }  
    
    // read the state of the switch into a local variable:
    buttonState = digitalRead(button);
    if ((buttonState != lastButtonState) && (buttonState == HIGH)) 
    {
      power_flag = !power_flag;
    }
    lastButtonState = buttonState;

    if((power_flag == false)&&(power_flag_last == true))
    {
      digitalWrite(led, LOW);
    }
    else if((power_flag == true)&&(power_flag_last == false))
    {
      digitalWrite(led, HIGH);
    }
    power_flag_last = power_flag; 

    
    if(power_flag == true)
    {
      int l = analogRead(light);
      int r = analogRead(rotary);
      int t = temp; 
      int h = humid;
   
      Serial.print("light = ");
      Serial.print(l);
      Serial.print("\t rotary = ");
      Serial.println(r);

      if((l <= r) && (t >= low_temp) && (h >= high_humid)) // Conditions for the dehumidifier to start
      {
        if(started_flag == false)
        {
          Start_Dehumidifier();
        }
      }
      else if((l > r)||(h <= low_humid))   // Conditions for the dehumidifier to auto off
      {
        if(started_flag ==true)
        {
          Stop_Dehumidifier();
        }
      }
      else if ((t < low_temp) || (t > high_temp))
      {
        if(started_flag ==true)
        {
          Stop_Dehumidifier();
        }
        analogWrite(buzzer, 200);
        delay(1000);
        analogWrite(buzzer, 0);}
    }  
    else
    {
      if(started_flag ==true)
        {
          Stop_Dehumidifier();
        }
    }
   delay(100);
 }

void Start_Dehumidifier()
{
  
  digitalWrite(fans, HIGH);
  delay(warm_up_ms);
  analogWrite(buzzer, 200);
  delay(1000);
  analogWrite(buzzer, 0);
  digitalWrite(peltier, HIGH);  
  started_flag = true;
}

void Stop_Dehumidifier()
{
  analogWrite(buzzer, 200);
  delay(1000);
  analogWrite(buzzer, 0);
  digitalWrite(peltier, LOW);
  delay(cool_down_ms);
  digitalWrite(fans, LOW);   
  started_flag = false; 
}


void Temp_show()
{
  humid = dht.readHumidity();
  temp = dht.readTemperature();
  int l = analogRead(light);
  int r = analogRead(rotary);
  
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_t0_16b_mr);
    u8g2.setCursor(32, 12);
    u8g2.print(F("temp:"));
    u8g2.setCursor(72, 12);
    u8g2.print(temp);
    u8g2.setCursor(96, 12);
    u8g2.print("C");
    u8g2.setCursor(32, 28);
    u8g2.print(F("humid:"));
    u8g2.setCursor(80, 28);
    u8g2.print(humid);
    u8g2.setCursor(104, 28);
    u8g2.print("%");
    
    u8g2.setCursor(32, 44);
    u8g2.print(F("light:"));
    u8g2.setCursor(80, 44);
    u8g2.print(l);

    u8g2.setCursor(32, 60);
    u8g2.print(F("rotary:"));
    u8g2.setCursor(88, 60);
    u8g2.print(r);
    
    u8g2.drawLine(25, 0, 25, 64);
    //u8g2.drawLine(103, 0, 103, 64);
    u8g2.drawXBMP(0, 0, 20, 30, temp_bmp);
    u8g2.drawXBMP(0, 35, 20, 20, hum_bmp);
  } while (u8g2.nextPage());
}
