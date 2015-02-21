// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#include <avr/power.h>

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            6

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      150

#define BYTES_TO_READ (NUMPIXELS * 3)
#define RESET_BIT -2

// This is 0xFE

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 500; // delay for half a second
char readB[BYTES_TO_READ];
char color[BYTES_TO_READ];
int pixIdx0;

int led = 13;


void setAllPixels(uint32_t color) {
  for(int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, color);
  }
  pixels.show();
}

void readToReset() {
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  int numResets = 0;
  while(numResets < 6) {
    while(Serial.available() < 1) {
      // Wait for some to be avaliable
      delay(1);
    }
    char r = Serial.read();
    if(r == -1) {
      // Should never happen.
      continue;
    } else if(r == RESET_BIT) {
      numResets++;
    } else {
      numResets = 0;
    }
  }
  digitalWrite(led, LOW);   // turn the LED on (HIGH is the voltage level)
}

//bool checkLastBytesReset(char bytes[]) {
//  const size_t x = BYTES_TO_READ;
//  bool y =  bytes[x - 6] == RESET_BIT &&
//         bytes[x - 5] == RESET_BIT &&
//         bytes[x - 4] == RESET_BIT &&
//         bytes[x - 3] == RESET_BIT &&
//         bytes[x - 2] == RESET_BIT &&
//         bytes[x - 1] == RESET_BIT;
//  return y;
//}


void setup() {
  Serial.begin(1000000);
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code

  pixels.begin(); // This initializes the NeoPixel library.
  
  memset(color, 32, sizeof(color));
  
  
  pinMode(led, OUTPUT);
  setAllPixels(pixels.Color(0, 0, 32));
}

void loop() {  
  // For a set of NeoPixels the first NeoPixel is 0, second is 1, all the way up to the count of pixels minus one.
//  while(Serial.available() < 1) {
//    delay(1);
//  }
  int read = Serial.readBytes(readB, BYTES_TO_READ);
  if (read != BYTES_TO_READ) {
    // Insufficient data read.
    // 2 Options:
    //  - Continue reading and adding to array.
    //  - Ignore this buffer(reuse old one) and read to reset.
    
    // I'm lazy
    // GREEN
  } else {
    memcpy(color, readB, sizeof(color));
  }
  
  for(int pix = 0; pix < NUMPIXELS; pix++) {
    pixels.setPixelColor(pix, pixels.Color(color[(pix * 3)], color[(pix * 3) + 1], color[(pix * 3) + 2]));
  }
  pixels.show();
  Serial.write("a");
}
