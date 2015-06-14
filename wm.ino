#include <EEPROM.h>
#include <MenuBackend.h>    //MenuBackend library - copyright by Alexander Brevig
#include <LiquidCrystal.h>  //this library is included in the Arduino IDE
#include <TimerOne.h>

#define FULLTURN1 360/1.8*16 //full turn per steps drive1
#define FULLTURN2 360/3.6*16 //full turn per steps drive2

#define STEP_PER_MM FULLTURN2 / 0.8 //шагов на мм для шпильки M5

#define HELLO_TXT "Winding machine"
#define CLS "                "
const int buttonPinLeft = 0;    // pin for the Up button
const int buttonPinRight = 1;   // pin for the Down button
const int buttonPinEsc = 2;     // pin for the Esc button                                                                                     
const int buttonPinEnter = 13;   // pin for the Enter button                                                                                  
                                                                                                                                              
const int sd1 = 9;               // pin for step steps drive 1                                                                                
const int sd1d = 10;              // pin for dir steps drive 1                                                                                
const int sd2 = 11;               // pin for step steps drive 2
const int sd2d = 12;              // pin for dir steps drive 2

int lastButtonPushed = 0;

int lastButtonEnterState = LOW;   // the previous reading from the Enter input pin
int lastButtonEscState = LOW;   // the previous reading from the Esc input pin
int lastButtonLeftState = LOW;   // the previous reading from the Left input pin
int lastButtonRightState = LOW;   // the previous reading from the Right input pin

long lastEnterDebounceTime = 0;  // the last time the output pin was toggled
long lastEscDebounceTime = 0;  // the last time the output pin was toggled
long lastLeftDebounceTime = 0;  // the last time the output pin was toggled
long lastRightDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 100;    // the debounce time

LiquidCrystal lcd(8, 7, 6, 5, 4, 3); //init LCD display, pins 1,2,3,4,5

//Menu variables
MenuBackend menu = MenuBackend(menuUsed,menuChanged);
//initialize menuitems
    MenuItem menu1Item1 = MenuItem("Calibration");
    MenuItem menu1Item2 = MenuItem("Setup");
      MenuItem menuItem2SubItem1 = MenuItem("Wired");
      MenuItem menuItem2SubItem2 = MenuItem("Window");
      MenuItem menuItem2SubItem3 = MenuItem("Coils");
      MenuItem menuItem2SubItem4 = MenuItem("Direction");
    MenuItem menu1Item3 = MenuItem("Run");

float WIRED = 0.5; //default wire diameter, mm
float WINDOW = 20; //default window, mm
int COILS = 200;   //default coils
int spd = 100;     //default step motor delay
int steps1 = 0;    //счетчик шагов
int count_coils = 0; // счетчик витков

int dir = 1;
int flag_i = 0; //для мотора на прерываниях то 1 выдаем то 0
int motor_on = 0; //вкл./выкл. мотор на прерываниях

void setup()
{
  pinMode(buttonPinLeft, INPUT);
  pinMode(buttonPinRight, INPUT);
  pinMode(buttonPinEnter, INPUT);
  pinMode(buttonPinEsc, INPUT);

  pinMode(sd1, OUTPUT);
  pinMode(sd2, OUTPUT);
  pinMode(sd1d, OUTPUT);
  pinMode(sd2d, OUTPUT);
  
  lcd.begin(16, 2);

  //configure menu
  menu.getRoot().add(menu1Item1);
  menu1Item1.addRight(menu1Item2).addRight(menu1Item3);
  menu1Item2.add(menuItem2SubItem1).addRight(menuItem2SubItem2).addRight(menuItem2SubItem3).addRight(menuItem2SubItem4);
  //menu.toRoot();
  cls();
  lcd.setCursor(0,0);
  lcd.print(HELLO_TXT);


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
  
  
  //инициалищируем таймер
  Timer1.initialize(100); // set a timer of length 100000 microseconds (or 0.1 sec - or 10Hz => the led will blink 5 times, 5 cycles of on-a
  Timer1.attachInterrupt( timerIsr ); // attach the service routine here

}  // setup()...


void loop()
{
 
  readButtons();  //I splitted button reading and navigation in two procedures because 
  navigateMenus();  //in some situations I want to use the button for other purpose (eg. to change some settings)
                  
} //loop()... 



