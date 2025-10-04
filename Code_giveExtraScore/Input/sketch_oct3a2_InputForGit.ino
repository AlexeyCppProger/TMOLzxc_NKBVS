#include <WiFi.h>
#include <WebServer.h>

int DELAY = 100; 

const int redPin = 12;
const int bluePin = 22;
const int greenPin = 23;


String inputText = "";
String bitStream;           // поток битов для трансляции


const char* ssidAP = "ESP32_LEDZZZ";  // Имя WiFi сети
const char* passwordAP = "12345678Z"; // пароль, минимум 8 символов

WebServer server(80);

unsigned long lastBitTime = 0;
int bitIndex = 0;
bool bitStreamReady = false;

// Функция преобразования текста в битовую строку
String textToBitStream(String text) {
  String bitStream = "";
  for (int i = 0; i < text.length(); i++) {
    byte b = text[i];
    for (int bit = 7; bit >= 0; bit--) {
      bitStream += (bitRead(b, bit) ? '1' : '0');
    }
  }
  return bitStream;
}

// Обновление битового потока
void updateBitStream() {
  bitStream = textToBitStream(inputText);
  Serial.print("Новый битовый поток: ");
  Serial.println(bitStream);
}

void handleRoot() {
  String html = R"(<!doctype html><html lang=ru><head><meta charset=UTF-8><meta name=viewport content="width=device-width,initial-scale=1"><title>форма отправки сообщения</title><style>body{margin:0;height:100vh;display:flex;justify-content:center;align-items:center;font-family:'Helvetica Neue',Helvetica,Arial,sans-serif;background:linear-gradient(135deg,#74ebd5,#acb6e5);color:#333}form{background:rgba(255,255,255,.9);padding:30px 20px;border-radius:15px;box-shadow:0 8px 16px rgba(0,0,0,.2);max-width:500px;width:90%;display:flex;flex-direction:column;gap:20px}h1{margin:0;text-align:center;font-size:22px;color:#444}label{font-weight:600;display:flex;flex-direction:column;font-size:16px}.message-input{width:100%;padding:12px 16px;margin-top:8px;border:2px solid #ccc;border-radius:16px;font-size:14px;transition:border-color .2s,box-shadow .2s;box-sizing:border-box;min-height:100px;resize:vertical}.message-input:focus{border-color:#66afe9;outline:0;box-shadow:0 0 8px rgba(102,175,233,.6)}.radio-group{display:flex;flex-wrap:wrap;justify-content:space-around;padding:10px 0;background:linear-gradient(135deg,#f6d365,#fda085);border-radius:10px;box-shadow:inset 0 0 5px rgba(0,0,0,.1)}.radio-label{display:flex;align-items:center;font-size:14px;cursor:pointer;margin:5px auto 5px auto;transition:transform .2s}.radio-text{padding-top:2px}.radio-label:hover{transform:scale(1.05)}.radio-label input[type=radio]{display:none}.radio-custom{width:20px;height:20px;border:2px solid #555;border-radius:50%;margin-right:10px;position:relative;transition:all .2s}.radio-custom::after{content:"";position:absolute;top:50%;left:50%;transform:translate(-50%,-50%);width:12px;height:12px;border-radius:50%;background-color:transparent;transition:background-color .2s}input[type=radio]:checked+.radio-custom{border-color:#6a5acd}input[type=radio]:checked+.radio-custom::after{background-color:#6a5acd}button{padding:14px;font-size:16px;background:linear-gradient(135deg,#89f7fe,#66a6ff);border:none;border-radius:10px;color:#fff;cursor:pointer;box-shadow:0 4px 8px rgba(0,0,0,.2);transition:background .3s,transform .2s}button:hover{transform:scale(1.05);background:linear-gradient(135deg,#66a6ff,#89f7fe)}.radio-item{display:flex;align-items:center;margin-bottom:8px;font-size:15px;font-weight:700}@media (max-width:600px){h1{font-size:20px}label{font-size:16px}.message-input{font-size:15px;padding:14px 18px;min-height:120px}.radio-group{display:flex;flex-direction:column;align-items:center}.radio-label{margin-bottom:8px}}</style><script>
document.getElementById('myForm').addEventListener('submit', function(e) {
  e.preventDefault(); // отменить стандартную отправку

  const form = this;
  const formData = new FormData(form);

  fetch('/send', {
    method: 'POST',
    body: formData
  })
  .then(response => {
    if (response.ok) {
      form.reset();
    } else {
      alert('Ошибка при отправке формы');
    }
  })
  .catch(error => {
    console.error('Ошибка:', error);
  });
});
</script></head><body><form id=myForm action=/send method=POST><h1>Отправка формы</h1><label>Текст сообщения: <textarea name=textInput class=message-input rows=4 required></textarea></label><div><p style=margin-bottom:8px;font-size:16px;font-weight:700>Скорость передачи информации:</p><div class=radio-group><div class=radio-item><label class=radio-label><input type=radio name=choice value=5 checked=checked> <span class=radio-custom></span></label><div><span class=radio-text>5 символов в секунду</span></div></div><div class=radio-item><label class=radio-label><input type=radio name=choice value=10> <span class=radio-custom></span></label><div><span class=radio-text>10 символов в секунду</span></div></div><div class=radio-item><label class=radio-label><input type=radio name=choice value=15> <span class=radio-custom></span></label><div><span class=radio-text>15 символов в секунду</span></div></div><div class=radio-item><label class=radio-label><input type=radio name=choice value=20> <span class=radio-custom></span></label><div><span class=radio-text>20 символов в секунду</span></div></div><div class=radio-item><label class=radio-label><input type=radio name=choice value=25> <span class=radio-custom></span></label><div><span class=radio-text>25 символов в секунду</span></div></div><div class=radio-item><label class=radio-label><input type=radio name=choice value=30> <span class=radio-custom></span></label><div><span class=radio-text>30 символов в секунду</span></div></div><div class=radio-item><label class=radio-label><input type=radio name=choice value=35> <span class=radio-custom></span></label><div><span class=radio-text>35 символов в секунду</span></div></div><div class=radio-item><label class=radio-label><input type=radio name=choice value=40> <span class=radio-custom></span></label><div><span class=radio-text>40 символов в секунду</span></div></div></div></div><button type=submit>Отправить</button></form></body></html>)";
  server.send(200, "text/html", html);
}


