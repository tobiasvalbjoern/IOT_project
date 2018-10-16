/*
 * Project hello_webhook
 * Description:
 * Author:
 * Date:
 */

 int avgBPM=65;

 void setup() {
Particle.subscribe("hook-response/bpm", myHandler, MY_DEVICES);
 Serial.begin(115200);
 }

 void loop() {
 String data=String(avgBPM);
Serial.println(data);
 Particle.publish("bpm",data,PRIVATE);
 delay(10000);
 }

  void myHandler(const char *event, const char *data) {
    Serial.println("Im in!");
    Serial.print("event: ");
    Serial.println(event);
    Serial.print("Time: ");
    Serial.println(millis());
    Serial.print("Data: ");
    Serial.println(data);

  //event and data are C-strings. when comparing with "hook-response/bpm/0",
  //a string object will be created. event=="hook-response/bpm/0" would
  //never be true. Therefore strcmp has to be used.

   if(strcmp(event,"hook-response/bpm/0")==0 && strcmp(data,"0")){
    Serial.println("inside if statement");
    Serial.print("event: ");
    Serial.println(event);
    Serial.print("Time: ");
    Serial.println(millis());
    Serial.print("Data: ");
    Serial.println(data);
   }
  }
