#include <FuzzyRule.h>
#include <FuzzyComposition.h>
#include <Fuzzy.h>
#include <FuzzyRuleConsequent.h>
#include <FuzzyOutput.h>
#include <FuzzyInput.h>
#include <FuzzyIO.h>
#include <FuzzySet.h>
#include <FuzzyRuleAntecedent.h>
#include <Wire.h>
#include <PinChangeInt.h>
#define PIN1 2
#define LUX_SENSOR_ADDRESS1 0x4A

#define DOWN      0
#define UP   1 
#define OTHER 2

unsigned int lux;
const int elecmfactor=3.86;
const int mfactor=1.4;
unsigned int setpoint=500;
unsigned int taskplaneill=0;
unsigned int readings[11]={0,2,4,6,8,10,12,14,16,18,20};
unsigned int dimming[11]={0,70,100,120,140,155,170,185,205,225,255};
unsigned int voltagemFactor=883;
const int interval=120000;
char pwm=255;
int derror;
int err=0;
int preverr;
char flag=1;
long previousupdate,currentupdate,motionoccured,timenow;
char stable=0;
char count;
char relay=1;
char prevrelay=1;
char change=0;
char prevpwm=255;
float changescale;
// Instanciando um objeto da biblioteca
Fuzzy* fuzzy = new Fuzzy();

FuzzySet* verydim = new FuzzySet(-500, -500, -80, -40);
FuzzySet* dim = new FuzzySet(-80, -40, -40, 0);
FuzzySet* mid = new FuzzySet(-40, 0, 0, 40);
FuzzySet* bright = new FuzzySet(0, 40, 40, 80);
FuzzySet* verybright = new FuzzySet(40, 80, 500, 500);


FuzzySet* bigneg = new FuzzySet(-80, -80, -20, -10);
FuzzySet* neg = new FuzzySet(-20, -10, -10, 0);
FuzzySet* zero = new FuzzySet(-10, 0, 0, 10);
FuzzySet* pos = new FuzzySet(0, 10, 10, 20);
FuzzySet* bigpos = new FuzzySet(10, 20, 80, 80);




