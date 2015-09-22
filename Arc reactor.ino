#include <Bounce2.h>
#include <Adafruit_NeoPixel.h>
/****************************************
Example Sound Level Sketch for the 
Adafruit Microphone Amplifier
****************************************/

#define BUTTON_PIN 3
#define NEOPIN 8
 
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;


int previousPeak = 0;
int previousDisplayPeak = 0;
int fallRate = 25; // rate that the value of peakToPeak falls at when is lower than previousPeak
int fadeFallRate = 10;
int buffer = 20; // buffer for setting leds to fall
byte mode = 8;

unsigned long previousMode = 0;

Bounce debouncer = Bounce(); // instantiate a bounce object
Adafruit_NeoPixel strip = Adafruit_NeoPixel(15, NEOPIN, NEO_GRB + NEO_KHZ800);
 
 
 
void setup() 
{
  // Button debouncing setup
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(5); // interval in ms default is 5
  
  // NeoPixel
  strip.begin();
  strip.show(); // initialize all pixels to off
  
   Serial.begin(9600);
}
 
 
void loop() 
{
    Serial.println("");
   
   // check mode button
   debouncer.update();
   int buttonValue = debouncer.read();
   if ( buttonValue == LOW && (millis() - previousMode) > 1000) // button pressed
   { 
    modeChange(); // cycle to next mode 
   }
  
   unsigned long startMillis= millis();  // Start of sample window
   unsigned int peakToPeak = 0;   // peak-to-peak level
 
   unsigned int signalMax = 0;
   unsigned int signalMin = 1024;
   
   
 
   // collect data for 50 mS
   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(0);
      if (sample < 1024)  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;  // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;  // save just the min levels
         }
      }
   }
   peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
   double volts = (peakToPeak * 3.3) / 1024;  // convert to volts
   
   
   if((peakToPeak + buffer) < previousPeak)
   {
    if(previousPeak > fallRate) // check that previous peak will not go below zero
    {
     previousPeak -= fallRate;    
    }
    else
    {
     previousPeak = 0;
    }
    sendToLeds(previousPeak);
   }
   else if((peakToPeak) >= previousPeak)
   {
    previousPeak = peakToPeak;
    sendToLeds(previousPeak);
   }

   

}



