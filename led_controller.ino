#include <Adafruit_NeoPixel.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoMatrix.h>
#include <BLEAdvertisedDevice.h>
//Proots MAC ADDRESS 14:2b:2f:c4:e8:80
//sudo chmod a+rw /dev/ttyUSB0

#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif


//Screen resolution
#define SCREEN_WIDTH 128 // OLED width,  in pixels
#define SCREEN_HEIGHT 64 // OLED height, in pixels



#define PIN1        4            // Pin for first matrix
#define PIN2        5            // Pin for second matrix
#define WIDTH      19            // Width of the LED matrix 
#define HEIGHT     8             // Height of the LED matrix
#define NUMPIXELS_PER_STRIP  200 // Total number of LEDs


//Prepare LEDs
Adafruit_NeoPixel strip1(NUMPIXELS_PER_STRIP, PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(NUMPIXELS_PER_STRIP, PIN2, NEO_GRB + NEO_KHZ800);

//Prepare LEDs to support text
Adafruit_NeoMatrix matrix1 = Adafruit_NeoMatrix(20, 8, PIN1,
  NEO_MATRIX_BOTTOM     + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_RGB            + NEO_KHZ800);

Adafruit_NeoMatrix matrix2 = Adafruit_NeoMatrix(20, 8, PIN2,
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_RGB            + NEO_KHZ800);

//Default colour 
int Red = 163;
int Green = 6;
int Blue = 163;

//Client ID and Services ID
static BLEUUID SERVICE_UUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID moveButtonUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");
static BLEUUID selectButtonUUID("beb5483e-36e1-4688-b7f5-ea07361b26a2");


static boolean doConnect = false;   //Try to connect
static boolean connected = false;   //If BLE connection was succseful. 
static boolean doScan = false;      //Scan on disconect

//WHY?
bool deviceConnected = false;
bool oldDeviceConnected = false;


BLEServer* pServer = NULL;          //Set up a server
//BLECharacteristic* pCharacteristic = NULL;

//Prepare BLE characteristics
static BLERemoteCharacteristic *pCharacteristicMoveButton;
static BLERemoteCharacteristic *pCharacteristicSelectButton;
static BLEAdvertisedDevice *myDevice;


bool rainbow = false;   //Raindow toggle
bool blinking = false;  //Blinking toggle

bool up = false;                    //Rainbow direction
int rainbowcolour;                  //Rainbow colour
int colour;                         //Colour out of RGB
int colourToChange = random(1,4);   //A new colour to pick
int currentAnimation = 0;           //Current transition animation

//Initial menu WHY?
byte mainmenu = 1;
byte animation_menu = 0;
byte colour_menu = 0;

//Structure for current menu
struct SelectedMenu{
  int menu;
  int item;
  char* name;
  char* options;
  int len;
};

SelectedMenu currentMenu;

//Prepare the OLED screen
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


uint32_t value = 0;       //What's this?
String oldColour;         //Old colour for BLE
String newColour;         //New colour for BLE
int x = matrix1.width();  //WHY?
int pass = 0;             //WHY?

//Callback for debugging
static void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify) {
  Serial.print("Notify callback for characteristic ");
  Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
  Serial.print(" of data length ");
  Serial.println(length);
  Serial.print("data: ");
  Serial.write(pData, length);
  Serial.println();
}

//Frame storage
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

//Blinking animation
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

//Happy animation
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

//Boykisser animation
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

//Turn the zigzag order into a x by x matrix
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

//Render frame the line by line
void lineByLineRender(byte const unsigned long frame[8])
{
    
  for(int x = HEIGHT-1; x >= 0; x--)
  {
    int row = frame[x]; 
    for (int n = 0; n <= 14;n++)
    {
      if(row & (1<<n))
      {
        strip1.setPixelColor(getLEDIndex(n,x,true), strip1.Color(Red, Green, Blue));
        strip2.setPixelColor(getLEDIndex(n,x,true), strip2.Color(Red, Green, Blue));
        strip1.show();
        strip2.show();
        delay(1);
      }
      else
      {
        strip1.setPixelColor(getLEDIndex(n,x,true), strip1.Color(Red, Green, Blue));
        strip2.setPixelColor(getLEDIndex(n,x,true), strip2.Color(Red, Green, Blue));
        strip1.show();
        strip2.show();
        delay(1);
        strip1.setPixelColor(getLEDIndex(n,x,true), strip1.Color(0, 0, 0));
        strip2.setPixelColor(getLEDIndex(n,x,true), strip2.Color(0, 0, 0));
        strip1.show();
        strip2.show();
      }
    }
  }
}
    
