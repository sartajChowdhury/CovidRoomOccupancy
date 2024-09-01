#include "Adafruit_VL53L0X.h"

#include "WirelessCommunication.h"
#include "sharedVariable.h"
#include "Preferences.h"

// make an instance of the library:
Adafruit_VL53L0X sensor1 = Adafruit_VL53L0X();
Adafruit_VL53L0X sensor2 = Adafruit_VL53L0X();

#define SHUT1 27
#define SHUT2 14
#define IRPinL1 34
#define IRPinL2 39
#define IRPinR1 32
#define IRPinR2 35


void init_non_vol_storage();
void update_non_vol_count();
void update_button_count();

const int maxDistance = 2000;

volatile int state = 0;
volatile int IRStateL = 0;
volatile int IRStateR = 0;

volatile bool entering = false;
volatile bool exiting = false;

volatile bool enteringL = false;
volatile bool exitingL = false;

volatile bool enteringR = false;
volatile bool exitingR = false;

//IDEA: COUNT TOTAL ACTIVATIONS, USE WHICHEVER HIGHER?
//OR JUST USE DIST (prob this)
volatile bool LIRActive = false;
volatile bool RIRActive = false;

//state 0: both off
//state 1: sens1 on, sens2 off
//state 2: sens1 off, sens2 on
//state 13: both on (coming from 1);
//state 23; both on (coming from 2); ??

volatile uint32_t count = 0;
volatile int LIRCount = 0;
volatile int RIRCount = 0;
volatile int TOFCount = 0;

volatile shared_uint32 x;
Preferences nonVol;//used to store the count in nonvolatile memory

void setup() {
    // initialize serial, wait 3 seconds for
    // Serial Monitor to open:
    //Serial.begin(9600);
    Serial.begin(115200);

    pinMode(IRPinL1, INPUT);

    pinMode(IRPinL2, INPUT);

    pinMode(IRPinR1, INPUT);
    pinMode(IRPinR2, INPUT);

    if (!Serial) delay(3000);
    Serial.println("Stage 1");
    pinMode(SHUT1, OUTPUT);
    pinMode(SHUT2, OUTPUT);

    Serial.println("Stage 1.1");
    digitalWrite(SHUT1, LOW);
    digitalWrite(SHUT2, LOW);

    Serial.println("Stage 1.2");
    delay(10);
    digitalWrite(SHUT1, HIGH);

    Serial.println("Stage 1.3");
    // initialize sensor, stop if it fails:
    if (!sensor1.begin(0x30)) {
        Serial.println("Sensor 1 not responding. Check wiring.");
        while (true);
    }
    delay(10);
    digitalWrite(SHUT2, HIGH);
    // initialize sensor, stop if it fails:
    if (!sensor2.begin(0x31)) {
        Serial.println("Sensor 2 not responding. Check wiring.");
        while (true);
    }

    Serial.println("Stage 2");
    /* config can be:
      VL53L0X_SENSE_DEFAULT: about 500mm range
      VL53L0X_SENSE_LONG_RANGE: about 2000mm range
      VL53L0X_SENSE_HIGH_SPEED: about 500mm range
      VL53L0X_SENSE_HIGH_ACCURACY: about 400mm range, 1mm accuracy
    */
    sensor1.configSensor(Adafruit_VL53L0X::VL53L0X_SENSE_LONG_RANGE);
    sensor2.configSensor(Adafruit_VL53L0X::VL53L0X_SENSE_LONG_RANGE);
    // set sensor to range continuously:
    sensor1.startRangeContinuous();
    sensor2.startRangeContinuous();

    //TEMP DISABLE WIFI
init_wifi_task();

init_non_vol_count();//initializes nonvolatile memory and retrieves latest count
    count = 0;
    LIRCount = 0;
    RIRCount = 0;
    INIT_SHARED_VARIABLE(x, count);
}