void setup(){
  Wire.begin();
Serial.begin(9600);
pinMode(9, OUTPUT);
pinMode(PIN1, INPUT); //digitalWrite(PIN1, HIGH);
PCintPort::attachInterrupt(PIN1, &quicfunc, RISING); 


  
  // FuzzyInput
  FuzzyInput* error = new FuzzyInput(1);
  error->addFuzzySet(verydim);
  error->addFuzzySet(dim);
  error->addFuzzySet(mid);
  error->addFuzzySet(bright);
  error->addFuzzySet(verybright);

  fuzzy->addFuzzyInput(error);
//-------------------------------------------------------------------------------------------------------------------------------
  // FuzzyInput
  FuzzyInput* deltaerror = new FuzzyInput(2);
  deltaerror->addFuzzySet(bigneg);
  deltaerror->addFuzzySet(neg);
  deltaerror->addFuzzySet(zero);
  deltaerror->addFuzzySet(pos);
  deltaerror->addFuzzySet(bigpos);

  fuzzy->addFuzzyInput(deltaerror);

//---------------------------------------------------------------------------------------------------------------------------------
  // FuzzyOutput
  //dimming=[0,70,100,120,140,155,170,185,205,225,255]
  FuzzyOutput* outputpwm = new FuzzyOutput(1);

  FuzzySet* verylow = new FuzzySet(0, 0, 70, 100);
  outputpwm->addFuzzySet(verylow);
  FuzzySet* low = new FuzzySet(70, 120, 120, 150);
  outputpwm->addFuzzySet(low);
  FuzzySet* med = new FuzzySet(120, 120, 150, 170);
  outputpwm->addFuzzySet(med);
  FuzzySet* high = new FuzzySet(150, 170, 170, 205);
  outputpwm->addFuzzySet(high);
  FuzzySet* veryhigh = new FuzzySet(185, 205, 255, 255);
  outputpwm->addFuzzySet(veryhigh);
   
  fuzzy->addFuzzyOutput(outputpwm);
//-----------------------------------------------------------------------------------------------------------------------------------


  // Building FuzzyRule row1
  FuzzyRuleAntecedent* errorverydimANDdeltaerrorbigneg = new FuzzyRuleAntecedent();
  errorverydimANDdeltaerrorbigneg->joinWithAND(verydim , bigneg);
  FuzzyRuleConsequent* thenoutputpwmveryhigh = new FuzzyRuleConsequent();
  thenoutputpwmveryhigh->addOutput(veryhigh);
  FuzzyRule* fuzzyRule1 = new FuzzyRule(1, errorverydimANDdeltaerrorbigneg, thenoutputpwmveryhigh);
  fuzzy->addFuzzyRule(fuzzyRule1);

  FuzzyRuleAntecedent* errorverydimANDdeltaerrorneg = new FuzzyRuleAntecedent();
  errorverydimANDdeltaerrorneg->joinWithAND(verydim , neg);
  FuzzyRule* fuzzyRule2 = new FuzzyRule(2, errorverydimANDdeltaerrorneg, thenoutputpwmveryhigh);
  fuzzy->addFuzzyRule(fuzzyRule2);

  FuzzyRuleAntecedent* errorverydimANDdeltaerrorzero = new FuzzyRuleAntecedent();
  errorverydimANDdeltaerrorzero->joinWithAND(verydim , zero);
  FuzzyRuleConsequent* thenoutputpwmhigh = new FuzzyRuleConsequent();
  thenoutputpwmhigh->addOutput(high);
  FuzzyRule* fuzzyRule3 = new FuzzyRule(3, errorverydimANDdeltaerrorzero, thenoutputpwmhigh);
  fuzzy->addFuzzyRule(fuzzyRule3);

  FuzzyRuleAntecedent* errorverydimANDdeltaerrorpos = new FuzzyRuleAntecedent();
  errorverydimANDdeltaerrorpos->joinWithAND(verydim , pos);
  FuzzyRule* fuzzyRule4 = new FuzzyRule(4, errorverydimANDdeltaerrorpos, thenoutputpwmhigh);
  fuzzy->addFuzzyRule(fuzzyRule4);

  FuzzyRuleAntecedent* errorverydimANDdeltaerrorbigpos = new FuzzyRuleAntecedent();
  errorverydimANDdeltaerrorbigpos->joinWithAND(verydim , bigpos);
  FuzzyRuleConsequent* thenoutputpwmmed = new FuzzyRuleConsequent();
  thenoutputpwmmed->addOutput(med);
  FuzzyRule* fuzzyRule5 = new FuzzyRule(5, errorverydimANDdeltaerrorbigpos, thenoutputpwmmed);
  fuzzy->addFuzzyRule(fuzzyRule5);

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//fuzzy rule row2

  FuzzyRuleAntecedent* errordimANDdeltaerrorbigneg = new FuzzyRuleAntecedent();
  errordimANDdeltaerrorbigneg->joinWithAND(dim , bigneg);
  FuzzyRule* fuzzyRule6 = new FuzzyRule(6, errordimANDdeltaerrorbigneg, thenoutputpwmveryhigh);
  fuzzy->addFuzzyRule(fuzzyRule6);

  FuzzyRuleAntecedent* errordimANDdeltaerrorneg = new FuzzyRuleAntecedent();
  errordimANDdeltaerrorneg->joinWithAND(dim , neg);
  FuzzyRule* fuzzyRule7 = new FuzzyRule(7, errordimANDdeltaerrorneg, thenoutputpwmhigh);
  fuzzy->addFuzzyRule(fuzzyRule7);

  FuzzyRuleAntecedent* errordimANDdeltaerrorzero = new FuzzyRuleAntecedent();
  errordimANDdeltaerrorzero->joinWithAND(dim , zero);
  FuzzyRule* fuzzyRule8 = new FuzzyRule(8, errordimANDdeltaerrorzero, thenoutputpwmhigh);
  fuzzy->addFuzzyRule(fuzzyRule8);

  FuzzyRuleAntecedent* errordimANDdeltaerrorpos = new FuzzyRuleAntecedent();
  errordimANDdeltaerrorpos->joinWithAND(dim , pos);
  FuzzyRule* fuzzyRule9 = new FuzzyRule(9, errordimANDdeltaerrorpos, thenoutputpwmhigh);
  fuzzy->addFuzzyRule(fuzzyRule9);

  FuzzyRuleAntecedent* errordimANDdeltaerrorbigpos = new FuzzyRuleAntecedent();
  errordimANDdeltaerrorbigpos->joinWithAND(dim , bigpos);
  FuzzyRule* fuzzyRule10 = new FuzzyRule(10, errordimANDdeltaerrorbigpos, thenoutputpwmmed);
  fuzzy->addFuzzyRule(fuzzyRule10);
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//fuzzy rule row3

  FuzzyRuleAntecedent* errormidANDdeltaerrorbigneg = new FuzzyRuleAntecedent();
  errormidANDdeltaerrorbigneg->joinWithAND(mid , bigneg);
  FuzzyRuleConsequent* thenoutputpwmzero = new FuzzyRuleConsequent();
  thenoutputpwmzero->addOutput(zero);
  FuzzyRule* fuzzyRule11 = new FuzzyRule(11, errormidANDdeltaerrorbigneg, thenoutputpwmzero);
  fuzzy->addFuzzyRule(fuzzyRule11);

  FuzzyRuleAntecedent* errormidANDdeltaerrorneg = new FuzzyRuleAntecedent();
  errormidANDdeltaerrorneg->joinWithAND(mid , neg);
  FuzzyRule* fuzzyRule12 = new FuzzyRule(12, errormidANDdeltaerrorneg, thenoutputpwmzero);
  fuzzy->addFuzzyRule(fuzzyRule12);

  FuzzyRuleAntecedent* errormidANDdeltaerrorzero = new FuzzyRuleAntecedent();
  errormidANDdeltaerrorzero->joinWithAND(mid , zero);
  FuzzyRule* fuzzyRule13 = new FuzzyRule(13, errormidANDdeltaerrorzero, thenoutputpwmzero);
  fuzzy->addFuzzyRule(fuzzyRule13);

  FuzzyRuleAntecedent* errormidANDdeltaerrorpos = new FuzzyRuleAntecedent();
  errormidANDdeltaerrorpos->joinWithAND(mid , pos);
  FuzzyRule* fuzzyRule14 = new FuzzyRule(14, errormidANDdeltaerrorpos, thenoutputpwmzero);
  fuzzy->addFuzzyRule(fuzzyRule14);

  FuzzyRuleAntecedent* errormidANDdeltaerrorbigpos = new FuzzyRuleAntecedent();
  errormidANDdeltaerrorbigpos->joinWithAND(mid , bigpos);
  FuzzyRule* fuzzyRule15 = new FuzzyRule(15, errormidANDdeltaerrorbigpos, thenoutputpwmzero);
  fuzzy->addFuzzyRule(fuzzyRule15);
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//fuzzy rule row4

  FuzzyRuleAntecedent* errorbrightANDdeltaerrorbigneg = new FuzzyRuleAntecedent();
  errorbrightANDdeltaerrorbigneg->joinWithAND(bright , bigneg);
  FuzzyRuleConsequent* thenoutputpwmlow = new FuzzyRuleConsequent();
  thenoutputpwmlow->addOutput(low);
  FuzzyRule* fuzzyRule16 = new FuzzyRule(16, errorbrightANDdeltaerrorbigneg, thenoutputpwmlow);
  fuzzy->addFuzzyRule(fuzzyRule16);

  FuzzyRuleAntecedent* errorbrightANDdeltaerrorneg = new FuzzyRuleAntecedent();
  errorbrightANDdeltaerrorneg->joinWithAND(bright , neg);
  FuzzyRule* fuzzyRule17 = new FuzzyRule(17, errorbrightANDdeltaerrorneg, thenoutputpwmlow);
  fuzzy->addFuzzyRule(fuzzyRule17);

  FuzzyRuleAntecedent* errorbrightANDdeltaerrorzero = new FuzzyRuleAntecedent();
  errorbrightANDdeltaerrorzero->joinWithAND(bright , zero);
  FuzzyRuleConsequent* thenoutputpwmverylow = new FuzzyRuleConsequent();
  thenoutputpwmverylow->addOutput(verylow);
  FuzzyRule* fuzzyRule18 = new FuzzyRule(18, errorbrightANDdeltaerrorzero, thenoutputpwmverylow);
  fuzzy->addFuzzyRule(fuzzyRule18);

  FuzzyRuleAntecedent* errorbrightANDdeltaerrorpos = new FuzzyRuleAntecedent();
  errorbrightANDdeltaerrorpos->joinWithAND(bright , pos);
  FuzzyRule* fuzzyRule19 = new FuzzyRule(19, errorbrightANDdeltaerrorpos, thenoutputpwmverylow);
  fuzzy->addFuzzyRule(fuzzyRule19);

  FuzzyRuleAntecedent* errorbrightANDdeltaerrorbigpos = new FuzzyRuleAntecedent();
  errorbrightANDdeltaerrorbigpos->joinWithAND(bright , bigpos);
  
  FuzzyRule* fuzzyRule20 = new FuzzyRule(20, errorbrightANDdeltaerrorbigpos, thenoutputpwmverylow);
  fuzzy->addFuzzyRule(fuzzyRule20);
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//fuzzy rule row5

  FuzzyRuleAntecedent* errorverybrightANDdeltaerrorbigneg = new FuzzyRuleAntecedent();
  errorverybrightANDdeltaerrorbigneg->joinWithAND(verybright , bigneg);
  FuzzyRule* fuzzyRule21 = new FuzzyRule(21, errorverybrightANDdeltaerrorbigneg, thenoutputpwmverylow);
  fuzzy->addFuzzyRule(fuzzyRule21);

  FuzzyRuleAntecedent* errorverybrightANDdeltaerrorneg = new FuzzyRuleAntecedent();
  errorverybrightANDdeltaerrorneg->joinWithAND(verybright , neg);
  FuzzyRule* fuzzyRule22 = new FuzzyRule(22, errorverybrightANDdeltaerrorneg, thenoutputpwmverylow);
  fuzzy->addFuzzyRule(fuzzyRule22);

  FuzzyRuleAntecedent* errorverybrightANDdeltaerrorzero = new FuzzyRuleAntecedent();
  errorverybrightANDdeltaerrorzero->joinWithAND(bright , zero);
  FuzzyRule* fuzzyRule23 = new FuzzyRule(23, errorverybrightANDdeltaerrorzero, thenoutputpwmverylow);
  fuzzy->addFuzzyRule(fuzzyRule23);

  FuzzyRuleAntecedent* errorverybrightANDdeltaerrorpos = new FuzzyRuleAntecedent();
  errorverybrightANDdeltaerrorpos->joinWithAND(verybright , pos);
  FuzzyRule* fuzzyRule24 = new FuzzyRule(24, errorverybrightANDdeltaerrorpos, thenoutputpwmverylow);
  fuzzy->addFuzzyRule(fuzzyRule24);

  FuzzyRuleAntecedent* errorverybrightANDdeltaerrorbigpos = new FuzzyRuleAntecedent();
  errorverybrightANDdeltaerrorbigpos->joinWithAND(verybright , bigpos);
  FuzzyRule* fuzzyRule25 = new FuzzyRule(25, errorverybrightANDdeltaerrorbigpos, thenoutputpwmverylow);
  fuzzy->addFuzzyRule(fuzzyRule25);
//--------------------------------------------------------------------------------------------------------------------------------------------------------

}
void quicfunc() {
  delay(100);
Serial.println("Motion detected");  
relay=1;
motionoccured=millis();
}


