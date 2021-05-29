#include "Arduino.h"
/*
 Example using the SparkFun HX711 breakout board with a scale
 By: Nathan Seidle
 SparkFun Electronics
 Date: November 19th, 2014
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 This example demonstrates basic scale output. See the calibration sketch to get the calibration_factor for your
 specific load cell setup.

 This example code uses bogde's excellent library:"https://github.com/bogde/HX711"
 bogde's library is released under a GNU GENERAL PUBLIC LICENSE

 The HX711 does one thing well: read load cells. The breakout board is compatible with any wheat-stone bridge
 based load cell which should allow a user to measure everything from a few grams to tens of tons.
 Arduino pin 2 -> HX711 CLK
 3 -> DAT
 5V -> VCC
 GND -> GND

 The HX711 board can be powered from 2.7V to 5V so the Arduino 5V power should be fine.

 */

#include "HX711.h"

#define calibration_factor -19000 //This value is obtained using the SparkFun_HX711_Calibration sketch

#define LOADCELL_DOUT_PIN  19
#define LOADCELL_SCK_PIN  21

HX711 scale;

//lcd defines;
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>             // Arduino SPI library

// ST7789 TFT module connections
//#define TFT_CS    10  // define chip select pin
#define TFT_CS    -1  // define chip select pin
#define TFT_DC     15  // define data/command pin
#define TFT_RST   4  // define reset pin, or set to -1 and connect to Arduino RESET pin

//check pins_arduino.h for MOSI and SCL pins- i changed them
// Initialize Adafruit ST7789 TFT library
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
int fontScale = 3;
int fontHeight = fontScale * 8;

//BLE setup includes and globals
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID        "d586aca9-12be-4e03-8ad7-b0106ba6018f"
#define CHARACTERISTIC_UUID "42b5ebfe-a946-4f85-ba49-03495b0aa5ee"

BLECharacteristic *pCharacteristic; //global for the characterisy=tic, that way i can access it in loop


float oldWeight = 0.000; // will use this variable to print over text in screen in backgound colour to blank text before next write

float newWeight = 0; //
int count = 0; //will use this for testing mqtt persistence




class MyCallbacks: public BLECharacteristicCallbacks {
	void onWrite(BLECharacteristic *pCharacteristic) {
		std::string value = pCharacteristic->getValue();
		BLEUUID gotUUID = pCharacteristic->getUUID();
		if (value.length() > 0) {
			Serial.println("*********");
			Serial.print("New value: ");
			for (int i = 0; i < value.length(); i++)
				Serial.print(value[i]);

			Serial.println();
			Serial.println("*********");
		}
		if (gotUUID.bitSize() > 0) {
			std::string valueUUID = gotUUID.toString();
			for (int i = 0; i < valueUUID.length(); i++)
				Serial.print(valueUUID[i]);

		}
	}
	void onRead(BLECharacteristic *pCharacteristic) {
	   	 Serial.println("++++++++Read from client has just happened+++++++++++++++++++");

	}
};
void connect() {

}

void messageReceived(String &topic, String &payload) {

}
void setup() {

	Serial.begin(115200);
	//setup BLE setup BLE setup BLE setup BLE setup BLE
	Serial.println("Starting BLE work!");

	BLEDevice::init("SCALESGasBottleSCALES");
	BLEServer *pServer = BLEDevice::createServer();
	BLEService *pService = pServer->createService(SERVICE_UUID);
	pCharacteristic = pService->createCharacteristic(
	CHARACTERISTIC_UUID,
			BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
	pCharacteristic->setNotifyProperty(true);
	pCharacteristic->setValue("ThisIsAPlatformForBigGasBottles");
	pCharacteristic->setCallbacks(new MyCallbacks());
	pService->start();
	BLEAdvertising *pAdvertising = pServer->getAdvertising(); // this still is working for backward compatibility
	//BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
	pAdvertising->addServiceUUID(SERVICE_UUID);
	pAdvertising->setScanResponse(true);
	pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
	pAdvertising->setMinPreferred(0x12);
	/*//set some addvertisement nice data
	 std::string manufacturerString = "ManuNM";
	 BLEAdvertisementData advertisementData;
	 advertisementData.setManufacturerData(manufacturerString);

	 pAdvertising->setAdvertisementData(advertisementData);
	 */
	pAdvertising->start();
	Serial.println(
			"Characteristic defined! Now you can read it in your phone!");
	Serial.println("HX711 scale initialise");

	scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
	scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
	scale.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0
	if (scale.is_ready()) {
		Serial.println("HX711 scale is rrrrrrrreadeeeee");
	}
	else
		Serial.println("HX711 scale is in state of;ERROR ERROR ERROR ERROR");
	//imnitialise lcd;
	Serial.print(F("Hello! ST77xx TFT Test"));

	// if the display has CS pin try with SPI_MODE0
	tft.init(240, 240, SPI_MODE2);    // Init ST7789 display 240x240 pixel

	// if the screen is flipped, remove this command
	tft.setRotation(2);

	Serial.println(F(". Initialized"));

	tft.setTextWrap(true);
	tft.fillScreen(ST77XX_BLACK);
	tft.setCursor(0, 0);
	tft.setTextColor(ST77XX_RED);
	tft.setTextSize(fontScale);
	tft.println("Weight in Kg");
}

void loop() {


	Serial.print("ReadingRaw: ");
	Serial.print(oldWeight, 3); //scale.get_units() returns a float
	Serial.print(" raw\n"); //You can change this to kg but you'll need to refactor the calibration_factor
	Serial.println("================================");
	/* Serial.println("================================");*/
	//print weight to lcd
	int cursorY = fontHeight * 2;

	tft.setTextColor(ST77XX_BLACK); //blank the last text bit(instead of blanking entire screen)
	tft.setCursor(0, cursorY + 10);
	tft.print(oldWeight); // print text in background colour to blank text (instaed of blanking entire screen)
	//oldWeight = (scale.read_average(50));
	oldWeight = (scale.read());
	tft.setTextColor(ST77XX_RED); // set text colour back to red- finished blanking old text
	tft.setCursor(0, cursorY + 10);
	tft.print(oldWeight);
	tft.setCursor(0, 0);

	tft.println("Weight in Kg");
	//count ++;

	//plotWith(Serial,  oldWeight);
	// delay(1500);
	char tempString[12];
		//itoa(count, tempString, 10);
		dtostrf( oldWeight, 6, 3, tempString);
		pCharacteristic->setValue(tempString);
			pCharacteristic->notify();
		delay(3000);
}
