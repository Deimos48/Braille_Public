#include <AFMotor.h>

// Скорость соленоида
int write_speed = 30;  // Скорость печати
int move_speed = 100;  // Скорость перемещения

// Размеры бумаги А4 и координаты начала/конца по оси OX/OY
int page_w = 210;                  // Ширина листа
int page_h = 297;                  // Высота листа
int mar_x = 20, mar_y = 15;        // Длина рамок по OX и OY
int min_x = 0 + mar_x;             // Начало по OX
int max_x = page_w - mar_x;        // Конец по OX
int min_y = 0 + mar_y;             // Начало по OY
int max_y = page_h - mar_y;        // Конец по OY

// Пины
#define PIN_X0 10     // Пин кнопки
#define SOL_PIN 9     // Пин соленоида
#define BUT_PIN 2     // Пин кнопки

// Настройка моторов
double motor_scale_x = 10; // Коэффициент для перевода в мм по OX
double motor_scale_y = 8;  // Коэффициент для перевода в мм по OY
#define MOTOR_STEP_MODE INTERLEAVE

// Распознавание
int numbers_to_brayel[10] = {26, 1, 3, 9, 25, 17, 11, 27, 19, 10};
int letters_to_brayel[33] = {1, 3, 58, 27, 25, 17, 26, 53, 10, 47, 5, 7, 13, 29, 21, 15, 23, 14, 30, 37, 11, 19, 9, 31, 49, 45, 55, 46, 62, 42, 51, 43, 33};

// Connect a stepper motor
AF_Stepper motor1(200, 2);
AF_Stepper motor2(200, 1);

// Стартовое и нынешнее расположение соленоида
double x0 = mar_x;
double y0 = mar_y;
double pxLast = x0;
double pyLast = y0;

String Data = "";   // Вводимая строка

boolean recievedFlag = false;
boolean brailleOn = false;    // Флаги
bool is_number = false;

int brailleOrder[] = { 1, 2, 3, 7, 8, 6, 5, 4 };

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(50);

  pinMode(PIN_X0, INPUT_PULLUP);
  pinMode(SOL_PIN, OUTPUT);
  pinMode(BUT_PIN, INPUT);

  motor1.setSpeed(write_speed);
  motor2.setSpeed(write_speed);
}


void motorX(double st_mm) {                                                 // Переход на координату по оси OX
  st_mm = -st_mm;
  motor1.step(abs(st_mm)*motor_scale_x, st_mm >= 0 ? FORWARD : BACKWARD, MOTOR_STEP_MODE);
}

void motorY(double st_mm) {                                                 // Переход на координату по оси OY
  motor2.step(1.2*abs(st_mm)*motor_scale_y, st_mm >= 0 ? FORWARD : BACKWARD, MOTOR_STEP_MODE);
}

void moveXToStart() {                                                       // Переход в начало строки
  motor1.release();
  motor2.release();
  motor1.setSpeed(move_speed);      // Установить скорость для перемещения
  while (!digitalRead(PIN_X0)) {    // Ожидание нажатия на кнопку
    motorX(1);
  }
  motorX(-(page_w - mar_x * 2));    // Переход в начало
  pxLast = mar_x;
  motor1.setSpeed(write_speed);     // Установить скорость для печати
  x0 = mar_x;
}

void nextLine() {                                                            // Переход на следующую строку
  drawBrailleChar(0);
  moveXToStart();         // Переход в начало строки
  motorY(6);              // Перемещение по OY
  x0 = mar_x;
  y0 += 6;
  pyLast += 6;
}

void moveTo(double chx, double chy) {                                        // Переход по координатам (предназначено для печати точек соленоидом)
  double dx, dy,
         x = x0 + 2.5 * chx,
         y = y0 + 2.5 * chy;

  if (x != pxLast && x >= min_x && x <= max_x) {
    dx = x - pxLast;
    motorX(dx);
    pxLast = x;
  }

  if (y != pyLast && y >= min_y && y <= max_y) {
    dy = y - pyLast;
    motorY(dy);
    pyLast = y;
  }
}

void knock() {                                                // Точка
  delay(70);
  digitalWrite(SOL_PIN, HIGH);
  delay(300);
  digitalWrite(SOL_PIN, LOW);
  delay(50);
}

void drawBrailleChar(int bch) {                              // Печать символа
  int id;
  for (int i = 0; i < 8; i++) {
    id = brailleOrder[i];
    if ((bch & (1 << (id - 1))) != 0) {
      moveTo(id < 7 ? (id - 1) / 3 : id - 7, id < 7 ? ((id - 1) % 3) : 3);
      if (digitalRead(BUT_PIN) == 1) {
      } else {
        knock();
      }
    }
  }
}


int convertToBraille(char ascii) {                          // Перевод в шрифт Брайля
  int d = int(ascii);
  Serial.println(d);
  if (d == -72) {
    is_number = false;
    return int(letters_to_brayel[32]);
  } else if (d < 0) {
    is_number = false;
    return int(letters_to_brayel[d + 32]);
  } else if (d > 47) {
    if (!is_number) {
      is_number = true;
      drawBrailleChar(60);
    }
    return int(numbers_to_brayel[d - 48]);
  } else if (d == 32) {
    return 0;
  }
}


void loop() {
  if (digitalRead(BUT_PIN) == 1) {
    brailleOn = false;                     // При нажатии на кнопку работа прерывается
  }
  if (Serial.available() > 0) {
    Data += (char)Serial.read();
    recievedFlag = true;
    delay(2);
  }
  else if (recievedFlag) {
    Serial.println(Data);                 // Вывод символа
    for (int i = 0; i < Data.length(); i++) {
      if (digitalRead(PIN_X0) == 1){
        nextLine();
      }
      char data = Data[i];
      Serial.println(data); 
      if (data == '\n' || data == '\r') {  // Символы, которые будут вводиться всегда, но они нам не нужны
      } else if (data == '{') {            // Начало работы принтера
        if (digitalRead(BUT_PIN) == 0) {
          nextLine();
          brailleOn = true;
        }
      } else if (data == '}') {            // Конец работы принтера
        brailleOn = false;
        is_number = false;
        motor1.release();
        motor2.release();
      } else if (data == '#') {            // Переход на следующую строку
        is_number = false;
        if (digitalRead(BUT_PIN) == 0) {
          nextLine();
        }
      } else {
        if (digitalRead(BUT_PIN) == 1) {
          brailleOn = false;
        }
        if (brailleOn) {                         // Печать символа
          if (data < '~') {
            data = convertToBraille(data);
            drawBrailleChar(data);
          } else {
            drawBrailleChar((data - 0x2800));
          }
          x0 += 6.5;
        }
      }
    }
    Data = "";                          // очистить
    recievedFlag = false;                  // опустить флаг
  }
}