
#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0x90,0xA2,0xDA,0x0F,0x16,0xF2  }; //physical mac address
IPAddress ip(192,168,1,34); // ip in lan
//IPAddress gateway(192,168,1,1); // internet access via router
//IPAddress subnet(255,255,255,0); //subnet mask
IPAddress myserver(192,168,1,61); 
EthernetServer server(80); //server port
EthernetClient client;
//String readString;
void setup(){

  //pinMode(4, OUTPUT); //pin selected to control
  Ethernet.begin(mac, ip); 
  server.begin();
  Serial.begin(9600); 
   Serial.println(Ethernet.localIP());

}

void loop(){
  // check for serial input
  if (Serial.available() > 0) 
  {
    byte inChar;
    inChar = Serial.read();
    if(inChar == 'u')
    {
      sendGET(1); // call sendGET function
    }
    if(inChar == 'd')
    {
      sendGET(0); // call sendGET function
    }
  }  

  EthernetClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        char c;
        String test=String(""); 
        while(1)
        {
          c = client.read();
          if(c==-1)
          {
            break;
          }
          else
          {
            test+=c;
          }
         // Serial.print(c);
          
        }
      //U CAN RESPOND TO THE CLIENT ACCORING TO THE REQUEST BY STRING PARSING
        Serial.print(test);
          delay(1);
          //stopping client
          client.stop();
          //clearing string for next read
          test="";

        }
      }
    }
  }

void sendGET(int movement) //client function to send/receie GET request data.
{
 
  if (client.connect(myserver,80)) {
    Serial.println("connected");
    if(movement==1)
     client.println("hey move up");
    else if(movement==0)
      client.println("hey move down");
  } 
  else {
    Serial.println("connection failed");
    Serial.println();
  }
  delay(10);
 if (client.available()) {
   char c;
        String test=String(""); 
        while(1)
        {
          c = client.read();
          if(c==-1)
          {
            break;
          }
          else
          {
            test+=c;
          }
         // Serial.print(c);
          
        }
        Serial.println(test);
  }
  
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    

  }
  
}
