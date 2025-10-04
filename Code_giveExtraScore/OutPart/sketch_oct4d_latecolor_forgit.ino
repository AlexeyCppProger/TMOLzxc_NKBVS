#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <LiquidCrystal.h>        

LiquidCrystal lcd(12, 10, 6, 4, 3, 2);

#define redpin 3
#define greenpin 5
#define bluepin 6

#define commonAnode true


byte gammatable[256];
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
String bits;
String latecolor = "w", textOutput, message;

void setup() {
  Serial.begin(9600);
  Serial.println("Запуск системы...");

  lcd.begin(16, 2);
  lcd.print("Init LCD...");      
  if (!tcs.begin()) {
    lcd.clear();
    lcd.print("TCS34725 FAIL");
    Serial.println("ERROR: TCS34725 not found");
    while (1) { }
  }

  lcd.clear();
  lcd.print("Ready!");
  Serial.println("TCS34725 initialized successfully");
  delay(1000);   
  lcd.clear();  
  //////
  
  pinMode(redpin, OUTPUT);
  pinMode(greenpin, OUTPUT);
  pinMode(bluepin, OUTPUT);


  for (int i=0; i<256; i++) {
    float x = i;
    x /= 255;
    x = pow(x, 2.5);
    x *= 255;

    if (commonAnode) {
      gammatable[i] = 255 - x;
    } else {
      gammatable[i] = x;
    }
    
  }
}


void loop() {
  float red, green, blue;
  String colorName;
  
  tcs.setInterrupt(false); 

  delay(60);

  tcs.getRGB(&red, &green, &blue);
  
  tcs.setInterrupt(true);

  const float per = 35;
  if (red > green + per && red > blue + per && latecolor != "r") {
    colorName = "RED";
    bits += '1';
    latecolor = "r";
  }       
  else if (green > red + per && green > blue + per) {
    colorName = "GREEN";
    latecolor = "g";
    textOutput = bitStreamToText(bits);

    Serial.print(bits);
    Serial.print("              ");
    Serial.print(textOutput);
    Serial.println("");
    message = textOutput;
    bits = "";
    //lcd.clear();
  }  
  else if (blue > red + per && blue > green + per && latecolor != "b") {
    colorName = "BLUE";
    bits += '0';
    latecolor = "b";
  }  
  else {
    colorName = "WHITE";
    latecolor = "w";
  }

  if (textOutput != "") {
    textOutput = "";
    lcd.clear();
  }
  lcd.setCursor(0, 0);
  lcd.print(message);
  //lcd.setCursor(0, 1);

  Serial.print("R:\t"); Serial.print(int(red)); 
  Serial.print("\tG:\t"); Serial.print(int(green)); 
  Serial.print("\tB:\t"); Serial.print(int(blue));
  Serial.print("              ");
  Serial.print(colorName);
  Serial.print("              ");
  Serial.println("");
  
  analogWrite(redpin, gammatable[(int)red]);
  analogWrite(greenpin, gammatable[(int)green]);
  analogWrite(bluepin, gammatable[(int)blue]);

}






// обратная декодировка из бит в текст
String bitStreamToText(const String &bits) {
  String result = "";
  int n = bits.length() / 8;      

  for (int i = 0; i < n; i++) {
    byte b = 0;
    for (int bit = 0; bit < 8; bit++) {
      char c = bits.charAt(i * 8 + bit);
      if (c == '1') {
        // Устанавливаем в b бит (7-bit) MSB first
        b |= (1 << (7 - bit));
      }
    }
    // Преобразуем полученный байт в символ и добавляем к результату
    result += (char)b;
  }
  return result;
}




