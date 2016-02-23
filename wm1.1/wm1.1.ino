//23.02.2016

#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <TimerOne.h>
#include <MenuSystem.h>

#define FULLTURN1 360/1.8*16 //full turn per steps drive1
#define FULLTURN2 360/1.8*32 //full turn per steps drive2
#define STEP_PER_MM FULLTURN2 / 0.8 //шагов на мм для шпильки M5

#define d1  1 //для указания направления
#define d2  -1

const int sd1d = 9;              // pin for dir steps drive 1
const int sd1 = 10;              // pin for step steps drive 1
const int sd2d = 11 ;            // pin for dir steps drive 2
const int sd2 = 12;              // pin for step steps drive 2


LiquidCrystal lcd(8, 7, 6, 5, 4, 3); //init LCD display, pins 1,2,3,4,5

#define enable 13 //15
#define load 0   //1
#define clock 1  //2
#define data 2   //9

float WIRED = 0.3; //default wire diameter, mm
float WINDOW = 30; //default window, mm
int COILS = 200;   //default coils
int spd = 200;     //default step motor delay
int steps1 = 0;    //счетчик шагов
int steps2 = 0;    //счетчик шагов для второго шд
int count_coils = 0; // счетчик витков
int count_coilspw = 0; //счетчик на заполнение окна
int STEP_PER_COIL = 0;

int dir = 1;
int flag_i = 0; //для мотора на прерываниях то 1 выдаем то 0
int motor_on = 0; //вкл./выкл. мотор намотки
int key = 0;
char k;

//menu
MenuSystem ms;
Menu mm("------Menu------");
MenuItem mm_mi1("Calibration");
MenuItem mm_mi2("Diagnostic");
MenuItem mm_mi3("Wired");
MenuItem mm_mi4("Window");
MenuItem mm_mi5("Coils");
MenuItem mm_mi6("Direction");
MenuItem mm_mi7("Run");



void setup()
{
  pinMode(enable, OUTPUT);
  pinMode(load, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(data, INPUT);
  pinMode(sd1, OUTPUT);
  pinMode(sd2, OUTPUT);
  pinMode(sd1d, OUTPUT);
  pinMode(sd2d, OUTPUT);
  digitalWrite(load, HIGH);
  digitalWrite(enable, HIGH);
  pinMode(A5, INPUT);
  lcd.begin(16, 2);

  //возвращаем параметры из eeprom
  float WIRED_EEPROM = 0;
  WIRED_EEPROM = EEPROM_float_read(0);
  if ((WIRED_EEPROM >= 0.2) && (WIRED_EEPROM <= 0.6))
  {
    WIRED = WIRED_EEPROM;
  }

  float WINDOW_EEPROM = 0;
  WINDOW_EEPROM = EEPROM_float_read(4);
  if ((WINDOW_EEPROM >= 0.5) && (WINDOW_EEPROM <= 150))
  {
    WINDOW =  WINDOW_EEPROM;
  }

  int COILS_EEPROM = 0;
  COILS_EEPROM = EEPROM_int_read(8);
  if ((COILS_EEPROM >= 10) && (COILS_EEPROM <= 5000))
  {
    COILS = COILS_EEPROM;
  }

  int DIR_EEPROM = 0;
  COILS_EEPROM = EEPROM_int_read(12);
  if ((DIR_EEPROM == 1) || (DIR_EEPROM == -1))
  {
    dir = DIR_EEPROM;
  }

  mm.add_item(&mm_mi1, &Calibration);
  mm.add_item(&mm_mi2, &Diagnostic);
  mm.add_item(&mm_mi3, &Wired);
  mm.add_item(&mm_mi4, &Window);
  mm.add_item(&mm_mi5, &Coils);
  mm.add_item(&mm_mi6, &Direction);
  mm.add_item(&mm_mi7, &Runing);
  //  mm.add_item(&mm_mi6, &day_setup_selected);
  ms.set_root_menu(&mm);

  //инициалищируем таймер
  Timer1.initialize(100000); // set a timer of length 100000 microseconds (or 0.1 sec - or 10Hz => the led will blink 5 times, 5 cycles of on-a
  Timer1.attachInterrupt( timerIsr ); // attach the service routine here
  hello();
}  // setup()...



void loop()
{
  menu();
} //loop()...


//обработчик прерывания
void timerIsr()
{
  k = keyboard();
  if (k == 'u')
  {
    if (spd > 20)spd = spd - 10;
  }
  if (k == 'd')
  {
    if (spd < 1000)spd = spd + 10;
  }


}

void displayMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  // Display the menu
  Menu const* cp_menu = ms.get_current_menu();

  //lcd.print("Current menu name: ");
  lcd.print(cp_menu->get_name());
  lcd.setCursor(0, 1);
  lcd.print(cp_menu->get_selected()->get_name());
  delay(500);

}

