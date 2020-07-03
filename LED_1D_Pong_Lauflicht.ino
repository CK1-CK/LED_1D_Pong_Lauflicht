// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// Released under the GPLv3 license to match the rest of the
// Adafruit NeoPixel library

#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        6 // On Trinket or Gemma, suggest changing this to 1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 120 // Popular NeoPixel ring size

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);


#define LedStandardDelay 30 // Time (in milliseconds) to pause between pixels - Startgeschwindigkeit
#define StandardSpeedStep 2
#define MaxScore 6

#define PIN_INPUT_BUTTON_1 7
#define PIN_INPUT_BUTTON_2 8

int LedDelay= LedStandardDelay; // Time (in milliseconds) to pause between pixels - Startgeschwindigkeit
int SpeedStep=StandardSpeedStep;

boolean Button1Pressed = false; // True, wenn der Taster gedrückt wird, sonst false.
boolean Button2Pressed = false; // True, wenn der Taster gedrückt wird, sonst false.
boolean LastButton1Pressed = false; // True, wenn der Taster gedrückt wird, sonst false.
boolean LastButton2Pressed = false; // True, wenn der Taster gedrückt wird, sonst false.
boolean Button1Active=true;
boolean Button2Active=true;

int LedIndex=0; //LED Laufindex
int LastLedIndexButtonPressed; //Letzter Index des Cursors bei Button Click
#define GameArea 6 //LED Spielfeldbegrenzung
#define MaxBrightness 1 //Maximale Helligkeit = 255
boolean Laufrichtung = true; //True=Aufwärts False=Abwärts
boolean CursorStarted= false;
int CountRounds=0; //Anzahl der gespielten Runden, nach jeder Runde wird es schneller
int ScorePlayer1 = 0;
int ScorePlayer2 = 0;
int LastWinner = 0; //Wer hat den Letzten Punkt gemacht

void setup() {
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif // END of Trinket-specific code.
  
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.setBrightness(MaxBrightness);
  //Eingangspins setzen
  pinMode(PIN_INPUT_BUTTON_1, INPUT_PULLUP);
  pinMode(PIN_INPUT_BUTTON_2, INPUT_PULLUP);
  Serial.begin(19200);
  Serial.println("Init beendet\n");

  pinMode(LED_BUILTIN, OUTPUT); //Interne LED
}

void loop() 
{
  CheckButton1();
  CheckButton2();

  if (Button1Pressed && Button1Active)
  { 
    if((Button1Pressed!=LastButton1Pressed))   
    {
      if(CursorStarted==true)
      {
        LastLedIndexButtonPressed=LedIndex;
        if(HasTheRoundAWinner())
        {
          StoppGame();
        }
        else
        {
          ActivateButtonXAndDeactivateTheOther(2);
          SetLaufrichtung(true);
          CountRounds++;
          IncreaseGameSpeed(SpeedStep);
          CheckExtraSpeed();
        }
      }
      else
      {
        StartGame(true);
        SetLowerLEDIndex();
        ActivateButtonXAndDeactivateTheOther(2);
      }
    }
  }

  if (Button2Pressed && Button2Active)
  {
    if((Button2Pressed!=LastButton2Pressed))
    {
      if(CursorStarted==true)
      {
        LastLedIndexButtonPressed=LedIndex;
        if(HasTheRoundAWinner())
        {
          StoppGame();
        }
        else
        {
          ActivateButtonXAndDeactivateTheOther(1);
          SetLaufrichtung(false);
          CountRounds++;
          IncreaseGameSpeed(SpeedStep);
          CheckExtraSpeed();
        }
      }
      else
      {
        StartGame(false);
        SetUpperLEDIndex();
        ActivateButtonXAndDeactivateTheOther(1);
      }
    }
  }  

  LastButton1Pressed=Button1Pressed;
  LastButton2Pressed=Button2Pressed;

  if(CursorStarted)
  {
    MovePixelIndex();
  }
  ShowGameField_Cursor_Score();
  NewRound();
  HasTheGameAWinner();
}

void CheckButton1()
{
  if (digitalRead(PIN_INPUT_BUTTON_1) == LOW) 
  { 
    // Wenn der Taster aktuell gedrückt wird, ... 
    Button1Pressed = true; // merke dir, dass der Taster gedrückt wurde, ... 
    delay(10); // und warte 10 Millisekunden bis die Kontakte des Tasters vollständig geschlossen sind.
  }
  
  if (digitalRead(PIN_INPUT_BUTTON_1) == HIGH && Button1Pressed == true) 
  {
      Button1Pressed = false;  
  }
}
void CheckButton2()
{
  if (digitalRead(PIN_INPUT_BUTTON_2) == LOW) 
  { 
    // Wenn der Taster aktuell gedrückt wird, ... 
    Button2Pressed = true; // merke dir, dass der Taster gedrückt wurde, ... 
    delay(10); // und warte 10 Millisekunden bis die Kontakte des Tasters vollständig geschlossen sind.
  } 
  
  if (digitalRead(PIN_INPUT_BUTTON_2) == HIGH && Button2Pressed == true) 
  {
      Button2Pressed = false;  
  }
}

void SetUpperLEDIndex()
{
  LedIndex=NUMPIXELS-1;
}

void SetLowerLEDIndex()
{
  LedIndex=0;
}

void DeactivateButton(int ButtonNo)
{
  if(ButtonNo==1)
    Button1Active=false;

  if(ButtonNo==2)
    Button2Active=false;
}

void ActivateButton(int ButtonNo)
{
  if(ButtonNo==1)
    Button1Active=true;

  if(ButtonNo==2)
    Button2Active=true;
}

