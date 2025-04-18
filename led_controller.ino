#include <Adafruit_NeoPixel.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoMatrix.h>

#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif

#define SCREEN_WIDTH 128 // OLED width,  in pixels
#define SCREEN_HEIGHT 64 // OLED height, in pixels

#define PIN1        4        // Pin where NeoPixels are connected. Was 6. 
#define PIN2        5        //Works for ESP32
#define WIDTH      15       // Width of the LED matrix 32
#define HEIGHT     8        // Height of the LED matrix
#define NUMPIXELS_PER_STRIP  200 // Total number of LEDs

Adafruit_NeoPixel strip1(NUMPIXELS_PER_STRIP, PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(NUMPIXELS_PER_STRIP, PIN2, NEO_GRB + NEO_KHZ800);


Adafruit_NeoMatrix matrix1 = Adafruit_NeoMatrix(15, 8, PIN1,
  NEO_MATRIX_BOTTOM     + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_RGB            + NEO_KHZ800);

Adafruit_NeoMatrix matrix2 = Adafruit_NeoMatrix(15, 8, PIN2,
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_RGB            + NEO_KHZ800);

//sudo chmod a+rw /dev/ttyUSB0
int Red = 163;
int Green = 6;
int Blue = 163;

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;



byte mainmenu = 2;
byte animation_menu = 0;
byte colour_menu = 0;
byte* selectedMenu;

struct SelectedMenu{
  int menu;
  int item;
  char* name;
  int len;
};

SelectedMenu currentMenu;

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

uint32_t value = 0;
String oldcolour;

int x = matrix1.width();
int pass = 0;


#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      BLEDevice::startAdvertising();
    }
};


void menuchosen()
{

  if(mainmenu != 0)
  {
    currentMenu.menu = 0;
    currentMenu.item = mainmenu;
    currentMenu.len = 2;
  }
  
  if(animation_menu != 0)
  {
    currentMenu.menu = 1;
    currentMenu.item = animation_menu;
    currentMenu.len = 6;
  }

  if(colour_menu != 0)
  {
    currentMenu.menu = 2;
    currentMenu.item = colour_menu;
    currentMenu.len = 6;
  }
}

byte const unsigned long full[8] =
{
0x7FFF,
0x7FFF,
0x7FFF,
0x7FFF,
0x7FFF,
0x7FFF,
0x0000,
0x0000
};

byte const unsigned long newBlinking[3][8] =
{
  {
    0x780F, //111100000001111
    0x3C07, //011110000000111
    0x1800, //001100000000000
    0x000,  //000000000000000
    0x18,   //000000000011000
    0xE7E,  //000111001111110
    0x7E7,  //000011111100111
    0x0183  //000000110000011
  },
  {
    0x780F, //111100000001111
    0x3C07, //011110000000111
    0x00,   //000000000000000
    0x000,  //000000000000000
    0x18,   //000000000011000
    0xE7E,  //000111001111110
    0x7E7,  //000011111100111
    0x0183  //000000110000011
  },
  {
    0x780F, //111100000001111
    0x0007, //000000000000111
    0x0000, //000000000000000
    0x000,  //000000000000000
    0x18,   //000000000011000
    0xE7E,  //000111001111110
    0x7E7,  //000011111100111
    0x0183  //000000110000011
  }
};


byte const unsigned long happy[3][8] =
{
  {
    0x780F, //111100000001111
    0x3C07, //011110000000111
    0x1800, //001100000000000
    0x000,  //000000000000000
    0x18,   //000000000011000
    0xE7E,  //000111001111110
    0x7E7,  //000011111100111
    0x0183  //000000110000011
  },
  {
    0x380F, //011100000001111
    0x3C07, //011110000000111
    0x00,   //000000000000000
    0x000,  //000000000000000
    0x18,   //000000000011000
    0xE7E,  //000111001111110
    0x7E7,  //000011111100111
    0x0183  //000000110000011
  },
  {
    0x180F, //001100000001111
    0x2407, //010010000000111
    0x0000, //000000000000000
    0x000,  //000000000000000
    0x18,   //000000000011000
    0xE7E,  //000111001111110
    0x7E7,  //000011111100111
    0x0183  //000000110000011
  }
};