/*
char keyboard() {

  digitalWrite(load, LOW);
  //delayMicroseconds(5);
  digitalWrite(load, HIGH);

  digitalWrite(clock, HIGH);
  digitalWrite(enable, LOW);
  byte incoming = shiftIn(data, clock, MSBFIRST);
  digitalWrite(enable, HIGH);

  if (bitRead(incoming, 0) == 1) {
    return 'u';
  } else if (bitRead(incoming, 1) == 1) {
    return 'd';
  } else if (bitRead(incoming, 2) == 1) {
    return 'c';
  } else if (bitRead(incoming, 3) == 1) {
    return 'l';//
  } else if (bitRead(incoming, 4) == 1) {
    //return 'e';
  } else if (bitRead(incoming, 5) == 1) {
    return 'e';
  } else if (bitRead(incoming, 6) == 1) {
    return 'r';//
  } else if (bitRead(incoming, 7) == 1) {
    //return 'S';//
  } else {
    return 0;
  }
}
*/
char keyboard() {
  key = analogRead (5);

  //debug
  //lcd.setCursor(10, 1);
  //lcd.print(key);

  //Esc/Cancel
  if ((key > 350) && (key < 360)) {
    return 'c';
  }
  //up
  if ((key > 210) && (key < 230)) {
    return 'u';
  }
  //right
  if ((key > 560) && (key < 590)) {
    return 'r';
  }
  //down
  if ((key > 510) && (key < 540)) {
    return 'd';
  }
  //left
  if ((key > 440) && (key < 470)) {
    return 'l';
  }
  //Enter
  if (key < 20) {
    return 'e';
  }
  return 0;
}

void hello() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Winding machine"));
}

void menu() {

  //k = keyboard();
  switch (k) {
    case 'c':
      ms.back();
      hello();
      break;

    case 'u':
      ms.prev();
      displayMenu();
      break;
    case 'd':
      ms.next();
      displayMenu();
      break;
    case 'e':
      ms.select();
      displayMenu();
      break;
  }
}


void Calibration(MenuItem* p_menu_item) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Calibration mode"));

  delay(300);
  while (k != 'c') {

    if (k == 'd') {
      spd = 1500;
      step(sd1, -16);
    }
    if (k == 'u') {
      spd = 1500;
      step(sd1, 16);
    }
    if (k == 'r') {
      spd = 80;
      step(sd2, 16);
    }
    if (k == 'l') {
      spd = 80;
      step(sd2, -16);
    }

  }
}

void Diagnostic(MenuItem* p_menu_item) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Diagnostic mode"));

  delay(300);
  while (k != 'c') {

    if (k == 'd') {
      spd = 200;
      step(sd1, FULLTURN1 * -1);
    }
    if (k == 'u') {
      spd = 200;
      step(sd1, FULLTURN1);
    }
    if (k == 'r') {
      spd = 50;
      int i = 0;
      while (i != 43) {
        step(sd2, STEP_PER_MM);
        i++;
      }
    }

    if (k == 'l') {
      spd = 50;
      step(sd2, FULLTURN2 * -1);
    }

  }
}

void Wired(MenuItem* p_menu_item) {
  while (k != 'c') {
    lcd.setCursor(0, 1);
    lcd.print("Wired:");
    lcd.print(WIRED, 2);
    lcd.print(" < ");
    delay(200);
    if (k == 'l') {
      if (WIRED > 0.2) {
        WIRED = WIRED - 0.01;
        delay(100);
      }
    }
    if (k == 'r') {
      if (WIRED < 0.6) {
        WIRED = WIRED + 0.01;
        delay(100);
      }
    }
    if (k == 'e') {
      EEPROM_float_write(0, WIRED);
      lcd.print(" OK");
      delay(100);
    }
  }

}

void Window(MenuItem* p_menu_item) {
  while (k != 'c') {
    lcd.setCursor(0, 1);
    lcd.print("Window:");
    lcd.print(WINDOW, 1);
    lcd.print(" < ");
    delay(200);
    if (k == 'l') {
      if (WINDOW > 0.5) {
        WINDOW = WINDOW - 0.5;
        delay(100);
      }
    }
    if (k == 'r') {
      if (WINDOW < 150) {
        WINDOW = WINDOW + 0.5;
        delay(100);
      }
    }
    if (k == 'e') {
      EEPROM_float_write(4, WINDOW);
      lcd.print("OK");
      delay(100);
    }

  }

}
void Coils(MenuItem* p_menu_item) {
  while (k != 'c') {
    lcd.setCursor(0, 1);
    lcd.print("Coils:");
    lcd.print(COILS);
    lcd.print(" < ");
    delay(200);
    if (k == 'l') {
      if (COILS > 10) {
        COILS = COILS - 1;
        delay(60);
      }
    }
    if (k == 'r') {
      if (COILS < 5000) {
        COILS = COILS + 1;
        delay(60);
      }
    }
    if (k == 'u') {
      if (COILS < 4900) {
        COILS = COILS + 100;
        delay(60);
      }
    }
    if (k == 'd') {
      if (COILS > 100) {
        COILS = COILS - 100;
        delay(60);
      }
    }

    if (k == 'e') {
      EEPROM_int_write(8, COILS);
      lcd.print("OK");
      delay(100);
    }
  }
}