//Render with straight vertical line
void straightLineRender(byte const unsigned long frame[8])
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
        strip2.setPixelColor(led[lednum], strip2.Color(Red, Green, Blue));
    }
        strip1.show();
        strip2.show();
        delay(20);

    for(int lednum = 0; lednum < 8; lednum++)
    {
        if(ledStatus[lednum] == 0)
        {
          strip1.setPixelColor(led[lednum], strip1.Color(0,0,0));
          strip2.setPixelColor(led[lednum], strip1.Color(0,0,0));
        }
    }
    strip1.show();
    strip2.show();
  }


}

//Render with diagonal line
void diagonalStartRender()
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


void diagonalChangeRender(byte const unsigned long frame[8],int R, int G, int B)
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

void displayText(const char* text)
{
  int lenght = (strlen(text)*7);
  int matrixsize = (0-(strlen(text)*7));
  int x = matrix1.width(); 

  for(int y = 0; y < lenght; y++)
  {
    matrix1.fillScreen(0);
    matrix2.fillScreen(0);
    matrix1.setCursor(x, 0);
    matrix2.setCursor(x, 0);
    matrix1.print(F(text));
    matrix2.print(F(text));
    if(--x < matrixsize) 
    { // Lenght of the matrix for the animation
      x = matrix1.width();
      x = matrix2.width();
      matrix1.setTextColor(matrix1.Color(Green,Blue, Red));
      matrix2.setTextColor(matrix2.Color(Green, Blue, Red));
    }
    matrix1.show();
    matrix2.show();
    delay(40); //Animation speed
  }
}

//Render the loading text animation
void loadingAnimRender()
{
//Length of the animation: 56 for 7 characters
//Length of the matrix: -40
//Animation speed 40
//
for(int y = 0; y < 56; y++) //Time of the animation roughly 8 per character.
{
  matrix1.fillScreen(0);
  matrix2.fillScreen(0);
  matrix1.setCursor(x, 0);
  matrix2.setCursor(x, 0);
  matrix1.print(F("Loading"));
  matrix2.print(F("Loading"));
  if(--x < -40) { // Lenght of the matrix for the animation
    x = matrix1.width();
    x = matrix2.width();

  }
  matrix1.show();
  matrix2.show();
  delay(40); //Animation speed
}
}
//Return to the main menu
void returnToMainMenu()
{
  currentMenu.menu = 0;
  currentMenu.item = 1;
  currentMenu.len = 3;
    


  free(currentMenu.name);
  free(currentMenu.options);

  currentMenu.name = (char*)malloc(strlen("Main")+1);
  strcpy(currentMenu.name, "Main");

  currentMenu.options = (char*)malloc(strlen("Animations|Colours|Modes")+1);
  strcpy(currentMenu.options, "Animations|Colours|Modes");
    

}

//Changes the menu based on the current selected option
void moveToMenu()
{
  switch(currentMenu.menu)
  {
    case 0:
      switch(currentMenu.item)
      {
        case 1:
        currentMenu.menu = 1;
        currentMenu.item = 1;
        currentMenu.len = 6;
        
        free(currentMenu.name);
        free(currentMenu.options);

        currentMenu.name = (char*)malloc(strlen("Animations")+1);
        strcpy(currentMenu.name, "Animations");
    
        currentMenu.options = (char*)malloc(strlen("Blink|Smile|Sad|Confusion|Angry")+1);
        strcpy(currentMenu.options, "Blink|Smile|Sad|Confusion|Angry");
        break;
        
        case 2:
        currentMenu.menu = 2;
        currentMenu.item = 1;
        currentMenu.len = 6;

        free(currentMenu.name);
        free(currentMenu.options);

        currentMenu.name = (char*)malloc(strlen("Colours")+1);
        strcpy(currentMenu.name, "Colours");
    
        currentMenu.options = (char*)malloc(strlen("Purple|Orange|Yellow|Red|Green|Blue")+1);
        strcpy(currentMenu.options, "Purple|Orange|Yellow|Red|Green|Blue");

        break;
        case 3:
        currentMenu.menu = 3;
        currentMenu.item = 1;
        currentMenu.len = 3;

        free(currentMenu.name);
        free(currentMenu.options);

        currentMenu.name = (char*)malloc(strlen("Modes")+1);
        strcpy(currentMenu.name, "Modes");
    
        currentMenu.options = (char*)malloc(strlen("Rainbow|Blinking|Transition")+1);
        strcpy(currentMenu.options, "Rainbow|Blinking|Transition");

        break;
      }
    break;
    case 1:
      switch(currentMenu.item)
      {
        case 1:
          runAnimation(0);
        break;

        case 2:
          runAnimation(1);
        break;

        case 7:
          returnToMainMenu();
        break;
      }
    break;
    case 2:
      switch(currentMenu.item) //Purple|Orange|Yellow|Red|Blue|Green
      { 
        case 1: //ADD LIGHT, WHITE AND DIM PINK
                newColour = "163 003 163"; //\\3, 248, 248
        break;
        case 2:
                newColour = "255 040 000";//Too green
        break;
        case 3:
                newColour = "255 255 006";
        break;
        case 4:
                newColour = "255 000 000";
        break;
        case 5:
                newColour = "000 255 000";
        break;
        case 6:
                newColour = "000 000 255";
        break;
        case 7:
          returnToMainMenu();
        break;
      }
    break;
    case 3:
        switch (currentMenu.item)
        {
          case 1:
          rainbow = !rainbow;
          break;
          case 2:
          blinking = !blinking;
          break;
          case 3:
          if(currentAnimation < 2)
          {
            currentAnimation++;
          }
          else 
          {
            currentAnimation = 0;
          }
          break;
          case 4:
            returnToMainMenu();
          break;
        }
    break;
  }
  
}


