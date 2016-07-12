#include <AccelStepper.h>
#include <SPI.h>
#include <Ethernet.h>
//blaktoyellow
//browntoblue
// The IP address will be dependent on your local network:
byte mac[] = { 
  0x90,0xA2,0xDA,0x0F,0x17,0x0E };
IPAddress ip(192,168,1,61);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);

/*#define RELAYS 5
long relaytime,presenttime;
volatile int relay=HIGH;
const int stableinterval=300000;
*/
int distance1=0,steps1=21492,newd=0,distance2=0,steps2;
int x=0,y=0,z=0,x1=0,y1=10,z1=10;
float t1=0.33,t2=0.45,t3=0.66;
int motorDirPin = 6; //digital pin 6
int motorStepPin = 7; //digital pin 7
AccelStepper stepper(1, motorStepPin, motorDirPin); 
int motorDirPin1 = 8; //digital pin 81
int motorStepPin1 = 9; //digital pin 9
AccelStepper stepper1(1, motorStepPin1, motorDirPin1); 
int motorSpeed = 8000;
float speed2=3000,speed1=3000,rratio,radius1=17,radius2=28;
char cmd=0;
int rounds,i,rnd2=0,current=0,flag1=1;
int clientrequest=2;

int count=0;


void setup()
{
  //pinMode(RELAYS, OUTPUT); 
     Serial.begin(9600);
     stepper1.setMaxSpeed(motorSpeed);
    stepper.setMaxSpeed(motorSpeed); 
     // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  
  
      
     
}
void loop()
{ 
  int n=18;
 /* digitalWrite(RELAYS, relay);
  presenttime=millis();
  if((presenttime-relaytime)>stableinterval)
  {
    relay=LOW;
    digitalWrite(RELAYS, relay);
    Serial.println("not stable");
  }*/
  
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
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
            int x= count+1;
            if(x>20)
              x=20;
            server.println(x);
            clientrequest=1;
          }
          if(test.indexOf("down") >0)//checks for off
          {
            Serial.println("blind down");
                int x= count-1;
            if(x<0)
              x=0;
            server.println(0);
            server.println(x);
            clientrequest=0;
          }
          //clearing string for next read
          test="";
    // give the web browser time to receive the data
  }
   delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
   }
   }
 //myStringCount = String(1);
  if (clientrequest==0 || clientrequest==1) 
  {
    //Serial.println("entered");
   // relaytime=millis();
    //relay=HIGH;
    if(flag1==1)
    {
      
    if(clientrequest==0 && count>0)
    {
     
      rounds=0;
      count--;
    }
    else if(clientrequest==1)
    {
      rounds=1;
      count++;

    }
    Serial.print(count);
    
    
    
    //Serial.println(rounds);
    //radius1=21492+ x*100+y*200+z*300; 
    if(rounds==1)
    {
      steps2=(int)21492*radius1/radius2;
      distance1=steps1;
      distance2=(steps2);
      flag1=0;
    }
    else if(rounds==0)
    {
      steps2=(int)21492*radius1/radius2;
      distance1=-(steps1);  
      distance2=-(steps2);
      flag1=0;
    }


     if(x<7 && rounds==1)
    {
      x++;
     //Serial.println(x);

      radius1+=t1;
      //Serial.println(x);
    }
    else if(x<7 && x>0 && rounds==0)
    {
      x--;
    // Serial.println(x);

      radius1-=t1;
    }
    else if(x==7 && y<10 && rounds==1)
    {
       y++;
      // Serial.println(y);
      radius1+=t2;
     //Serial.println(y);

    }
    else if(x==7 && y<10 &&  y>0 &&  rounds==0)
    {
      y--;
     // Serial.println(y);
      radius1-=t2;
    }
    else if(x==7 && y==10 && z<10 && rounds==1)
    {
      z++;
      radius1+=t3;
    }
    else if(x==7 && y==10 && z<10 &&  z>0 && rounds==0)
    {
      z--;
      radius1-=t3;
    }
    clientrequest=2;
   
    rratio=(float)steps2/steps1; 
    speed2= (3000*rratio);
    stepper1.move(distance1);
    stepper1.setSpeed(speed1);
    
    //flag1=0;
    stepper.move(distance2);
    stepper.setSpeed(speed2);
    
  }
  }  
  
 if(rounds==1)
  {
    if(stepper.currentPosition()==current)
    {
      
       if (x1==0 && y1<=10 && y1>0)
       {
         y1--;
         radius2-=t2;
       }
       else if (x1==0 && y1==0 && z1<=10 && z1>0)
       {
         z1--;
         radius2-=t3;
       }
    }
  }
  else if(rounds==0)
  {
    
    if(stepper.currentPosition()==(21492-current))
    {
      
      if(z1==10 && y1<10)
      {
        y1++;
        radius2+=t2;
      }
      else if(z1<10)
      {
        z1++;
        radius2+=t3;
      }
     }
  }
  if(stepper.distanceToGo()==0 && rounds==1 && flag1==0)
  {
    if(steps2<=current)
    {
      current=current-steps2;
       //Serial.println(current);
      flag1=1;
    }
    else
    {  
      current=21492-steps2+current;
    //  Serial.println(current);
      flag1=1;
    }
  }
  else if(stepper.distanceToGo()==0 && rounds==0 && flag1==0)
  {
    if(steps2>(21492-current))
    {
    current=steps2+current-21492; 
  //  Serial.println(current);
    flag1=1;
    }
    else if(steps2<=(21492-current))
    {
      current=steps2+current;
      //Serial.println(current);
      flag1=1;
    }
  }
  
  stepper.runSpeedToPosition();
  stepper1.runSpeedToPosition();
  
}