void lux_value()
{
    unsigned int data1,mantissa,exponent;     
    unsigned int data2;
    Wire.beginTransmission(0x4A);
    Wire.write(0x03);
    Wire.endTransmission();    
    Wire.requestFrom(0x4A, 2);
    data1=Wire.read();
    data2=Wire.read();
    
    mantissa= (data2 & 0x0f)+ ((data1 & 0x0f)<<4) ;
    exponent = ((data1 & 0xff)>>4);
    lux= (1<<exponent) * mantissa * 0.045;
    Serial.println(lux);
  }
void taskplaneill_value()
{
  unsigned int daylight;
  daylight=(lux-((pwm*voltagemFactor)/10000)); 
  taskplaneill=(daylight*mfactor)+(((pwm*voltagemFactor)/10000)*elecmfactor);//voltagemfactor will be multiplied by changescale in recalibration
  
}

void controller()
{
       fuzzy->setInput(1, err);
       fuzzy->setInput(2, derror);
       fuzzy->fuzzify();
       pwm=int(fuzzy->defuzzify(1));
}

void changeshadeposition(char cases)
{
  switch (cases)
  {
    case DOWN: 
    while(count>0 )
    {
        //moveshadedown bring light film
        delay(7000);
        lux_value();
        taskplaneill_value();
        err=taskplaneill-setpoint;
        if(abs(err)>20)
        { preverr=err;
          cases=OTHER;
        }
        
    }
    break;
    case UP:
      while(count<20 && abs(err)>20)
    {
        //moveshadeup bring dark film
        delay(7000);
        lux_value();
        taskplaneill_value();
        err=taskplaneill-setpoint;
    } 
    break;
          
    case OTHER:
      while(count>0)
    {
        //moveshadedown
        delay(7000);
        lux_value();
        taskplaneill_value();
        err=taskplaneill-setpoint;
        derror=err-preverr;
        controller();
        preverr=err;
        if(pwm<50)
           cases=DOWN;
            
    }
         break;

  }  
  stable=0;
  return;
 
  
}
void recalibration()
{
  changescale=1.0+ ((derror*1.0)/err);
  setpoint=setpoint*changescale;
  voltagemFactor=voltagemFactor*changescale;
  }


