# Sci-Fi-LED-Helmet

## What's this?
A futuristic helmet with LED matricies that I'm building and planning to use for future cosplay events. It can display simple animations, dynamic/static text, and facial expressions. The head is controlled remotely via a hand controller.

## What I used
* ESP32 x2
* WS2812b x2 matricies (32\*8). Later repalced with HUB75 x2 matricies(64\*32). 
* 0.96 inch OLED screen
* Buttons x2
* 5V fan
* 12V to 5V converter
* 18650 Batteries x4

## How it works?
Both LEDs matricies are connected to the ESP32 inside the head. Available animations are displayed on a small OLED screen at the front of the head and can be selected via the controller. A small fan is installed at bottom to keep fresh air coming in. The whole system is powered by 3 18650 batteries (12v~ stepped down to 5v).

The controller consists of an ESP32, 2 buttons (One to scroll and one to select) and an 1 18650 battery to power everything. 

The head and the controller use BLE to communicate between each other. 

## Pictures!
The first few pictures are of the outdated design with WS2818b, on the ones after you will see HUB75 LEDs.

/////////////OUTDATED/////////////
![20250105_225237](https://github.com/user-attachments/assets/c0d33c18-fe95-4ea5-a085-f643a006db75)


![20250208_232330](https://github.com/user-attachments/assets/9d76efbe-9ee2-4e48-86ea-7912e97a7b2f)

<img width="698" height="473" alt="image" src="https://github.com/user-attachments/assets/25c6a709-dd5c-4265-b191-a06187299631" />
<img width="460" height="451" alt="image" src="https://github.com/user-attachments/assets/34656643-7d6c-4a10-8b88-0c877d5a78f2" />

/////////////OUTDATED/////////////
<img width="960" height="1280" alt="image" src="https://github.com/user-attachments/assets/a58d44c6-bd6a-400c-88fb-f7101879abce" />
<img width="960" height="1280" alt="image" src="https://github.com/user-attachments/assets/17f4ad8a-70e7-41ad-a3df-4c25ab798044" />
<img width="918" height="683" alt="image" src="https://github.com/user-attachments/assets/dcfe3141-107a-428c-abae-4ce16cd4dc0e" />
<img width="842" height="657" alt="image" src="https://github.com/user-attachments/assets/04936ad0-75cc-482a-90b6-230196754f05" />
<img width="537" height="661" alt="image" src="https://github.com/user-attachments/assets/b6de824c-7a0d-4844-bccf-f7a16017451f" />

