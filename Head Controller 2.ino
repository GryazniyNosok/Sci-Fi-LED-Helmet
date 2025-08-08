#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Adafruit_SSD1306.h>

#include <Adafruit_NeoPixel.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <BLEAdvertisedDevice.h>
// # Xbm Bitmap example
// ## Requirements
// * To generate the required Xbm data to be copied into the Sketch. Have python and [paint.net](https://www.getpaint.net/) installed.
// * Bitmap should match the resolution of your display configuration.

// ## Instructions 
//  1. SAVE BITMAP AS 1BIT COLOUR in paint.net 
//  1. Run: bmp2hex.py -i -x <BITMAP> (e.g. "bmp2hex.py -i -x WiFi1bit.bmp")
//  1. Copy paste output into sketch.
 
//  ![bmp2hex usage screenshot](screenshot.jpg)
 
//sudo chmod a+rw /dev/ttyUSB0

/*--------------------- RGB DISPLAY PINS -------------------------*/
#define R1_PIN 25
#define G1_PIN 26
#define B1_PIN 27
#define R2_PIN 14
#define G2_PIN 12
#define B2_PIN 13
#define A_PIN 23
#define B_PIN 19 // Changed from library default
#define C_PIN 5
#define D_PIN 17
#define E_PIN -1
#define LAT_PIN 4
#define OE_PIN 15
#define CLK_PIN 16

/*--------------------- MATRIX LILBRARY CONFIG -------------------------*/
#define PANEL_RES_X 64            // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 32            // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 2             // Total number of panels chained one to another
#define PANEL_ONE_START_POINT 0    
#define PANEL_TWO_START_POINT 64  


#define SCREEN_WIDTH          128 
#define SCREEN_HEIGHT         64 


MatrixPanel_I2S_DMA *dma_display = nullptr;
int offsetclock = 0;
int offsetarray[64];
int monooffset[64];
int col = 7;

uint16_t new_colour = dma_display->color565(136, 0, 204);
uint16_t old_colour;

bool is_glitchy = false;


uint16_t rgb_pattern[8] = {dma_display->color565(255, 0, 0),dma_display->color565(0, 255, 0),dma_display->color565(0, 0, 255),dma_display->color565(255, 0, 0),dma_display->color565(0, 255, 0),dma_display->color565(0, 0, 255),dma_display->color565(255, 0, 0),dma_display->color565(0, 255, 0)};
// Module configuration
HUB75_I2S_CFG mxconfig(
	PANEL_RES_X,   // module width
	PANEL_RES_Y,   // module height
	PANEL_CHAIN    // Chain length
);

//Structure for current menu
struct SelectedMenu{
  int menu;
  int item;
  char* name;
  char* options;
  int len;
};

SelectedMenu currentMenu;

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

static char default_smile[] = {
0xff, 0x1f, 0x00, 0x00, 0x00, 0xf8, 0xf8, 0xf8, 0xff, 0x07, 0x00, 0x00, 0x00, 0x7c, 0x7c, 0x7c,
0xff, 0x01, 0x00, 0x00, 0x00, 0x3e, 0x3e, 0x3e, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x1f, 0x1f,
0x1f, 0x00, 0x00, 0x00, 0x80, 0x8f, 0x8f, 0x0f, 0x07, 0x00, 0x00, 0x00, 0xc0, 0xc7, 0xc7, 0x07,
0x01, 0x00, 0x00, 0x00, 0xe0, 0xe3, 0xe3, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xff,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x3f,
0x00, 0x7e, 0x00, 0x00, 0x00, 0x80, 0xff, 0x01, 0xc0, 0xff, 0x03, 0x00, 0x00, 0xe0, 0x3f, 0x00,
0xf0, 0xff, 0x07, 0x00, 0x00, 0xfc, 0x0f, 0x00, 0xf8, 0xff, 0x0f, 0x00, 0x00, 0xff, 0x03, 0x00,
0xfc, 0x81, 0x1f, 0x00, 0xe0, 0x7f, 0x00, 0x00, 0x7e, 0x00, 0x3f, 0x00, 0xfe, 0x1f, 0x00, 0x00,
0x3f, 0x00, 0xfe, 0xc0, 0xff, 0x07, 0x00, 0x00, 0x1f, 0x00, 0xfc, 0xf9, 0x7f, 0x00, 0x00, 0x00,
0x0f, 0x00, 0xf8, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x07, 0x00, 0xe0, 0xff, 0x01, 0x00, 0x00, 0x00,
0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
,};


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