//Render the menu on the OLED screen
void renderMenu(int menu, int item, int len)
{

  //Screen size  width 128 x height 64

  oled.clearDisplay(); // clear display

  oled.setTextSize(1);         // set text size
  oled.setTextColor(WHITE);    // set text color
  oled.setCursor(0, 0);       // set position to display
  oled.print("Animation: "); // set text
  oled.println(currentAnimation);


  oled.setTextSize(1);         // set text size
  oled.setTextColor(WHITE);    // set text color
  oled.setCursor(0, 10);       // set position to display
  oled.print("Menu: "); // set text
  oled.println(currentMenu.name);

  oled.drawLine(0,20,127,20,WHITE);

  item -= 1;
  int pos = 0;
  int pages = len / 4;
  int currentpage = (currentMenu.item / 4);
  char *tempname;
  char *tempoptions;


  tempoptions = (char*)malloc(strlen(currentMenu.options)+1);
  strcpy(tempoptions, currentMenu.options);

  tempname = strtok(tempoptions,"|");
  
  for(int i = 0; i < currentpage; i++)
    {
      tempname = strtok(nullptr, "|"); 
      tempname = strtok(nullptr, "|"); 
      tempname = strtok(nullptr, "|");  
    }
  
  for(int x = 0; x < 3 && x < currentMenu.len; x++)
  {
    
    oled.setTextSize(1);         // set text size
    oled.setTextColor(WHITE);    // set text color
    oled.setCursor(0, (x * 10 + 22));       // set position to display
    
    if((currentpage*3)+x == item)
    {
      oled.print(">");
    }
    
    oled.println(tempname);

    tempname = strtok(nullptr, "|");

    pos = x;
    
  }

  oled.setTextSize(1);         // set text size
  oled.setTextColor(WHITE);    // set text color
  oled.setCursor(0, (++pos * 10 + 22));       // set position to display
  
  if( currentMenu.len== item)
  {
    oled.print(">");
  }

  oled.print("Back"); // set text
  oled.display();  
  free(tempoptions);

}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient *pclient) {}

  void onDisconnect(BLEClient *pclient) {
    doConnect = true;
    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer() {
  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());

  BLEClient *pClient = BLEDevice::createClient();
  
  Serial.println(" - Created client");

  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remove BLE Server.
  pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
  Serial.println(" - Connected to server");
  pClient->setMTU(517);  //set client to request maximum MTU from server (default is 23 otherwise)

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService *pRemoteService = pClient->getService(SERVICE_UUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(SERVICE_UUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  pCharacteristicMoveButton = pRemoteService->getCharacteristic(moveButtonUUID);
  if (pCharacteristicMoveButton == nullptr) {
    Serial.print("Failed to find move characteristic UUID: ");
    Serial.println(moveButtonUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found move characteristic");


  pCharacteristicSelectButton = pRemoteService->getCharacteristic(selectButtonUUID);
  if (pCharacteristicSelectButton == nullptr) {
    Serial.print("Failed to find select characteristic UUID: ");
    Serial.println(moveButtonUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found select characteristic");


  // Read the value of the characteristic.
  if (pCharacteristicMoveButton->canRead()) {
    String value = pCharacteristicMoveButton->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());
  }

    if (pCharacteristicSelectButton->canRead()) {
    String value = pCharacteristicSelectButton->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());
  }

  if (pCharacteristicMoveButton->canNotify()) {
    pCharacteristicMoveButton->registerForNotify(notifyCallback);
  }

  if (pCharacteristicSelectButton->canNotify()) {
    pCharacteristicSelectButton->registerForNotify(notifyCallback);
  }

  connected = true;
  return true;
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(SERVICE_UUID)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    }  // Found our server
  }  // onResult
  
};  // MyAdvertisedDeviceCallbacks


void rgbMode()
{

    // Serial.print("Colour: ");
    // Serial.println(colourToChange);
    // Serial.println(rainbowcolour);
    // Serial.print("R: ");
    // Serial.print(Red);
    // Serial.print("G: ");
    // Serial.print(Green);
    // Serial.print("B: ");
    // Serial.println(Blue);
if(rainbowcolour < 10 || rainbowcolour > 245)
    {
      colourToChange = random(1,4);
      switch (colourToChange)
      {
        case 1:
          rainbowcolour = Red;
        break;
        case 2:
          rainbowcolour = Green;
        break;
        case 3:
          rainbowcolour = Blue;
        break;
      }
    }
    if(rainbowcolour > 245)
    {
      up = false;
    }
    else if(rainbowcolour < 10)
    {
      up = true;
    }

    if(up)
    {
      rainbowcolour += 5;
    }
    else
    {
      rainbowcolour -= 5;
    }
    switch (colourToChange)
    {
      case 1:
      if(up)
      {
        Red += 5;
      }
      else
      {
        Red -= 5;
      } 
      break;
      case 2:
      if(up)
      {
        Green+= 5;
      }
      else
      {
        Green-= 5;
      }
      break;
      case 3:
      if(up)
      {
        Blue+= 5;
      }
      else
      {
        Blue-= 5;
      }
      break;
      default:
      colourToChange = random(1,4);
      break;
      
    }
    strip1.clear();
    strip2.clear();
    for(int x = 0; x < HEIGHT; x++)
    {
      int row = newBlinking[0][x]; 
      for (int n = 14; n >= 0;n--)
      {
        if(row & (1<<n))
        {
          strip1.setPixelColor(getLEDIndex(n,x,true), strip1.Color(Red, Green, Blue));
          strip2.setPixelColor(getLEDIndex(n,x), strip2.Color(Red, Green, Blue));
        }
      }
    }
    strip1.show(); 
    strip2.show(); 
}

void animationstyle()
{


  switch (currentAnimation)
 {

  case 0:
    diagonalChangeRender(newBlinking[0], newColour.substring(0,3).toInt(), newColour.substring(4,8).toInt(), newColour.substring(8,12).toInt()); 
  break;
  case 1:
    Red = newColour.substring(0,3).toInt();
    Green = newColour.substring(4,8).toInt();
    Blue = newColour.substring(8,12).toInt();
    lineByLineRender(newBlinking[0]);
  break;
  case 2:
    Red = newColour.substring(0,3).toInt();
    Green = newColour.substring(4,8).toInt();
    Blue = newColour.substring(8,12).toInt();
    straightLineRender(newBlinking[0]);
  break;
  case 3:

  break;
 }
}

void setup() {
  
  currentMenu.menu = 0;
  currentMenu.item = 1;
  currentMenu.len = 3;
    
  currentMenu.name = (char*)malloc(strlen("Main")+1);
  strcpy(currentMenu.name, "Main");
    
  currentMenu.options = (char*)malloc(strlen("Animations|Colours|Modes")+1);
  strcpy(currentMenu.options, "Animations|Colours|Modes");
  
  
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


  // Create the BLE Device

  BLEDevice::init("ProotBrain");
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
  
loadingAnimRender();

displayText("Service-Top here. What can I do for you? :3");
diagonalStartRender();

delay(2000);




//lineByLineAnimation(newBlinking);
//straightLineAnimation(newBlinking);
//diagonalChange(boyk, 163,6,163);


}

void loop(){
  
  if (doConnect == true) 
  {
    if (connectToServer()) 
    {
      Serial.println("We are now connected to the BLE Server.");
      doConnect = false;
    } 
    else 
    {
      Serial.println("We have failed to connect to the server; there is nothing more we will do.");
    }

  }


  if(rainbow)
  {
      rgbMode();

    
  }

  if (connected) {

    if(pCharacteristicMoveButton->readValue() == "Down")
    {
      if(currentMenu.item <= currentMenu.len)
      {
        currentMenu.item++;
      }
      else
      {
        currentMenu.item=1;
      }
      
      pCharacteristicMoveButton->writeValue("Waiting");
      Serial.println(pCharacteristicMoveButton->readValue());
    }


    if(pCharacteristicSelectButton->readValue() == "Select")
    {
      Serial.println(pCharacteristicSelectButton->readValue());
      moveToMenu();
      pCharacteristicSelectButton->writeValue("Waiting");
    }
  } 

//Serial.printf("Free heap: %u bytes\n", esp_get_free_heap_size());
  
  
  renderMenu(currentMenu.menu,  currentMenu.item, currentMenu.len);

  
  if(newColour != oldColour)
  {
    animationstyle();
    oldColour = newColour;
  }
  delay(10);
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
    //Serial.println("");

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
    renderFrame(newBlinking[0]);
    delay(50);
    renderFrame(newBlinking[1]);
    delay(50);
    renderFrame(newBlinking[2]);
    delay(100);
    renderFrame(newBlinking[1]);
    delay(100);
    renderFrame(newBlinking[0]);
 
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