// Обработка POST запроса
void handleSend() {
  if (server.hasArg("textInput")) {
    inputText = server.arg("textInput");
    Serial.println('текст: ' + inputText);
    updateBitStream();

    DELAY = 1000 / server.arg("choice").toInt();

    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  } else {
    server.send(400, "text/plain", "Ошибка: параметр text не найден");
  }
}

// Функция отображения бита светодиодом
void showBit(char bit) {
  if (bit == '1') {
    setColor(255, 0, 0);
    delay(DELAY);
  } else if (bit == '0') {
    setColor(0, 0, 255);
    delay(DELAY);
  } 

  setColor(255, 255, 255);
  delay(DELAY);
}

void setup() {
  Serial.begin(9600);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // Настраиваемся как точка доступа
  WiFi.softAP(ssidAP, passwordAP);
  IPAddress IP = WiFi.softAPIP();

  Serial.print("Создана точка доступа WiFi: ");
  Serial.println(ssidAP);
  Serial.print("IP адрес: ");
  Serial.println(IP);

  // Запускаем сервер
  server.on("/", HTTP_GET, handleRoot);
  server.on("/send", HTTP_POST, handleSend);
  server.begin();
  Serial.println("HTTP сервер запущен");

  // Инициализация битового потока
  updateBitStream();
}

void loop() {
  server.handleClient();

  for (int i = 0; i < bitStream.length(); i++) {
    char bit = bitStream[i];
    showBit(bit);
    Serial.print("Бит: ");
    Serial.println(bit);
  }

  Serial.print(bitStream);
  bitStream = "";

  setColor(0, 255, 0);

  delay(100);

  setColor(0, 0, 0);

  delay(100);
}

void setColor(int red, int green, int blue) {
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}