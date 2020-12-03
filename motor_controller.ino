
volatile boolean intstate = 0;
uint32_t t0; // to calc frequency
uint32_t t1;
uint32_t tmrp; // serial print tmr
//String instr = "";
int8_t motor_pin = 6;
uint8_t pwm_out = 0;

int16_t kp = 21;
int32_t ki = 0;
int16_t kd = 0;
int setvalue = 750;
int16_t error;
int32_t errorinc;
uint32_t tmrc;

void setup()
{
    Serial.begin(9600);
    attachInterrupt(0, state_change, RISING);
    pinMode(motor_pin, OUTPUT);
    t0 = micros();
    tmrp = millis();
    tmrc = millis();
}

void loop()
{
    //sprinter(f(), 1000);
    tperiod();
    if(Serial.available())
    {
        //delay(5);
        //Serial.println(Serial.available());
        ser_read();
        
    }
    
    if(millis() - tmrp > 1000)
    {
        Serial.print("sp=");
        Serial.print(setvalue);
        Serial.print("|f=");
        Serial.print(f());
        Serial.print("|pwm=");
        Serial.print(pwm_out);
        Serial.print("|kp=");
        Serial.print(kp);
        Serial.print("|ki=");
        Serial.print(ki);
        Serial.print("|e=");
        Serial.print(error);
        Serial.print("|einc=");
        Serial.print(errorinc);
        Serial.print("|iout=");
        Serial.print(ki * errorinc / 1000000);
        Serial.print("|kd");
        Serial.print(kd / 100);
        Serial.print("|dout=");
        Serial.println(error * kd /100);
        tmrp = millis();
    }
    
    if(millis() - tmrc > 20)
    {
        pwm_out = pi_out();
        analogWrite(motor_pin, pwm_out);
        tmrc = millis();
    }
    

}

void state_change()
{
    intstate = 1;

}

uint32_t tperiod()
{
    if (intstate)
    {
        intstate = 0;
        t1 = micros() - t0;
        t0 = micros();
    }
    if (micros() - t0 > 100000)
    {
        return 4294967295; 
    }
    else
    {
        return t1;
    }
}

uint16_t f() //frequency
{

    return 1000000 / tperiod();
}

void sprinter(uint32_t inprint, uint16_t indly)
{
    if (millis() - tmrp > indly)
    {
        Serial.println(inprint);
        tmrp = millis();
    }
}

int16_t ser_read_int()
{
    String instr = "";
    while (Serial.available())
    {
        instr += (char) Serial.read();
        delay(5);
        if (!Serial.available())
        {
            //Serial.println(instr.toInt());
            //instr = "";
            return instr.toInt();
        } 
    }
}

void ser_read()
{
    String instr = "";
    while (Serial.available())
    {
        instr += (char) Serial.read();
        delay(5);
        //Serial.println("in");
        if (!Serial.available())
        {
            if(instr.startsWith("s"))
            {
                instr.remove(0, 1);
                setvalue = instr.toInt();
                //Serial.println("sv");
            }
            else if(instr.startsWith("p"))
            {
                instr.remove(0, 1);
                kp = instr.toInt();
            }  
            else if(instr.startsWith("i"))
            {
                instr.remove(0, 1);
                ki = instr.toInt();
            } 
            else if(instr.startsWith("d"))
            {
                instr.remove(0, 1);
                kd = instr.toInt();
            }  
        } 
    }
}

float pout()
{
    float pgain = (float) kp / 100;
    return (int16_t)(pgain * setvalue);
}

float iout()
{
    float igain = (float) ki / 1000000;
    error = setvalue - f();
    if(ki != 0)
    {
        if(pwm_out < 0 && error < 0){}
        else if(pwm_out > 255 && error > 0){}
        else {errorinc += error;}
        float outv = igain * errorinc;
        //if(outv < 0)
            //outv = 0;
        return outv;
    }
    else
    {
        errorinc = 0;
    }
    
    return 0;
}

uint16_t dout()
{
    float dgain = (float)kd / 100;
    return error * kd;
}
int16_t pi_out()
{
    float outv = (float)pout() + iout() + dout();
    if(outv > 255)
        outv = 255;
    else if(outv < 0)
    {
        outv = 0;
    }
    return (int16_t)outv;
}