void menuChanged(MenuChangeEvent changed){
  //cls();
  MenuItem newMenuItem=changed.to; //get the destination menu
  
  lcd.setCursor(0,1); //set the start position for lcd printing to the second row
  
  if(newMenuItem.getName()==menu.getRoot()){
      lcd.print("Menu            ");
  }else if(newMenuItem.getName()=="Calibration"){
      lcd.print("Calibration     ");
    }else if(newMenuItem.getName()=="Setup"){
      lcd.print("Setup           ");
  }else if(newMenuItem.getName()=="Wired"){
      cls();
      lcd.print("Wired:");
      lcd.print(WIRED);
  }else if(newMenuItem.getName()=="Window"){
      cls();
      lcd.print("Window:");
      lcd.print(WINDOW);
  }else if(newMenuItem.getName()=="Coils"){
      cls();
      lcd.print("Coils:");
      lcd.print(COILS);
  }else if(newMenuItem.getName()=="Direction"){
      cls();
      lcd.print("Direction:");
      if (dir > 0){
         lcd.print(" >");
       }else{
         lcd.print(" <");
         }
  }else if(newMenuItem.getName()=="Run"){
      lcd.print("Run             ");
  }
}

void cls(){
lcd.setCursor(0,0);
lcd.print(CLS);
lcd.setCursor(0,1);
lcd.print(CLS);
lcd.setCursor(0,0);
}

void menuUsed(MenuUseEvent used){
  
  
  if (used.item.getName() == "Calibration"){   
    
   cls();
   int motor = sd2; 
   lcd.setCursor(0,0);  
   lcd.print("Calibration mode");
   spd = 100;
   lcd.setCursor(0,1);
   lcd.print("Motor2");
   delay(300);   
     while(digitalRead(buttonPinEsc)==LOW){
       if (digitalRead(buttonPinLeft)==HIGH){
         step(motor, -16);
       }
       if (digitalRead(buttonPinRight)==HIGH){
         step(motor, 16);
       }
       if (digitalRead(buttonPinEnter)==HIGH){
        //int i = 50;
        //while (i != 0){
        // i--;
         //step(sd2, STEP_PER_MM);
        // }
         lcd.setCursor(0,1);
         if (motor == sd2){
             motor = sd1;
             lcd.print("Motor1");
         }else{
             motor = sd2;
             lcd.print("Motor2");
         }
         delay(300);
       }
     }
  }
  
  
//установка диаметра провода  
if (used.item.getName() == "Wired"){

  while(digitalRead(buttonPinEsc)==LOW){
   
     lcd.setCursor(0,1);  
     lcd.print("Wired:");
     lcd.print(WIRED);
     lcd.print(" <- ");
     
       if (digitalRead(buttonPinLeft)==HIGH){
         if (WIRED > 0.2){
         WIRED = WIRED - 0.05;
         delay(200);
         }
       }
       if (digitalRead(buttonPinRight)==HIGH){
         if (WIRED < 0.6){
         WIRED = WIRED + 0.05;
         delay(200);
         }
       }
       if (digitalRead(buttonPinEnter)==HIGH){
         EEPROM_float_write(0,WIRED);
         delay(100);
         lcd.print(" Save");
       }
     }
}

//установка размера окна катушки
if (used.item.getName() == "Window"){

  while(digitalRead(buttonPinEsc)==LOW){
   
     lcd.setCursor(0,1);  
     lcd.print("Window:");
     lcd.print(WINDOW);
     lcd.print(" <-");
     delay(200);
       if (digitalRead(buttonPinLeft)==HIGH){
         if (WINDOW > 0.5){
         WINDOW = WINDOW - 0.5;
         delay(150);
         }
       }
       if (digitalRead(buttonPinRight)==HIGH){
         if (WINDOW < 150){
         WINDOW = WINDOW + 0.5;
         delay(150);
         }
       }
       if (digitalRead(buttonPinEnter)==HIGH){
         EEPROM_float_write(4,WINDOW);
         delay(100);
         lcd.print(" Save");
       }
     }
}  
//установка кол-ва витков
if (used.item.getName() == "Coils"){

  while(digitalRead(buttonPinEsc)==LOW){
   
     lcd.setCursor(0,1);  
     lcd.print("Coils:");
     lcd.print(COILS);
     lcd.print(" <-");
     delay(200);
       if (digitalRead(buttonPinLeft)==HIGH){
         if (COILS > 10){
         COILS = COILS - 1;
         delay(60);
         }
       }
       if (digitalRead(buttonPinRight)==HIGH){
         if (COILS < 5000){
         COILS = COILS + 1;
         delay(60);
         }
       }
       if (digitalRead(buttonPinEnter)==HIGH){
         EEPROM_int_write(8,COILS);
         delay(100);
         lcd.print(" Save");
       }
     }
}  



//установка направления намотки
if (used.item.getName() == "Direction"){

  while(digitalRead(buttonPinEsc)==LOW){
   
     lcd.setCursor(0,1);  
     lcd.print("Direction:");
     
     if (dir > 0){
         lcd.print(" >");
       }else{
         lcd.print(" <");
         }
     
     lcd.print(" <-");
     delay(200);
       if (digitalRead(buttonPinLeft)==HIGH){
         
         dir = 1;
         delay(300);
         
       }
       if (digitalRead(buttonPinRight)==HIGH){
         
         dir = -1;
         delay(300);
         
       }
       if (digitalRead(buttonPinEnter)==HIGH){
         EEPROM_int_write(12,dir);
         delay(100);
         lcd.print(" Save");
       }
     }
}  




//запуск процесса намотки
if (used.item.getName() == "Run"){
     
     count_coils = 0;
     int count_coilspw = 0; //счетчик на заполнение окна
     int d1 = 1; //переменные для указания направления
     int d2 = -1;

     int COILSPW = WINDOW/WIRED; //сколько витков помещается в окошке
     float STEP_PER_COIL = STEP_PER_MM * WIRED; //расчитываем кол-во шагов на виток
     
     lcd.setCursor(0,0);  
     lcd.print("Wir:");
     lcd.print(WIRED);
     lcd.print(" Wind:");
     lcd.print(WINDOW);
     //lcd.setCursor(14,1);
     //lcd.print(COILSPW);
     

     spd = 520;
     digitalWrite(sd1d,LOW);  //вот выбрали направление
     motor_on = 1;            //и идем     
     
     while(count_coils <= COILS){
       
       lcd.setCursor(0,1);  
       lcd.print("C:");
       lcd.print(count_coils);
       lcd.print("/");
       lcd.print(COILS);
       
       lcd.print(" ");
//       lcd.print(count_coilspw);
//       lcd.print("/");
       lcd.print(COILSPW);
       
       if (dir > 0){
         lcd.print(" >");
       }else{
         lcd.print(" <");
         }
              
       if (count_coilspw == COILSPW){
        if (dir==d1){
          dir=d2;
        }else{
          dir=d1;//меняем направление
        } 
        count_coilspw = 0;
      }

          if (steps1 <= 50 && steps1 >= 10){
           count_coilspw++;
           step(sd2, dir * STEP_PER_COIL);
           //delay(400);
          }

       
        //pause key-------------------------------
        if (digitalRead(buttonPinEnter)==HIGH)
        {
         motor_on = 0;
         delay(300);
         
         while(digitalRead(buttonPinEnter)==LOW)
         { 
           delay(10);
           if(digitalRead(buttonPinEsc)==HIGH){
            return;
           }
         } 
           motor_on = 1;
           delay(200);
        }
        //end pause key -----------------------
       
          //delayMicroseconds(spd);
     }
     motor_on = 0;            //стоп машина
}  
  
  delay(1000);  //delay to allow message reading
  cls();
  lcd.setCursor(0,0);  
  lcd.print(HELLO_TXT);
  //menu.toRoot();  //back to Main
}

