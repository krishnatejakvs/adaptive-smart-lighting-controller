
#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0x90,0xA2,0xDA,0x0F,0x17,0x0E  }; //physical mac address
IPAddress ip(192,168,1,61); // ip in lan
//IPAddress gateway(192,168,1,1); // internet access via router
//IPAddress subnet(255,255,255,0); //subnet mask
EthernetServer server(80); //server port


void setup(){

  //pinMode(4, OUTPUT); //pin selected to control
  Ethernet.begin(mac, ip); 
  server.begin();
  Serial.begin(9600); 
   Serial.println(Ethernet.localIP());

}
//
void loop(){
  // check for serial input  

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

        Serial.print(test);
          
          if(test.indexOf("up") >0)//checks for on
          {
            Serial.println("blind up");
            server.println("blind up");// U CAN EVEN SEND A VARIALBLE OR ANY DATA ACCORDING TO THE REQUEST
          }
          if(test.indexOf("down") >0)//checks for off
          {
            Serial.println("blind down");
            server.println("blind down");
          }
          //clearing string for next read
          test="";
          delay(1);
          //stopping client
         client.stop(); 

        }
      }
      
    }
  }