byte const unsigned long boyk[8] =
{
  0x0000,  // 000000000000000
  0x3E3E,  // 011111000111110
  0x1634,  // 001011000110100
  0x0630,  // 000011000110000
  0x0000,  // 000000000000000
  0x1002,  // 001000000000010
  0x2494,  // 010010010010100
  0x0360   // 000001101100000
};


int getLEDIndex(int x, int y, bool flip = false) {
    
    if (flip) {
        // Flipping horizontally by reversing the x-coordinate for strip2
        y = HEIGHT - 1 - y;
    }
    if (x % 2 == 0) {
        // Even column, left-to-right
        return x * HEIGHT + y;
    } else {
        // Odd column, right-to-left
        return x * HEIGHT + (HEIGHT - 1 - y);
    }
}



void lineByLineAnimation(byte const unsigned long frame[8])
{
    
  for(int x = HEIGHT-1; x >= 0; x--)
  {
    int row = frame[x]; 
    for (int n = 0; n <= 14;n++)
    {
      if(row & (1<<n))
      {
        strip1.setPixelColor(getLEDIndex(n,x,true), strip1.Color(Red, Green, Blue));
        strip1.show();
        delay(10);
      }
      else
      {
        strip1.setPixelColor(getLEDIndex(n,x,true), strip1.Color(Red, Green, Blue));
        strip1.show();
        delay(10);
        strip1.setPixelColor(getLEDIndex(n,x,true), strip1.Color(0, 0, 0));
        strip1.show();
      }
    }
  }
}
    
void straightLineAnimation(byte const unsigned long frame[8])
{

  int led[8];
  int ledStatus[8];


  for (int n = 0; n <= 14;n++)
  {
    int index = 0;
    for(int x = HEIGHT-1; x >= 0; x--)
    {
      int row = frame[x]; 
    
        if(row & (1<<n))
        {
          led[index] = getLEDIndex(n,x,true);
          ledStatus[index] = 1;
          index++; 
        }
        else
        {
          led[index] = getLEDIndex(n,x,true);
          ledStatus[index] = 0;
          index++;
        }

    }
    for(int lednum = 0; lednum < 8; lednum++)
    {
        strip1.setPixelColor(led[lednum], strip1.Color(Red, Green, Blue));
        Serial.println(led[lednum]);
        
    }
        strip1.show();
          delay(20);

    for(int lednum = 0; lednum < 8; lednum++)
    {
        if(ledStatus[lednum] == 0)
        {
          strip1.setPixelColor(led[lednum], strip1.Color(0,0,0));
        }
    }
    strip1.show();
  }


}

void diagonalstart()
{
 for(int k = 0; k < HEIGHT+WIDTH-1;k++)  //Combination of both
  {
    for(int x = 0; x < WIDTH;x++)
    {
      for(int y = 0; y < HEIGHT;y++) 
      {
        if((x+y) == k)
        {
          strip1.setPixelColor(getLEDIndex(x,y,true), strip1.Color(Red, Green, Blue));
          strip2.setPixelColor(getLEDIndex(x,y), strip1.Color(Red, Green, Blue));
        }
      }
    }
    strip1.show();
    strip2.show();
    delay(10);
    for(int x = 0; x < WIDTH;x++)
    {
      for(int y = 0; y < HEIGHT;y++)
      {
        if((x+y) == k)
        {
          if(newBlinking[0][y] & (1<<x))
          {
            
          }
          else
          {
            strip1.setPixelColor(getLEDIndex(x,y,true), strip1.Color(0,0,0));
            strip2.setPixelColor(getLEDIndex(x,y), strip1.Color(0,0,0));
          }
           
        }
      }
    }
  }
strip1.show();
strip2.show();
}