void Runing(MenuItem* p_menu_item) {
  count_coils = 0;
  int COILSPW = WINDOW / WIRED; //сколько витков помещается в окошке
  STEP_PER_COIL = STEP_PER_MM * WIRED; //расчитываем кол-во шагов на виток

  lcd.setCursor(0, 0);
  lcd.print("Wir:");
  lcd.print(WIRED);
  lcd.print(" W:");
  lcd.print(WINDOW, 1);

  //lcd.setCursor(14,1);
  //lcd.print(COILSPW);
  float Tpercent = 0;
  int stepsNado = 0;
  int s = 0;
  byte stat = 0;
  spd = 600;
  digitalWrite(sd1d, LOW); //вот выбрали направление
  motor_on = 1;            //и идем

  long ttt = 0;

  lcd.setCursor(0, 1);
  lcd.print("C:");

  while (count_coils <= COILS) {
    delayMicroseconds(spd);
    if (motor_on == 1) { //что-нибудь делаем, если только motor_on
      if (flag_i == 0) {
        digitalWrite(sd1, HIGH);
        flag_i = 1;
      } else {
        digitalWrite(sd1, LOW);
        flag_i = 0;
        steps1++; //счетчик шагов
      }

      Tpercent = float(steps1) / float(FULLTURN1);
      stepsNado = Tpercent * STEP_PER_COIL;
      s = stepsNado - steps2;

      steps2 = stepsNado;
      step(sd2, dir * s);

      if (steps1 == FULLTURN1) {
        count_coils++;
        count_coilspw++;
        steps1 = 0;
        steps2 = 0;
        lcd.setCursor(2, 1);
        lcd.print(count_coils);
        lcd.print("/");
        lcd.print(COILS);
        lcd.print(" ");
        lcd.print(COILSPW);

        if (dir > 0) {
          lcd.print(" >");
        } else {
          lcd.print(" <");
        }

        if (count_coilspw == COILSPW) {
          if (dir == d1) {
            dir = d2;
          } else {
            dir = d1; //меняем направление
          }
          count_coilspw = 0;
          spd = 600;
          pause(); //пауза
        }


      }// else {
        //delayMicroseconds(100);

      //}
    }

    if (k == 'e')
    {
      if (pause()) {
        return;
      }
    }
  }
  motor_on = 0;            //стоп машина

}

byte pause() {
  motor_on = 0;
  delay(300);
  int cnt = 0;
  int flag = 0;

  while (k != 'e') {

    lcd.setCursor(15, 1);
    if (flag) {
      lcd.print(" ");
    } else {
      lcd.print("P");
    }

    if (cnt < 100) {
      cnt++;
    } else {
      cnt = 0;
      if (flag) {
        flag = 0;
      } else {
        flag = 1;
      }
    }

    delay(10);
    if (k == 'c') {
      return 1;
    }
  }
  motor_on = 1;
  lcd.setCursor(15, 1);
  lcd.print(" ");
  delay(200);
  return 0;
}

void Direction(MenuItem * p_menu_item) {

  while (k != 'c') {
    lcd.setCursor(0, 1);
    lcd.print("Dir.:");
    if (dir > 0) {
      lcd.print(" >");
    } else {
      lcd.print(" <");
    }
    lcd.print(" < ");
    delay(200);
    if (k == 'l') {
      dir = 1;
      delay(200);
    }
    if (k == 'r') {
      dir = -1;
      delay(200);
    }
    if (k == 'e') {
      EEPROM_int_write(12, dir);
      lcd.print("OK");
      delay(100);
    }
  }

}

void step(int drive, int steps) {
  int sdir;
  int dirval;

  if (steps < 0 ) {
    steps = abs(steps);
    dirval = HIGH;
  } else {
    dirval = LOW;
  }

  if (drive == sd1) {
    sdir = sd1d;
  }
  if (drive == sd2) {
    sdir = sd2d;
  }

  digitalWrite(sdir, dirval);

  while (steps != 0 ) {
    delayMicroseconds(spd);
    digitalWrite(drive, HIGH);
    delayMicroseconds(spd);
    digitalWrite(drive, LOW);
    steps--;
  }

}


void EEPROM_float_write(int addr, float val) // запись в ЕЕПРОМ
{
  byte *x = (byte *)&val;
  for (byte i = 0; i < 4; i++) EEPROM.write(i + addr, x[i]);
}

float EEPROM_float_read(int addr) // чтение из ЕЕПРОМ
{
  byte x[4];
  for (byte i = 0; i < 4; i++) x[i] = EEPROM.read(i + addr);
  float *y = (float *)&x;
  return y[0];
}

void EEPROM_int_write(int p_address, int p_value)
{
  byte lowByte = ((p_value >> 0) & 0xFF);
  byte highByte = ((p_value >> 8) & 0xFF);

  EEPROM.write(p_address, lowByte);
  EEPROM.write(p_address + 1, highByte);
}

unsigned int EEPROM_int_read(int p_address)
{
  byte lowByte = EEPROM.read(p_address);
  byte highByte = EEPROM.read(p_address + 1);
  return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
}



