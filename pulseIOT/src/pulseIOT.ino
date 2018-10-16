
/*
The code is based on code from the following repository:
https://github.com/pkourany/PulseSensor_Spark.git

I am also using a standard library PulseSensor_Spark v. 1.5.4.
PulseSensor_Spark.cpp and PulseSensor_Spark.h is found at the same repository

Pulse Sensor sample aquisition and processing happens in the background via a
hardware Timer interrupt at 2mS sample rate.
On the Photon, TIMR3 is allocated and has no affect on the A2 pin.

The following variables are automatically updated:
rawSignal : int that holds the analog signal data straight from the sensor.
updated every 2mS.

IBI  :      int that holds the time interval between beats. 2mS resolution.
BPM  :      int that holds the heart rate value, derived every beat,
from averaging previous 10 IBI values.

QS  :       boolean that is made true whenever Pulse is found and BPM
is updated. User must reset.

Pulse :     boolean that is true when a heartbeat is sensed then false in time
with LED going out.

Pin D7 LED (onboard LED) will blink with heartbeat.
It will also fade an LED on pin fadePin with every beat.

Check here for detailed code walkthrough:
http://pulsesensor.myshopify.com/pages/pulse-sensor-amped-arduino-v1dot1

The SparkIntervalTimer library can be found here:
https://github.com/pkourany/Manchester_Library.git

Source file: pulseIOT.into
Author: Tobias Valbjørn, Joel Murphy, Yury Gitman
Date: 12-Oct-2018
Version: 1.9
*/

#include <SparkIntervalTimer.h>

extern void interruptSetup(void);
//Set to onboard LED D7
extern int blinkPin;
extern int pulsePin;

extern volatile int BPM;;
extern volatile int rawSignal;;
extern volatile int IBI;

extern volatile boolean Pulse;
extern volatile boolean QS;

//Pin that can be used to do fading with LED.
//It is set to D6
extern int fadePin;
extern int fadeRate;

int lastBPM=0;
int consecBeat=0;
int threshold=10;
int accBeats=0;
int numBeat=0;
int avgBPM=0;
unsigned long int timerDone=0;
boolean waitingResponse=false;
void setup(){
	//Default fade pin is D6, but D0-D3 is PWM on particle photon.
	fadePin=D3;

	// pin that will blink to your heartbeat!
	pinMode(blinkPin,OUTPUT);

	// pin that will fade to your heartbeat
	pinMode(fadePin,OUTPUT);

	Serial.begin(115200);

	// sets up to read Pulse Sensor signal every 2mS.
	//Find IBI and BPM. Sets QS to true when HB is detected.
	interruptSetup();

	//subscribe to the specific event name to receive the response
	//from the web server and use it in the firmware logic.
	Particle.subscribe("hook-response/bpm", myHandler, MY_DEVICES);
}

void loop(){
	// BPM and IBI have been Determined
	// Quantified Self "QS" true when arduino finds a heartbeat
	if (QS == true){

		//I want to avoid measuring big shifts in the heartbeats.
		//I want to measure the heartbeat when the resting heart rate has settled.
		if(abs(BPM-lastBPM) < 5)
		{
			consecBeat++;
			//It can take some time to find the pulse. Sometimes a lucky few
			//heartbeats can increase the consecBeat, but when consecBeat
			//has reached the threshold, there is a high chance that the pulse happens
			//been found and is steady, ready to meassure.
			if(consecBeat==threshold){
				//Serial.println("Success! consecBeat is:");
				//Serial.println(consecBeat);
				avgBPM=BPM;
				timerDone=millis()+20000;
				//Serial.println("Timer set");
			}
			//we will only do the measurements when we are above the threshold
			//for consequtive heartbeats, this will increase accuracy of data.
			if(consecBeat>threshold){
				numBeat++;
				//accumulated beats for averaging
				accBeats+=BPM;
				avgBPM=accBeats/numBeat;
			}
		}
		else{
			//If you have found your pulse but lost it again, irregular beats can
			//happen when you try to find it again. It will help avoid corrupting data
			consecBeat--;
		}

		// Blink LED, we got a beat.
		digitalWrite(blinkPin,HIGH);

		// Set 'fadeRate' Variable to 255 to fade LED with pulse
		fadeRate = 255;

		//Output beat to serial.
		//serialOutputWhenBeatHappens();

		// reset the Quantified Self flag for next time
		QS = false;
	}
	else {
		// There is not beat
		digitalWrite(blinkPin,LOW);
	}

	ledFadeToBeat();
	lastBPM=BPM;

	if(millis()>=timerDone && consecBeat>=threshold && waitingResponse==false)
	{
		//We don't want this function to be called, before there is a
		waitingResponse=true;
		sendToCloud();
		//Serial.println("Data send to cloud");
	}
	delay(20);
}

//  Decides How To OutPut BPM and IBI Data
void serialOutputWhenBeatHappens(){
		Serial.print("*** Heart-Beat Happened *** ");
		Serial.print("BPM: ");
		Serial.print(BPM);
		Serial.println("  ");
}
void ledFadeToBeat(){
	 //  set LED fade value
	fadeRate -= 15;
	//  keep LED fade value from going into negative numbers!
	fadeRate = constrain(fadeRate,0,255);
 	//  fade LED
	analogWrite(fadePin,fadeRate);
}

void sendToCloud() {
	//Serial.println("Sending number of beats to cloud:");
	//Serial.println(avgBPM);
	String data=String(avgBPM);
	bool success=false;
	while(!success){
			success=Particle.publish("bpm", data, PRIVATE);
		}
}


void myHandler(const char *event, const char *data) {
/*
	Serial.println("Im in!");
	Serial.print("event: ");
	Serial.println(event);
	Serial.print("Time: ");
	Serial.println(millis());
	Serial.print("Data: ");
	Serial.println(data);
*/
//event and data are C-strings. when comparing with "hook-response/bpm/0",
//a string object will be created. event=="hook-response/bpm/0" would
//never be true. Therefore strcmp has to be used.
 if(strcmp(event,"hook-response/bpm/0")==0 && strcmp(data,"0")){
	/*Serial.println("inside if statement");
	Serial.print("event: ");
	Serial.println(event);
	Serial.print("Time: ");
	Serial.println(millis());
	Serial.print("Data: ");
	Serial.println(data);
	*/
	//Indicate to the user, that the test is over.
	timerDone=millis()+10000;
	while(millis()<=timerDone){digitalWrite(fadePin, HIGH);}
	//Calculate the next time that data needs to be sent to cloud
	timerDone=millis()+10000;
	waitingResponse=false;
 }
}
