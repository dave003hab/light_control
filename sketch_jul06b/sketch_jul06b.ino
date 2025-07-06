const byte ppmInPin = 2;          // Входной пин для PPM сигнала
const byte ppmOutPin = 9;         // Выходной пин (будет модулировать биты несущей частотой)
const unsigned long START_GAP = 1400;        // Порог паузы для начала нового пакета
const byte MAX_PULSES = 5;                   // Кл-во импульсов в одном пакете
const unsigned long SILENCE_DURATION = 9800; // Длительность "тишины" после пакета

// настройки пинов
void setup() 
{
  pinMode(ppmInPin, INPUT);            // Настраиваем пин входа
  pinMode(ppmOutPin, OUTPUT);          // Настраиваем пин выхода
  digitalWrite(ppmOutPin, LOW);        // Начальное состояние — молчание
}

void loop() 
{
  static unsigned long lastTime = 0;         // Время последнего фронта вниз
  static byte pulseCount = 0;                // Счётчик импульсов в пакете
  static bool inPacket = false;              // Флаг состояния "внутри пакета"
  static bool inSilence = false;             // Флаг "режим тишины"
  static unsigned long silenceStart = 0;     // Время начала тишины

  static int lastState = HIGH;               // Предыдущее состояние входа
  int state = digitalRead(ppmInPin);         // Текущее состояние входа

  // --- Обработка тишины ---
  if (inSilence) {
    if (micros() - silenceStart >= SILENCE_DURATION) {
      inSilence = false;                     // Выход из режима тишины
      pulseCount = 0;                        // Сброс счётчика импульсов
    } 
    else 
    {
      noTone(ppmOutPin);                     // Отключить неущую часоту
      digitalWrite(ppmOutPin, LOW);          // Удерживать выход в LOW
      return;
    }
  }

  // --- Обнаружение фронта вниз (конец импульса) ---
  if (lastState == HIGH && state == LOW) 
  {
    unsigned long now = micros();
    unsigned long gap = now - lastTime;
    lastTime = now;

    if (gap > START_GAP) // Услове проверки следующего пакеа 
    {
      // Новый пакет
      pulseCount = 0;
      inPacket = true;
    } 
    else if (inPacket) // находимся в пакете
    {
      pulseCount++; // если находимся в пакете, то добавляем в счетчик 1 при каждой новой итерации, пока мы в екущем пакете
      if (pulseCount >= MAX_PULSES) // отслеживаю кол-во импульсов. 
      {
        // Завершение пакета. Обнуление счетчика
        inPacket = false; // если кол-во битов больше MAX_PULSES, то выходим из пакета 
        inSilence = true; // вход в молчание
        silenceStart = micros(); //обнуляем таймер
        noTone(ppmOutPin); // ничего не посылаем
        digitalWrite(ppmOutPin, HIGH);  // (работаем в режме инверсной логикии)
        return;
      }
    }
  }

  // --- Управление выходом во время пакета ---
  if (inPacket && pulseCount < MAX_PULSES) 
  {
    if (state == HIGH) 
    {
      noTone(ppmOutPin);               // Пауза — выключаем модуляцию
      digitalWrite(ppmOutPin, LOW);
    } 
    else 
    {
      tone(ppmOutPin, 38000);          // Импульс — включаем несущую часоу
    }
  } 
  else 
  {
    noTone(ppmOutPin);                 // Вне пакета — тишина
    digitalWrite(ppmOutPin, LOW);
  }

  lastState = state;                   // Обновляем состояние
}