void diagonalChange(byte const unsigned long frame[8],int R, int G, int B)
{


  for(int k = HEIGHT+WIDTH-1; k > 0 ;k--)  
  {
    for(int x = WIDTH-1; x >= 0;x--)//
    {
      for(int y = 0; y < HEIGHT ;y++) 
      {
        if((x+y) == k)
        {
          strip1.setPixelColor(getLEDIndex(x,y,true), strip1.Color(Red, Green, Blue));
          strip2.setPixelColor(getLEDIndex(x,y), strip1.Color(Red, Green, Blue));
        }
      }
    }
    strip1.show();
    strip2.show();
    delay(10);
    for(int x = WIDTH-1; x >= 0; x--)
    {
      for(int y = 0; y < HEIGHT ;y++)
      {
        if((x+y) == k)
        {

            strip1.setPixelColor(getLEDIndex(x,y,true), strip1.Color(0,0,0));
            strip2.setPixelColor(getLEDIndex(x,y), strip1.Color(0,0,0));
           
        }
      }
    }
  }
  strip1.show();
  strip2.show();
  Red = R;
  Green = G;
  Blue = B;


  for(int k = 0; k < HEIGHT+WIDTH-1;k++)  //Combination of both
  {
    for(int x = 0; x < WIDTH;x++)
    {
      for(int y = 0; y < HEIGHT;y++) 
      {
        if((x+y) == k)
        {
          strip1.setPixelColor(getLEDIndex(x,y,true), strip1.Color(Red, Green, Blue));
          strip2.setPixelColor(getLEDIndex(x,y), strip1.Color(Red, Green, Blue));
        }
      }
    }
    strip1.show();
    strip2.show();
    delay(10);
    for(int x = 0; x < WIDTH;x++)
    {
      for(int y = 0; y < HEIGHT;y++)
      {
        if((x+y) == k)
        {
          if(frame[y] & (1<<x))
          {
            
          }
          else
          {
            strip1.setPixelColor(getLEDIndex(x,y,true), strip1.Color(0,0,0));
            strip2.setPixelColor(getLEDIndex(x,y), strip1.Color(0,0,0));
          }
           
        }
      }
    }
  }
strip1.show();
strip2.show();

}


void loadingAnim()
{
for(int y = 0; y < 57; y++)
{
  matrix1.fillScreen(0);
  matrix2.fillScreen(0);
  matrix1.setCursor(x, 0);
  matrix2.setCursor(x, 0);
  matrix1.print(F("Loading"));
  matrix2.print(F("Loading"));
  if(--x < -40) { //-36
    x = matrix1.width();
    x = matrix2.width();
    matrix1.setTextColor(matrix1.Color(163, 163, 6));
    matrix2.setTextColor(matrix2.Color(163, 163, 6));
  }
  matrix1.show();
  matrix2.show();
  delay(40);
}
}

void moveToMenu()
{
  switch(currentMenu.menu)
  {
    case 0:
    mainmenu = 0;
      switch(currentMenu.item)
      {
        case 1:
        animation_menu = 1;
        break;
        case 2:
        colour_menu = 1;
        break;
      }
    break;
    case 1:
      switch(currentMenu.item)
      {
        case 1:
        //select the first animation
        break;
        case 2:
        //select the second animation
        break;
      }
    break;
    case 2:
      switch(currentMenu.item)
      {
        case 1:
        //select the first colour
        break;
        case 2:
        //select the second colour
        break;
      }
    break;
  }
  
}



void renderMenu(int menu, int item, int len)
{
  item -= 1;
  int pos = 0;
  for(int x = 0; x < len && x < 3; x++)
  {
    oled.setTextSize(1);         // set text size
    oled.setTextColor(WHITE);    // set text color
    oled.setCursor(0, (x * 10 + 22));       // set position to display
    
    if(x == item)
    {
      oled.print(">");
    }

    oled.print("Menu Option "); // set text
    oled.println(x);
    pos = x;
  }

  oled.setTextSize(1);         // set text size
  oled.setTextColor(WHITE);    // set text color
  oled.setCursor(0, (++pos * 10 + 22));       // set position to display
  
  if(3 == item)
  {
    oled.print(">");
  }

  oled.print("Back"); // set text

}

void setup() {
  Serial.begin(9600);

  strip1.begin();
  strip2.begin();

  strip1.show();  
  strip2.show();


  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("failed to start SSD1306 OLED"));
    while (1);
  }

  delay(1000);
  matrix1.begin();
  matrix1.setTextWrap(false);
  matrix1.setBrightness(40);
  matrix1.setTextColor(matrix1.Color(3, 248, 248));

  matrix2.begin();
  matrix2.setTextWrap(false);
  matrix2.setBrightness(40);
  matrix2.setTextColor(matrix2.Color(3, 248, 248));

  //renderFrame(newBlinking);
  //lineByLineAnimation(newBlinking);
  //straightLineAnimation(newBlinking);
  // Create the BLE Device

  BLEDevice::init("ProotBrain");

  
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  
                    );

  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
loadingAnim();
diagonalstart();

delay(2000);




//diagonalChange(boyk, 163,6,163);