class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient *pclient) {}

  void onDisconnect(BLEClient *pclient) {
    doConnect = true;
    connected = false;
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setInterval(1349); 
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(5, false);
    Serial.println("onDisconnect");
  }
};


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



//Render the menu on the OLED screen
void renderMenu(int menu, int item, int len)
{

  //Screen size  width 128 x height 64

  oled.clearDisplay(); // clear display

  oled.setTextSize(1);         // set text size
  oled.setTextColor(WHITE);    // set text color
  oled.setCursor(0, 0);       // set position to display
  oled.print("Memory: ");
  oled.println(esp_get_free_heap_size());
  //oled.print("Animation: "); // set text
  //oled.println(currentAnimation);


  oled.setTextSize(1);         // set text size
  oled.setTextColor(WHITE);    // set text color
  oled.setCursor(0, 10);       // set position to display
  oled.print("Menu: "); // set text
  oled.println(currentMenu.name);
  

  oled.drawLine(0,20,127,20,WHITE);

  item -= 1; //-1 because items need to start from 0
  int pos = 0;
  int pages = len / 4;
  int currentpage = (item / 3);
  char *tempname;
  char *tempoptions;


  tempoptions = (char*)malloc(strlen(currentMenu.options)+1);
  strcpy(tempoptions, currentMenu.options);

  tempname = strtok(tempoptions,"|");
  
  //Skip the first 3*currentpage options. 
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
    
    if((currentpage*3)+x == item && item != currentMenu.len)
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
  
  if(currentMenu.len == item) //Must be current length +1
  {
    oled.print(">");
  }

  oled.print("Back"); // set text
  oled.display();  
  free(tempoptions);

}

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


bool pixel_render(int x, int y, int imgWidth, int imgHeight, const char* xbm, int drawX, int drawY, uint16_t color = 0xFFFF, bool flipH = false, bool flipV = false) {
  if (x < 0 || x >= imgWidth || y < 0 || y >= imgHeight)
    return false; // out of bounds

  // Flip coordinates if needed
  int fx = flipH ? (imgWidth - 1 - x) : x;
  int fy = flipV ? (imgHeight - 1 - y) : y;

  // Width must be byte-aligned (XBM rule)
  int byteWidth = (imgWidth + 7) / 8;

  // Calculate byte index
  int byteIndex = fy * byteWidth + (fx / 8);
  uint8_t byteValue = pgm_read_byte(xbm + byteIndex);

  // Check bit in byte (0 is LSB, 7 is MSB)
  bool isSet = bitRead(byteValue, fx % 8);

  if (isSet) {
    dma_display->drawPixel(drawX + x, drawY + y, color);
    delay(1.5);
  }

  return isSet;
}


void simple_RGB_render(int x, int y, int width, int height, const char *xbm, uint16_t color = 0xffff, bool flipH = false, bool flipV = false, bool glitch_effect = false) 
{

  if (width % 8 != 0) {
      width =  ((width / 8) + 1) * 8;
  }

  for (int i = 0; i < width * height / 8; i++ ) {
    unsigned char charColumn = pgm_read_byte(xbm + i);
    for (int j = 0; j < 8; j++) {
      int origX = (i * 8 + j) % width;
      int origY = (8 * i / width);

      int targetX = flipH ? (width - 1 - origX) : origX;
      int targetY = flipV ? (height - 1 - origY) : origY;

      if (bitRead(charColumn, j)) {
        if(glitch_effect)
        {
          dma_display->drawPixel(x + targetX, y + targetY+ offsetarray[targetX], color); //For offset lag thing
        }
        else
        {
          dma_display->drawPixel(x + targetX, y + targetY, color);
          //dma_display->drawPixel(x + targetX, y + targetY, rgb_pattern[col]); //RAINBOW SHIT
        }
      }
        col--;
        if(col <= 0)
        {
          //Serial.println(col);
            col = 7;
        }
    }
  }
}




