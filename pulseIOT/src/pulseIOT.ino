
/*
>> Pulse Sensor Amped 1.4 <<
This code is for Pulse Sensor Amped by Joel Murphy and Yury Gitman
	www.pulsesensor.com
	>>> Pulse Sensor purple wire goes to Analog Pin A2 (see PulseSensor_Spark.h for details) <<<

Pulse Sensor sample aquisition and processing happens in the background via a hardware Timer interrupt at 2mS sample rate.
On the Core, PWM on selectable pins A0 and A1 will not work when using this code, because the first allocated timer is TIMR2!
On the Photon, TIMR3 is allocated and has no affect on the A2 pin.

The following variables are automatically updated:
rawSignal : int that holds the analog signal data straight from the sensor. updated every 2mS.
IBI  :      int that holds the time interval between beats. 2mS resolution.
BPM  :      int that holds the heart rate value, derived every beat, from averaging previous 10 IBI values.
QS  :       boolean that is made true whenever Pulse is found and BPM is updated. User must reset.
Pulse :     boolean that is true when a heartbeat is sensed then false in time with pin13 LED going out.

This code is designed with output serial data to Processing sketch "PulseSensorAmped_Processing-xx"
The Processing sketch is a simple data visualizer.
All the work to find the heartbeat and determine the heartrate happens in the code below.
Pin D7 LED (onboard LED) will blink with heartbeat.
If you want to use pin D7 for something else, specifiy different pin in PulseSensor_Spark.h
It will also fade an LED on pin fadePin with every beat. Put an LED and series resistor from fadePin to GND.
Check here for detailed code walkthrough:
http://pulsesensor.myshopify.com/pages/pulse-sensor-amped-arduino-v1dot1

Code Version 1.2 by Joel Murphy & Yury Gitman  Spring 2013
This update fixes the firstBeat and secondBeat flag usage so that realistic BPM is reported.

>>> Adapted for Spark Core by Paul Kourany, May 2014 <<<
>>> Updated for Particle Core and Photon by Paul Kourany, Sept 2015 <<<
>>> Updated to remove (outdated) SparkInterval library code, Oct 2016 <<<

In order for this app to compile correctly, the following Partible Build (Web IDE) library MUST be attched:
 - SparkIntervalTimer

*/
#include <SparkIntervalTimer.h>

extern void interruptSetup(void);
extern int pulsePin;
//Set to onboard LED D7
extern int blinkPin;
extern volatile int BPM;;
extern volatile int rawSignal;;
extern volatile int IBI;
extern volatile boolean Pulse;
extern volatile boolean QS;

//Pin that can be used to do fading with LED.
//It is set to D6
extern int fadePin;
extern int fadeRate;

void setup(){
	pinMode(blinkPin,OUTPUT);         // pin that will blink to your heartbeat!
	pinMode(fadePin,OUTPUT);          // pin that will fade to your heartbeat!
	Serial.begin(115200);             // we agree to talk fast!
	interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS
}

//  Where the Magic Happens
void loop(){

	if (QS == true){     //  A Heartbeat Was Found
		// BPM and IBI have been Determined
		// Quantified Self "QS" true when arduino finds a heartbeat
		digitalWrite(blinkPin,HIGH);     // Blink LED, we got a beat.
		fadeRate = 255;         // Makes the LED Fade Effect Happen
		// Set 'fadeRate' Variable to 255 to fade LED with pulse
		serialOutputWhenBeatHappens();   // A Beat Happened, Output that to serial.
		QS = false;                      // reset the Quantified Self flag for next time
	}
	else {
		digitalWrite(blinkPin,LOW);            // There is not beat, turn off onboard LED
	}

	ledFadeToBeat();                      // Makes the LED Fade Effect Happen
	delay(20);                             //  take a break
}



//  Decides How To OutPut BPM and IBI Data
void serialOutputWhenBeatHappens(){
    //  Code to Make the Serial Monitor Visualizer Work
		Serial.print("*** Heart-Beat Happened *** ");  //ASCII Art Madness
		Serial.print("BPM: ");
		Serial.print(BPM);
		Serial.println("  ");
}



void ledFadeToBeat(){
	fadeRate -= 15;                         //  set LED fade value
	fadeRate = constrain(fadeRate,0,255);   //  keep LED fade value from going into negative numbers!
	analogWrite(fadePin,fadeRate);          //  fade LED
}