//обработчик прерывания, тут крутим первый мотор
void timerIsr()
{
  if (motor_on == 1){ //что-нибудь делаем, если только motor_on
    if (steps1 == 0){
        count_coils++;
        steps1=FULLTURN1;
    }

      if (flag_i == 0){
        digitalWrite(sd1,HIGH);    
        flag_i = 1;
        }else{
          digitalWrite(sd1,LOW);    
          flag_i = 0;
          steps1--; //счетчик шагов
        }
         
  }
}


void  readButtons(){  //read buttons status
  int reading;
  int buttonEnterState=LOW;             // the current reading from the Enter input pin
  int buttonEscState=LOW;             // the current reading from the input pin
  int buttonLeftState=LOW;             // the current reading from the input pin
  int buttonRightState=LOW;             // the current reading from the input pin

  //Enter button
                  // read the state of the switch into a local variable:
                  reading = digitalRead(buttonPinEnter);

                  // check to see if you just pressed the enter button 
                  // (i.e. the input went from LOW to HIGH),  and you've waited 
                  // long enough since the last press to ignore any noise:  
                
                  // If the switch changed, due to noise or pressing:
                  if (reading != lastButtonEnterState) {
                    // reset the debouncing timer
                    lastEnterDebounceTime = millis();
                  } 
                  
                  if ((millis() - lastEnterDebounceTime) > debounceDelay) {
                    // whatever the reading is at, it's been there for longer
                    // than the debounce delay, so take it as the actual current state:
                    buttonEnterState=reading;
                    lastEnterDebounceTime=millis();
                  }
                  
                  // save the reading.  Next time through the loop,
                  // it'll be the lastButtonState:
                  lastButtonEnterState = reading;
                  

    //Esc button               
                  // read the state of the switch into a local variable:
                  reading = digitalRead(buttonPinEsc);

                  // check to see if you just pressed the Down button 
                  // (i.e. the input went from LOW to HIGH),  and you've waited 
                  // long enough since the last press to ignore any noise:  
                
                  // If the switch changed, due to noise or pressing:
                  if (reading != lastButtonEscState) {
                    // reset the debouncing timer
                    lastEscDebounceTime = millis();
                  } 
                  
                  if ((millis() - lastEscDebounceTime) > debounceDelay) {
                    // whatever the reading is at, it's been there for longer
                    // than the debounce delay, so take it as the actual current state:
                    buttonEscState = reading;
                    lastEscDebounceTime=millis();
                  }
                  
                  // save the reading.  Next time through the loop,
                  // it'll be the lastButtonState:
                  lastButtonEscState = reading; 
                  
                     
   //Down button   
                
                  // read the state of the switch into a local variable:
                  reading = digitalRead(buttonPinRight);

                  // check to see if you just pressed the Down button 
                  // (i.e. the input went from LOW to HIGH),  and you've waited 
                  // long enough since the last press to ignore any noise:  
                
                  // If the switch changed, due to noise or pressing:
                  if (reading != lastButtonRightState) {
                    // reset the debouncing timer
                    lastRightDebounceTime = millis();
                  } 
                  
                  if ((millis() - lastRightDebounceTime) > debounceDelay) {
                    // whatever the reading is at, it's been there for longer
                    // than the debounce delay, so take it as the actual current state:
                    buttonRightState = reading;
                   lastRightDebounceTime =millis();
                  }
                  
                  // save the reading.  Next time through the loop,
                  // it'll be the lastButtonState:
                  lastButtonRightState = reading;                  
                  
                  
    //Up button               
                  // read the state of the switch into a local variable:
                  reading = digitalRead(buttonPinLeft);

                  // check to see if you just pressed the Down button 
                  // (i.e. the input went from LOW to HIGH),  and you've waited 
                  // long enough since the last press to ignore any noise:  
                
                  // If the switch changed, due to noise or pressing:
                  if (reading != lastButtonLeftState) {
                    // reset the debouncing timer
                    lastLeftDebounceTime = millis();
                  } 
                  
                  if ((millis() - lastLeftDebounceTime) > debounceDelay) {
                    // whatever the reading is at, it's been there for longer
                    // than the debounce delay, so take it as the actual current state:
                    buttonLeftState = reading;
                    lastLeftDebounceTime=millis();;
                  }
                  
                  // save the reading.  Next time through the loop,
                  // it'll be the lastButtonState:
                  lastButtonLeftState = reading;  

                  //records which button has been pressed
                  if (buttonEnterState==HIGH){
                    lastButtonPushed=buttonPinEnter;

                  }else if(buttonEscState==HIGH){
                    lastButtonPushed=buttonPinEsc;

                  }else if(buttonRightState==HIGH){
                    lastButtonPushed=buttonPinRight;

                  }else if(buttonLeftState==HIGH){
                    lastButtonPushed=buttonPinLeft;

                  }else{
                    lastButtonPushed=0;
                  }                  
}

