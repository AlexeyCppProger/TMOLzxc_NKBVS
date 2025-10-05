#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <LiquidCrystal.h>
#include <IRremote.h>

LiquidCrystal lcd(12, 10, 6, 4, 3, 2);

#define redpin 3
#define greenpin 5
#define bluepin 6

#define commonAnode true


byte gammatable[256];
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
String bits;
String latecolor = "w", textOutput, message;


// Офсеты тёмного (без света)
uint16_t R_dark=0, G_dark=0, B_dark=0, C_dark=0;
// Коэффициенты выравнивания
float K_R=1.0, K_G=1.0, K_B=1.0;




void calibrateSensor() {
  // 1. Снимаем «тёмный» оффсет: без подсветки и без внешнего света
  tcs.setInterrupt(true);    // гасим LED
  delay(100);
  tcs.getRawData(&R_dark, &G_dark, &B_dark, &C_dark);

  // 2. Снимаем «белый» под текущим освещением
  tcs.setInterrupt(false);   // включаем LED (если он есть)
  delay(200);
  const int N = 10;
  uint32_t R_sum=0, G_sum=0, B_sum=0;
  for (int i=0; i<N; i++) {
    uint16_t r, g, b, c;
    tcs.getRawData(&r, &g, &b, &c);
    R_sum += (r - R_dark);
    G_sum += (g - G_dark);
    B_sum += (b - B_dark);
    delay(50);
  }
  // Усредняем
  float R_ref = float(R_sum) / N;
  float G_ref = float(G_sum) / N;
  float B_ref = float(B_sum) / N;
  // Среднее значение каналов
  float S_avg = (R_ref + G_ref + B_ref) / 3.0;

  // 3. Вычисляем коэффициенты
  K_R = S_avg / R_ref;
  K_G = S_avg / G_ref;
  K_B = S_avg / B_ref;
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


///////////////////////////////////////////////////////////////////////
  delay(100);
  calibrateSensor();
  Serial.println("Калибровка завершена.");
  ///////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////
// Снимаем тёмный оффсет
  float r0 = float(red) - R_dark;
  float g0 = float(green) - G_dark;
  float b0 = float(blue) - B_dark;

  // Применяем белый баланс
  float R_corr = r0 * K_R;
  float G_corr = g0 * K_G;
  float B_corr = b0 * K_B;



  // Дополнительно можно нормировать к 0–255
  float maxv = max(max(R_corr, G_corr), B_corr);
  if (maxv > 0) {
    R_corr = R_corr / maxv * 255.0;
    G_corr = G_corr / maxv * 255.0;
    B_corr = B_corr / maxv * 255.0;
  }
//////////////////////////////////////////////////////////////////////////////////


  
  tcs.setInterrupt(false); 

  delay(60);

  tcs.getRGB(&red, &green, &blue);
  
  tcs.setInterrupt(true);

  const float per = 35;
  /*
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
*/


  /////////////////////////////////////////////////////////////////////
  if ( red   > green + per && 
       red   > blue  + per && 
       latecolor != "r" ) {
    // обнаружили переход на красный
    latecolor = "r";
    colorName = "RED";
    bits += '1';
  }
  else if ( green > red   + per && 
            green > blue  + per && 
            latecolor != "g" ) {
    // переход на зелёный – знак конца «слова»
    latecolor   = "g";
    colorName = "GREEN";
    textOutput  = bitStreamToText(bits);
    Serial.print(bits);
    Serial.print("              ");
    Serial.print(textOutput);
    Serial.println("");
    message = textOutput;
    bits = "";
  }
  else if ( blue  > red   + per && 
            blue  > green + per && 
            latecolor != "b" ) {
    // переход на синий
    latecolor = "b";
    colorName = "BLUE";
    bits += '0';
  }
  else {
    // все остальные случаи (white или «неопределённый»)
    latecolor = "w";
    colorName = "WHITE";
  }
  ///////////////////////////////////////////////////////////////////////



  if (textOutput != "") {
    textOutput = "";
    lcd.clear();
  }
  lcd.setCursor(0, 0);
  lcd.print(message);
  lcd.setCursor(0, 1);

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