delay(1000);



}

void loop(){

  menuchosen();
  Serial.print("Current menu: ");
  Serial.println(currentMenu.menu);


  Serial.print("Current item: ");
  Serial.println(currentMenu.item);


  Serial.print("Current length: ");
  Serial.println(currentMenu.len);

  oled.clearDisplay(); // clear display

  oled.setTextSize(1);         // set text size
  oled.setTextColor(WHITE);    // set text color
  oled.setCursor(0, 0);       // set position to display
  oled.println("Colour: "); // set text


  oled.setTextSize(1);         // set text size
  oled.setTextColor(WHITE);    // set text color
  oled.setCursor(0, 10);       // set position to display
  oled.println("Animation: "); // set text

  oled.drawLine(0,20,127,20,WHITE);
  
  renderMenu(currentMenu.menu,  currentMenu.item, currentMenu.len);

   oled.display();    

  moveToMenu();



  String newcolour;
   newcolour = pCharacteristic->getValue();


  if(newcolour != oldcolour)
  {
      Serial.print("Red: ");
      Serial.println(newcolour.substring(0,3).toInt());
      Serial.print("Green: ");
      Serial.println(newcolour.substring(4,8).toInt());
      Serial.print("Blue: ");
      Serial.println(newcolour.substring(8,12).toInt());
      diagonalChange(newBlinking[0], newcolour.substring(0,3).toInt(), newcolour.substring(4,8).toInt(), newcolour.substring(8,12).toInt());    
      oldcolour = newcolour;
  }

  

  // switch (colour){
  //   case 1:
  //   Serial.println("Red");
  //   diagonalChange(newBlinking[0], 255,0,0);
  //   pCharacteristic->setValue("0");
  //   delay(1000);
  //   break;
  //   case 2:
  //   Serial.println("Green");
  //   diagonalChange(newBlinking[0], 0,255,0);
  //   pCharacteristic->setValue("0");
  //   delay(1000);
  //   break;
  //   case 3:
  //   Serial.println("Blue");
  //   diagonalChange(newBlinking[0], 0,0,255);
  //   pCharacteristic->setValue("0");
  //   delay(1000);
  //   break;
  // }
  // Serial.println(colour);
  //   delay(1000);
  // diagonalChange(newBlinking[0], 163,0,163);
  // delay(3000);
  

  // diagonalChange(newBlinking[0], 255,0,0);
  // delay(3000);


  // diagonalChange(newBlinking[0], 0,255,0);
  // delay(3000);


  // diagonalChange(newBlinking[0], 0,0,255);
  // delay(3000);


  // diagonalChange(newBlinking[0], 100,100,100);
  // delay(3000);


  // runAnimation(0);
  // delay(1000);

  // diagonalChange(newBlinking[0], 0,0,255);
  // delay(1000);

  // runAnimation(1);
  // delay(1000);

  // diagonalChange(newBlinking[0], 00,100,0);
   delay(1000);

}


void renderFrame(byte const unsigned long animation[8])
{

  strip1.clear();
  strip2.clear();
  for(int x = 0; x < HEIGHT; x++)
  {
    int row = animation[x]; 
    for (int n = 14; n >= 0;n--)
    {
      if(row & (1<<n))
      {
        strip1.setPixelColor(getLEDIndex(n,x,true), strip1.Color(Red, Green, Blue));
        strip2.setPixelColor(getLEDIndex(n,x), strip2.Color(Red, Green, Blue));
      }
    }
    Serial.println("");

  }
 strip1.show(); 
 strip2.show(); 
}
// Function to map 2D design array to the NeoPixel strip and display it
void runAnimation(int animationID) 
{
 
 switch (animationID)
 {

  case 0:
  {
    renderFrame(newBlinking[0]);
    delay(50);
    renderFrame(newBlinking[1]);
    delay(50);
    renderFrame(newBlinking[2]);
    delay(100);
    renderFrame(newBlinking[1]);
    delay(100);
    renderFrame(newBlinking[0]);
 
  }
  break;
  case 1:
    renderFrame(happy[0]);
    delay(50);
    renderFrame(happy[1]);
    delay(50);
    renderFrame(happy[2]);
    delay(1000);
    renderFrame(happy[1]);
    delay(50);
    renderFrame(happy[0]);
  break;
  case 2:

  break;
  case 3:

  break;
  default:

    break;
 }

   
}