void navigateMenus() {
  MenuItem currentMenu=menu.getCurrent();
  
  switch (lastButtonPushed){
    case buttonPinEnter:
      if(!(currentMenu.moveDown())){  //if the current menu has a child and has been pressed enter then menu navigate to item below
        menu.use();
      }else{  //otherwise, if menu has no child and has been pressed enter the current menu is used
        menu.moveDown();
       } 
      break;
    case buttonPinEsc:
      //menu.toRoot();  //back to main
      break;
    case buttonPinRight:
      menu.moveRight();
      break;      
    case buttonPinLeft:
      menu.moveLeft();
      break;      
  }
  
  lastButtonPushed=0; //reset the lastButtonPushed variable
}

//-------------------------------------------------------------------
void step(int drive, int steps){
  int sdir;
  int dirval;
    
  if (steps < 0 ){
  steps = abs(steps);
  dirval = HIGH;
  }else{
    dirval = LOW;
  }

  if (drive == sd1){sdir = sd1d;}
  if (drive == sd2){sdir = sd2d;}
  
  digitalWrite(sdir,dirval);
    
  while (steps !=0 ){
    delayMicroseconds(spd);
    digitalWrite(drive,HIGH);
    delayMicroseconds(spd);
    digitalWrite(drive,LOW);    
    steps--;
  }

}

void EEPROM_float_write(int addr, float val) // запись в ЕЕПРОМ
{  
  byte *x = (byte *)&val;
  for(byte i = 0; i < 4; i++) EEPROM.write(i+addr, x[i]);
}

float EEPROM_float_read(int addr) // чтение из ЕЕПРОМ
{    
  byte x[4];
  for(byte i = 0; i < 4; i++) x[i] = EEPROM.read(i+addr);
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