void loop() {
    // if the reading is done:
    if (sensor1.isRangeComplete() && sensor2.isRangeComplete()) {
        // read the result:
        int result = sensor1.readRangeResult();
        int result2 = sensor2.readRangeResult();
        //sens2 is exit sens
        //state 0 (both off)
        if (result == 0) {
            result = 1000;
        }
        if (result2 == 0) {
            result2 = 1000;
        }

        int L1 = digitalRead(IRPinL1);
        int L2 = digitalRead(IRPinL2);
        int R1 = digitalRead(IRPinR1);
        int R2 = digitalRead(IRPinR2);

        if ((result + result2) < 270) {
          LIRActive = true;
          RIRActive = true;
          }
        /*if (result < 150) {
          LIRActive = true;
        }
        if (result2 < 150) {
          RIRActive = true;
          }*/


        //LIR Logic
        //if ((L1 == LOW || L2 == LOW) && result < 70) {
        //    LIRActive = true; //STOP IRS
        //}
        if (L1 == HIGH && L2 == HIGH) {
            if (IRStateL == 0) { // do nothing        
            }
            else if (IRStateL == 1) {
                if (!enteringL && exitingL) {
                    LIRCount--;
                }
            }
            else if (IRStateL == 2) {
                if (enteringL && !exitingL) {
                    LIRCount++;
                }
            }
            else { //state 3

            }
            IRStateL = 0;
            enteringL = false;
            exitingL = false;
        }
        else if (L1 == LOW && L2 == HIGH) {
            Serial.println("IR 1 L");
            if (IRStateL == 0) {
                IRStateL = 1;
                enteringL = true;
            }
            else if (IRStateL == 1) {}
            else if (IRStateL == 2) {
                IRStateL = 1;
            }
            else { //state 3
                IRStateL = 1;
            }
        }
        else if (L1 == HIGH && L2 == LOW) {
            Serial.println("IR 2 L");
            if (IRStateL == 0) {
                IRStateL = 2;
                exitingL = true;
            }
            else if (IRStateL == 1) {
                IRStateL = 2;
            }
            else if (IRStateL == 2) {}
            else { //state 3
                IRStateL = 2;
            }
        }
        else { //else both r high
            Serial.println("IR 1, 2 L");
            if (IRStateL == 0) {
                IRStateL = 3;
            }
            else if (IRStateL == 1) {
                IRStateL = 3;
            }
            else if (IRStateL == 2) {
                IRStateL = 3;
            }
            else { //state 3

            }
        }


        //RIR Logic
        //if ((R1 == LOW || R2 == LOW) && result2 < 70) {
        //    RIRActive = true; //STOP IRS
        //}
        if (R1 == HIGH && R2 == HIGH) {
            if (IRStateR == 0) { // do nothing        
            }
            else if (IRStateR == 1) {
                if (!enteringR && exitingR) {
                    RIRCount--;
                }
            }
            else if (IRStateR == 2) {
                if (enteringR && !exitingR) {
                    RIRCount++;
                }
            }
            else { //state 3

            }
            IRStateR = 0;
            enteringR = false;
            exitingR = false;
        }
        else if (R1 == LOW && R2 == HIGH) {
            Serial.println("IR 1 R");
            if (IRStateR == 0) {
                IRStateR = 1;
                enteringR = true;
            }
            else if (IRStateR == 1) {}
            else if (IRStateR == 2) {
                IRStateR = 1;
            }
            else { //state 3
                IRStateR = 1;
            }
        }
        else if (R1 == HIGH && R2 == LOW) {
            Serial.println("IR 2 R");
            if (IRStateR == 0) {
                IRStateR = 2;
                exitingR = true;
            }
            else if (IRStateR == 1) {
                IRStateR = 2;
            }
            else if (IRStateR == 2) {}
            else { //state 3
                IRStateR = 2;
            }
        }
        else { //else both r high
            Serial.println("IR 1, 2 R");
            if (IRStateR == 0) {
                IRStateR = 3;
            }
            else if (IRStateR == 1) {
                IRStateR = 3;
            }
            else if (IRStateR == 2) {
                IRStateR = 3;
            }
            else { //state 3

            }
        }

        //state 0 (none on)
        if (result > 700 && result2 > 700) {
            //0 to 0
            if (state == 0) { //do nothing, same state     
            }
            // coming from state 1
            else if (state == 1) {
                if (entering && !exiting) {
                  TOFCount = 0;  
                } //person turned around
                
                //  TEMP DISABLE
                else if (exiting && !entering) {
                    //decrement count
                    //if (count != 0) {
                    //--TOFCount;
                    //}
                    //update_button_count();//update shared variable x (shared with WiFi task)
                    //update_non_vol_count();//updates nonvolatile count 
                }
                //else 2 person case
                state = 0;
            }
            //2 to 0
            else if (state == 2) {
              //TEMP DISABLE
                if (entering && !exiting) {
                    //increment count
                    //++TOFCount;
                    //update_button_count();//update shared variable x (shared with WiFi task)
                    //update_non_vol_count();//updates nonvolatile count 
                }
                else if (exiting && !entering) {
                  TOFCount = 0;
                  } //person turned around
                  
                //else 2 person case?
                state = 0;
            }
            //3 to 0
            else if (state == 3) {
                //two person case?
                state = 0;
            }
            entering = false;
            exiting = false;
        }
        //state 1 (just 1 on)
        else if (result <= 700 && result2 > 700) {
            //from state 0
            Serial.println("TOF 1");
            if (state == 0) {
                state = 1;
                entering = true;
            }
            //1 to 0
            else if (state == 1) {
            }
            //2 to 0
            else if (state == 2) {
                state = 1;
            }
            //3 to 0
            else if (state == 3) {
                 if (exiting && !entering) {
                    //decrement count
                    //if (count != 0) {
                    --TOFCount;
                 }
              
                state = 1;
            }
        }
        //state 2 (just 2 on)
        else if (result > 700 && result2 <= 700) {
            //from state 0
            Serial.print("                ");
            Serial.println("TOF 2");
            if (state == 0) {
                state = 2;
                exiting = true;
            }
            //1 to 0
            else if (state == 1) {
                state = 2;
            }
            //2 to 0
            else if (state == 2) {
            }
            //3 to 0
            else if (state == 3) {
              //NEW ADD FROM 3 TO 2 (removed prev from 2 to 0, should be caught by the check in state 0)
                if (entering && !exiting) {
                    ++TOFCount;
                }
                state = 2;
            }
        }
        //state 3 (both on)
        else {
            Serial.println("TOF 1, 2");
            //from state 3
            if (state == 0) {
            }
            /*1 3 2 3 2 0
            0 2 3 (1 or 3) (3 or 2) 1 0
            prob not ORs? must or likely empty towards side of movement when follow close? 
            0 1 3 2 0 //regular enter
            0 2 3 1 0 //regular exit
            */
            
            //1 to 3
            else if (state == 1) {
              if (exiting) { //case of walking very close together
                //TOFCount--;
              }
            }
            //2 to 3
            else if (state == 2) {
              if (entering) { //case of walking very close together
               // TOFCount++;
              }
            }
            //3 to 3
            else if (state == 3) {
            }
            state = 3;
        }

        //total result (everyone has passed thru)
        if (result > 700 && result2 > 700
            && L1 == HIGH && L2 == HIGH
            && R1 == HIGH && R2 == HIGH) {
            if (LIRActive && RIRActive) {// && RIRActive) {
                count += LIRCount + RIRCount;
            }
            else {
                count += TOFCount;
            }

            if(count > 4000000) {
              count = 0;}
            Serial.println(count);
            //Serial.print("IR    ");
            //Serial.println(LIRCount);
            //Serial.print("TOF   ");
            //Serial.println(TOFCount);

            LIRCount = 0;
            RIRCount = 0;
            TOFCount = 0;
            LIRActive = false;
            RIRActive = false;
            //TEMP DISABLE WIFI
            update_button_count();//update shared variable x (shared with WiFi task)
            update_non_vol_count();//updates nonvolatile count 
        }

        Serial.println(result + result2);
        //Serial.println(result2);
        if(LIRActive) {
          
        Serial.print("L Set");
        }
        if(RIRActive) {
          
        Serial.print("R Set");
          }
          

    }
    /*
    if (sensor2.isRangeComplete()) {
      // read the result:
      int result = sensor2.readRangeResult();
      // if it's with the max distance:
      if (result < maxDistance) {
        // print the result (distance in mm):
        Serial.print("Sensor2: ");
        Serial.println(result);
      }
    }
    */
}



//*********WIFI STUFF**********

//initializes nonvolatile memory and retrieves latest count
void init_non_vol_count()
{
    nonVol.begin("nonVolData", false);//Create a “storage space” in the flash memory called "nonVolData" in read/write mode
    count = nonVol.getUInt("count", 0);//attempts to retrieve "count" from nonVolData, sets it 0 if not found
}

//updates nonvolatile memery with lates value of count
void update_non_vol_count()
{
    nonVol.putUInt("count", count);//write count to nonvolatile memory
}

//example code that updates a shared variable (which is printed to server)
//under the hood, this implementation uses a semaphore to arbitrate access to x.value
void update_button_count()
{
    //minimized time spend holding semaphore
    LOCK_SHARED_VARIABLE(x);
    x.value = count;
    UNLOCK_SHARED_VARIABLE(x);
}