void glitchy_animation()
{
  if (offsetclock == 0)
  {
    //Serial.println("New array generated");
    for(int i = 0; i < 64; i++)
    {
      offsetarray[i] = rand()%4+1;
    } 
  } else if(offsetclock == 4) 
  {
    offsetclock = -1;
  }



  for(int i = 0; i < 64; i++)
  {
    if(offsetarray[i] > 0)
    {
      monooffset[i] = 1;
      offsetarray[i]--;
    }
    else
    {
      monooffset[i] = 0; 
    }

  } 
  dma_display->clearScreen();
  simple_RGB_render(64,0, 64, 32, default_smile, old_colour, false ,false ,true);
  simple_RGB_render(0,0, 64, 32, default_smile,old_colour, true, false ,true);
  delay(random(0,40));
}

void top_down_animation(uint16_t colour)
{
    for (int current_Pixel_Y = 0; current_Pixel_Y < PANEL_RES_Y; current_Pixel_Y++) 
  {
    for (int current_Pixel_X = 0; current_Pixel_X < PANEL_RES_X; current_Pixel_X++) 
    {
      pixel_render(current_Pixel_X, current_Pixel_Y, PANEL_RES_X, PANEL_RES_Y, default_smile, 0, 0, colour, true);
      pixel_render(current_Pixel_X, current_Pixel_Y, PANEL_RES_X, PANEL_RES_Y, default_smile, 64, 0, colour);
    }
  }

  delay(500);
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
        currentMenu.len = 8;
        
        free(currentMenu.name);
        free(currentMenu.options);

        currentMenu.name = (char*)malloc(strlen("Animations")+1);
        strcpy(currentMenu.name, "Animations");
    
        currentMenu.options = (char*)malloc(strlen("Blink|Smile|Eye|NotCute|Cross|Hearts|Lucian|Nuzzles")+1);
        strcpy(currentMenu.options, "Blink|Smile|Eye|NotCute|Cross|Hearts|Lucian|Nuzzles");
        break;
        
        case 2:
        currentMenu.menu = 2;
        currentMenu.item = 1;
        currentMenu.len = 8;

        free(currentMenu.name);
        free(currentMenu.options);

        currentMenu.name = (char*)malloc(strlen("Colours")+1);
        strcpy(currentMenu.name, "Colours");
    
        currentMenu.options = (char*)malloc(strlen("Purple|DimPink|Orange|Yellow|Red|Blue|Green|White")+1);
        strcpy(currentMenu.options, "Purple|DimPink|Orange|Yellow|Red|Blue|Green|White");

        break;
        case 3:
        currentMenu.menu = 3;
        currentMenu.item = 1;
        currentMenu.len = 7;

        free(currentMenu.name);
        free(currentMenu.options);

        currentMenu.name = (char*)malloc(strlen("Modes")+1);
        strcpy(currentMenu.name, "Modes");
    
        currentMenu.options = (char*)malloc(strlen("Glitch|Rainbow|Blinking|Transition|Dancin|Caramel|AFK")+1);
        strcpy(currentMenu.options, "Glitch|Rainbow|Blinking|Transition|Dancin|Caramel|AFK");

        break;
      }
    break;
    case 1:
      switch(currentMenu.item) //Blink|Smile|Eye|NotCute|Cross|Hearts|Lucian|Nuzzles
      {
        case 1:
          //runAnimation(0);
        break;

        case 2:
          //runAnimation(1);
        break;
        case 3:
          //runAnimation(2);
        break;
        case 4:
          //displayText("Not Cute >:(");
          //renderFrame(newBlinking[0]);
        break;
        case 5: //Crosses
          //renderPartical(cross);
        break;
        case 6: //Hearts
          //renderPartical(heart);
        break;
        case 7: 
          //Green = 0;
          //Red = 255;
          //Blue = 0;
          //displayText("Happy Bday Lucian");
        
          //runAnimation(2);
          //straightLineRender(newBlinking[0]);
        break;
        case 8:
          //displayText("<3 Nuzzles <3");
          //straightLineRender(newBlinking[0]);
        break; 
        case 9:
          returnToMainMenu();
        break;
      }
    break;
    case 2:
      switch(currentMenu.item) //Purple|DimPink|Orange|Yellow|Red|Blue|Green|White
      { 
        case 1: //ADD LIGHT, WHITE AND DIM PINK
              
                new_colour = dma_display->color565(136, 0, 204);
        break;
        case 2: 
                new_colour = dma_display->color565(61, 0, 163);
        break;
        case 3:
                new_colour = dma_display->color565(255, 40, 0);//Too green
        break;
        case 4:
                new_colour = dma_display->color565(255, 255, 6);
        break;
        case 5:
                new_colour = dma_display->color565(255, 0, 0);
        break;
        case 6:
                new_colour = dma_display->color565(0, 0, 255);
        break;
        case 7:
                new_colour = dma_display->color565(0, 255, 0);
        break;
        case 8:
                new_colour = dma_display->color565(255, 255, 255);
        break;
        case 9:
          returnToMainMenu();
        break;
      }
    break;
    case 3:
        switch (currentMenu.item) //Glitch|Rainbow|Blinking|Transition|Dancin|Caramel|AFK
        {
          case 1:
            is_glitchy = !is_glitchy;
            if(!is_glitchy)
            {
              dma_display->clearScreen();
              simple_RGB_render(64,0, 64, 32, default_smile, old_colour);
              simple_RGB_render(0,0, 64, 32, default_smile ,old_colour, true);
            }
          break;
          case 2:
         // is_rainbow = !is_rainbow;
          break;
          case 3:
         // is_blinking = !is_blinking;
          break;
          case 4:
          // if(currentAnimation < 2)
          // {
          //   currentAnimation++;
          // }
          // else 
          // {
          //   currentAnimation = 0;
          // }
          break;
          case 5:
           // is_dancinMode = !is_dancinMode;
            // if(is_dancinMode)
            // {
            //   Red = 0;
            //   Green = 255;
            //   Blue = 0;
            //   strip1.setBrightness(255);
            //   strip2.setBrightness(255);
            // }
            // else
            // {
            //   strip1.setBrightness(50);
            //   strip2.setBrightness(50);
            //   renderFrame(newBlinking[0]);
            // }
          break;
          case 6:
            // is_caramelldansen = !is_caramelldansen;
            // if(is_caramelldansen)
            // {
            //   strip1.setBrightness(255);
            //   strip2.setBrightness(255);
            // }
            // else
            // {
            //   strip1.setBrightness(50);
            //   strip2.setBrightness(50);
            //   renderFrame(newBlinking[0]);
            // }
          break;
          case 7:
           // displayStaticText("AFK");
            //is_blinking = false;
          break;
          case 8:
            returnToMainMenu();
          break;
        }
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
  
  // put your setup code here, to run once:
  delay(1000); Serial.begin(115200); delay(200);

  BLEDevice::init("ProotBrain");
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);

  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("failed to start SSD1306 OLED"));
    while (1);
  }
  /************** DISPLAY **************/
  Serial.println("...Starting Display");
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(90); //0-255
  dma_display->clearScreen();

  delay(2000);

}


void loop() {

  if (doConnect) 
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

  renderMenu(currentMenu.menu,  currentMenu.item, currentMenu.len);
  
  //dma_display->clearScreen();

  if(is_glitchy)
  {
    //Serial.println("Works?");
    glitchy_animation();
  }

  if(new_colour != old_colour)
  {
     top_down_animation(new_colour);
     old_colour = new_colour;
     Serial.println("should run once");
  }

}