void sendToLeds(int peak)
{
  strip.clear();
  
  int displayPeak = map(peak, 50, 500, 0, strip.numPixels() + 1);
  
  
  switch (mode) {
    case 1: { // rainbow mode
      boolean breaking = false;
      while (true)
      {
       uint16_t i, j;

       for(j=0; j<256; j++) {
         for(i=0; i<strip.numPixels(); i++) {
           strip.setPixelColor(i, Wheel((i+j) & 255));
         }
         strip.show();
         debouncer.update(); 
         if (debouncer.read() == LOW && (millis() - previousMode) > 1000){
          breaking = true;
          break; 
         }
         delay(20);
       }
       if (breaking)
         break;
      }
      modeChange();
      break; // switch break
    }
    
    case 2: {// sequential mode blue sound reactive
      for (int i = 0; i < displayPeak; i++)
      {
        strip.setPixelColor(i, strip.Color(0, 0, 255));
      }
      
     /* if (previousDisplayPeak > displayPeak) // fades previously on pixels to off
      {
       
       for (int i = displayPeak + 1; i <= previousDisplayPeak; i++)
       {
         uint8_t b = 0;
         // check if pixel needs to be faded//////////////////////////
        // fade by - 25 off of rgb value each time 
        if (splitColor(i, 'b') > fadeFallRate)// finds value of blue of pixel i
        {
         b = splitColor(i, 'b') - fadeFallRate;        
        }
        strip.setPixelColor(i, strip.Color(0, 0, b) );
       }
       if (displayPeak = 0)
       {
         uint8_t b = 0;
         // check if pixel needs to be faded//////////////////////////
        // fade by - 25 off of rgb value each time 
        if (splitColor(0, 'b') > fadeFallRate)// finds value of blue of pixel i
        {
         b = splitColor(0, 'b') - fadeFallRate;        
        }
        strip.setPixelColor(0, strip.Color(0, 0, b) );
       }
      } */

      break;
    }
    
    case 3: { // middle out brightness mode blue gradient sound reactive
      for (byte i = 0; i< strip.numPixels(); i++)
        strip.setPixelColor(i, strip.Color(0,0,10));
      for (int i = 1; i< (displayPeak /2); i++)
      {
       strip.setPixelColor(6 + i, strip.Color(20, 50, (10 + (i*(245/displayPeak)) ))); // start at middle of ring
       strip.setPixelColor(8 - i, strip.Color(20, 50, (10 + (i*(245/displayPeak)) )));;   
      }
      break;
    }
    
    case 4: {// middle out brightness mode white sound reactive
      for (byte i = 0; i< strip.numPixels(); i++)
        strip.setPixelColor(i, strip.Color(5,5,5));
      for (int i = 1; i< (displayPeak /2); i++)
      {
       strip.setPixelColor(6 + i, strip.Color((55 + (i*(200/displayPeak)) ) - 15, (55 + (i*(200/displayPeak)) ), (55 + (i*(200/displayPeak)) ))); // start at middle of ring
       strip.setPixelColor(8 - i, strip.Color((55 + (i*(200/displayPeak)) ) - 15, (55 + (i*(200/displayPeak)) ), (55 + (i*(200/displayPeak)) )));;   
      }
      break;
    }
      
    case 5: // brightness sound reactive color: white
    {
      int b = map(peak, 0, 512, 0, 255);
      for (int i = 0; i < strip.numPixels(); i++)
      {
        strip.setPixelColor(i, strip.Color(b,b,b));      
      }
      break;
    }
    
    case 6: // brightness sound reactive color: teal
    {
      int b = map(peak, 0, 512, 5, 255);
      for (int i = 0; i < strip.numPixels(); i++)
      {
        strip.setPixelColor(i, strip.Color( (b/2),(b/2),b ) );      
      }
      break;
    }
    
    case 7: // brightness sound reactive color: blue
    {
      int b = map(peak, 0, 512, 5, 255);
      for (int i = 0; i < strip.numPixels(); i++)
      {
        strip.setPixelColor(i, strip.Color(0,0,b ) );      
      }
      break;
    }
    
    case 8: { //pulsing mode
      boolean breaking = false;
      while (true)
      {
       for (int i = 10; i < 100; i++)
       {
         for(int j = 0; j< strip.numPixels(); j++)
           strip.setPixelColor(j, strip.Color( (i/4),(i/2),i ) ); 
         strip.show();
         debouncer.update(); 
         if (debouncer.read() == LOW && (millis() - previousMode) > 1000){
           breaking = true;
           break; 
         }
         delay(40);
       }
       
       if (breaking)
         break;
       for (int i = 100; i > 10; i--)
       {
         for(int j = 0; j< strip.numPixels(); j++)
           strip.setPixelColor(j, strip.Color( (i/4),(i/2),i ) ); 
         strip.show();
         debouncer.update(); 
         if (debouncer.read() == LOW && (millis() - previousMode) > 1000){
           breaking = true;
           break; 
         }
         delay(40);
       }
       
       if (breaking)
         break;
      }
      modeChange();
      break; // switch break
    }
    
    case 9: //swirly
    {
     boolean breaking = false;
     while (true)
     {
      for (int i = 0; i < strip.numPixels(); i++)
      {
        for(int k = 0; k < strip.numPixels(); k++)
          strip.setPixelColor(k, strip.Color( 0,0,10 ) ); 
        
        for(int j = 0; j< 5; j++)
        {
         if ( i + j < 15)
          strip.setPixelColor(i+j, strip.Color( 20,40,215 - (40*j) ) ); 
         else
          strip.setPixelColor( (i+j) - 15, strip.Color( 20,40,215 - (40*j) ));          
         
         if (i - j >= 0)
           strip.setPixelColor(i-j, strip.Color( 20,40,215 - (40*j) ) );
         else
           strip.setPixelColor((i-j) + 15, strip.Color( 20,40,215 - (40*j) ) );
        } // second for
        
        
        
        
        strip.show();
        debouncer.update(); 
        if (debouncer.read() == LOW && (millis() - previousMode) > 1000){
          breaking = true;
          break; 
        } // if 
        delay(90);
      } // first for
       
      if (breaking)
        break;
     } //while
      
     
     break; 
    }
    
    
    
  } // end switch
  strip.show();

  if (displayPeak > previousDisplayPeak)
  {
    previousDisplayPeak = displayPeak;
  }
  
  
  
}

void modeChange()
{
 byte numberOfModes = 9; // number of 'modes' or cases we have made
 if (mode < numberOfModes){
  mode++; 
 }
 else{
  mode = 1; 
 }
 
 previousMode = millis();
 
}


uint8_t splitColor(int pixel, char color)
{
  uint32_t c = strip.getPixelColor(pixel);
  switch (color) {
   case 'r': return (uint8_t)(c >> 16); 
   case 'g': return (uint8_t)(c >> 8);
   case 'b': return (uint8_t)(c >> 0);
   default: return 0;
  }
  // function from SeiRruf Adafruit forums
  // http://forums.adafruit.com/viewtopic.php?f=47&t=60176
}




// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