void ActivateButtonXAndDeactivateTheOther(int ButtonNo)
{
  if(ButtonNo==1)
  {
    Button1Active=true;
    Button2Active=false;
  }
    
  if(ButtonNo==2)
  {
    Button1Active=false;
    Button2Active=true;
  }
}

void ToggleLaufrichtung()
{
  Laufrichtung = !Laufrichtung;
}

void BlinkLED()
{
  for(int i=0; i<=5; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(100);                       // wait for a second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(100);                       // wait for a second
  }  
}

void SetLaufrichtung(boolean Lauf)
{
  Laufrichtung=Lauf;
}

void MovePixelIndex()
{
    if(Laufrichtung==true)
      {
        if(LedIndex<(NUMPIXELS-1))
          LedIndex++;
      }
      else
      {
        if(LedIndex>0)
          LedIndex--;
      }
}

boolean HasTheRoundAWinner()
{
  if(Laufrichtung==true)
  { 
    if(LastLedIndexButtonPressed>=NUMPIXELS-1 || LastLedIndexButtonPressed<=(NUMPIXELS-1-GameArea))
    {
      ScorePlayer1++;
      LastWinner=1;
      ActivateButtonXAndDeactivateTheOther(1);
      return true; //Es gab jemanden, der zu spät gedrückt hat.
    }
  }
  else if (LastLedIndexButtonPressed<=0||LastLedIndexButtonPressed>=GameArea)
    {
      ScorePlayer2++;
      LastWinner=2;
      ActivateButtonXAndDeactivateTheOther(2);
      return true; //Es gab jemanden, der zu spät gedrückt hat.
    }

   LastWinner=0;
   return false; //Es gab keinen Gewinner/Verlierer, die nächste Runde kann beginnen
}

void HasTheGameAWinner()
{
  if (ScorePlayer1==MaxScore)
  {
    rainbowCycle(0,0,NUMPIXELS/2-1);
    ResetGame();
  }
  else if (ScorePlayer2==MaxScore)
  {
    rainbowCycle(0,NUMPIXELS/2,NUMPIXELS-1);
    ResetGame();
  }
  
}

void ResetGame()
{
  ResetScore();
  LedIndex=0;
  CountRounds=0;
  LastWinner=0;
  StoppGame();
  SetLaufrichtung(true);
  ResetLEDSpeed();
  ResetSpeedStep();
}

void ResetScore()
{
  ScorePlayer1=0;
  ScorePlayer2=0;
}

void NewRound()
{
  if(LastWinner==1)
  {
    SetLowerLEDIndex();
    Laufrichtung=true;
    ResetLEDSpeed();
  }
  else if (LastWinner==2)
  {
    SetUpperLEDIndex();
    Laufrichtung=false;
    ResetLEDSpeed();
  }
  LastWinner=0;
}

void StartGame(boolean Laufricht)
{
  SetLaufrichtung(Laufricht);
  CursorStarted=true;
  CountRounds=0;
}

void StartGame()
{
  CursorStarted=true;
  CountRounds=0;
}

void StoppGame()
{
  CursorStarted=false;
}

void CreateGameField()
{
  for (int i = 0; i < GameArea; i++)
  {
    pixels.setPixelColor(i, pixels.Color(255, 0, 0));
    pixels.setPixelColor((NUMPIXELS-1)-i, pixels.Color(255, 0, 0));
  }
}

void CreateScore()
{
  pixels.setPixelColor(NUMPIXELS/2-1, pixels.Color(255, 255, 255)); //Mitte

  for (int i = 0; i < ScorePlayer1; i++)
    pixels.setPixelColor(NUMPIXELS/2-2-i, pixels.Color(255, 0, 255));

  for (int i = 0; i < ScorePlayer2; i++)
    pixels.setPixelColor(i+NUMPIXELS/2, pixels.Color(255, 255, 0));
}

void CreateLastPressedCursorPosition()
{
  pixels.setPixelColor(LastLedIndexButtonPressed,pixels.Color(0,0,255));
}

void ShowGameField_Cursor_Score()
{
  if(CursorStarted==true)
  {
    CreateGameField();
    CreateCursorPixel();
    CreateLastPressedCursorPosition();
  }
  else
  {
    pixels.clear();
    CreateGameField();
    CreateScore();
    CreateLastPressedCursorPosition();
  }

  ShowPixels();
}

void ShowPixels()
{
  pixels.show();
  delay(LedDelay);
  pixels.clear();
}

void CreateCursorPixel()
{
      pixels.setPixelColor(LedIndex, pixels.Color(0, 150, 0));
}

void ResetLEDSpeed()
{
  LedDelay=LedStandardDelay;
}

void ResetSpeedStep()
{
  SpeedStep=StandardSpeedStep;
}

void SetSpeedStep(int Step)
{
  SpeedStep=Step;
}

void IncreaseGameSpeed(int Step)
{
  LedDelay=LedDelay-Step;
}

void CheckExtraSpeed()
{
    if(LastLedIndexButtonPressed==NUMPIXELS-2 || LastLedIndexButtonPressed==1)
    {
      LedDelay=LedDelay-SpeedStep+2;
    }
}

void rainbowCycle(uint8_t wait, int LowerIndex, int UpperIndex) // Slightly different, this makes the rainbow equally distributed throughout
{
  uint16_t i, j;
 
  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=LowerIndex; i< UpperIndex; i++) {
      pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + j) & 255));
    }
    pixels.show();
    delay(wait);
  }
}
 
uint32_t Wheel(byte WheelPos) //Input a value 0 to 255 to get a color value. The colours are a transition r - g - b - back to r.
{
  if(WheelPos < 85) {
   return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