void loop(){
  timenow=millis();
  if((timenow-motionoccured)>300000)
      relay=0;
  lux_value();
  //pwm_value();//written fuzzy
  //relay(); written quicfunc
  taskplaneill_value();
  err=taskplaneill-setpoint;
  if(flag==1)
  {
    preverr=err;
    flag=0;
    }
  derror=err-preverr;
  
  if(abs(derror)>10)
  {
    //checkdaylightchange();//to check daylight change yet to write
    
    if(1)
    {
      //check it shade moved implies manual? else adjust voltage
       if (abs(err)>20)
       {
        controller();  
        analogWrite(9,pwm);
        previousupdate = millis();//to know the stable time  
        change=1;
        delay(100);   
       }
    }
    else
    {
      if(abs(prevpwm-pwm)>10)//deltaerror to know if limit reaced of pwm
        {
          controller();  
          analogWrite(9,pwm);
          previousupdate = millis();//to know the stable time  
          change=1;
          delay(100);  
        }
       else if(prevrelay-relay==1)
       {
        
        } 
       
       else
        recalibration(); //done
     }

  }
  
  currentupdate=millis();
  if(currentupdate - previousupdate > interval && change==1)
  {
    stable=1;
    //move shade and fuzzy till eof or v=0 
    while(stable)
    {
      //send
      if(pwm>205 && err <-20)//when electric light not sufficient
       {
        changeshadeposition(0);//movedowncommand
       }
      if(pwm<50 && err >20)
       {
          changeshadeposition(1);//moveup 
       }
      else
        changeshadeposition(2);
      
      }
    change=0;
    } 
  Serial.println(pwm);
  preverr=err;
  prevpwm=pwm;
  prevrelay=relay;
  delay(100);

 
}
