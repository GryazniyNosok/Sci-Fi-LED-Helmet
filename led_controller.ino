#include <Adafruit_NeoPixel.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>


#define PIN1        4        // Pin where NeoPixels are connected. Was 6. 
#define PIN2        5        //Works for ESP32
#define WIDTH      15       // Width of the LED matrix 32
#define HEIGHT     8        // Height of the LED matrix
#define NUMPIXELS_PER_STRIP  200 // Total number of LEDs

Adafruit_NeoPixel strip1(NUMPIXELS_PER_STRIP, PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(NUMPIXELS_PER_STRIP, PIN2, NEO_GRB + NEO_KHZ800);


int Red = 163;
int Green = 6;
int Blue = 163;

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;
String oldcolour;


#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};


// byte const unsigned long newBlinking[8] =
// {

//    0x3807, //11100000000111
//     0x1C03, //01110000000011
//     0x000,  //00000000000000
//     0x000,  //00000000000000
//     0x800, //00100000000000
//     0x1444, //01010001000100
//     0x2AA,  //00001010101010
//     0x111   //00000100010001

// };

//int diagonalarray = {}
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


// byte const unsigned long newBlinking[8] =
// {
// 0x3C0F, //11110000001111
//     0x1E07, //01111000000111
//     0xC00,  //00110000000000
//     0x000,  //00000000000000
//     0x183,  //00000110000001
//     0x7E7,  //00011111100111
//     0xE7E,  //00111001111110
//     0x018   //00000000011000
// };

//  0x3C0F, //11110000001111
//     0x1E07, //01111000000111
//     0xC00,  //00110000000000
//     0x000,  //00000000000000
//     0x000,  //00000000000000
//     0x4C3,  //00010011000011
//     0x336,  //00001100110110
//     0x01C   //00000000011100


//  0x3807, //11100000000111
//     0x1C03, //01110000000011
//     0x000,  //00000000000000
//     0x000,  //00000000000000
//     0x000, //00000000000000
//     0xA88,  //00101010001000
//     0x555,  //00010101010101
//     0x22    //00000000100010



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

// void renderPixels(int x[8], int y[8], int activeLEDs, int onOrOff[8])
// {
//   for(int n = 0; n < activeLEDs; n++)
//   {
//     strip1.setPixelColor(getLEDIndex(x[n],y[n], true), strip1.Color(163,6,163));
//   }

//   strip1.show();
//   delay(40);

//   for(int n = 0; n < activeLEDs; n++)
//   {
//     if(!onOrOff[n])
//     {
//       strip1.setPixelColor(getLEDIndex(x[n],y[n], true), strip1.Color(0,0,0));
//     }
//   }

// }
    


void setup() {
  Serial.begin(9600);
  strip1.begin();
  strip2.begin();
  strip1.show();  
  strip2.show();
  delay(1000);

  //renderFrame(newBlinking);
  //lineByLineAnimation(newBlinking);
  //straightLineAnimation(newBlinking);
    // Create the BLE Device
  BLEDevice::init("ESP32");

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
delay(2000);
}

void loop() {

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
  delay(1000);
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
  // delay(3000);

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



