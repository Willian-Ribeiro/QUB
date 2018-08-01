#include "mbed.h"
#include "Servo.h"
#include "C12832.h"

// servos
Servo myservo1(p21);
Servo myservo2(p22);
C12832 lcd(p5, p7, p6, p8, p11);

//LEDs
DigitalOut myled(LED1);

// interrupcao e serial
InterruptIn usbSerial(USBRX);
Serial pc(USBTX, USBRX); // tx, rx

const int nCoord = 2;

void interruptionIn();
int calculateInput(int i);
float secureValue(float v);
void cleanBuffer();
void posMotor(int i);
void outOfSight();

char charU[nCoord], charL[nCoord];

int ref[nCoord], coord[nCoord], eOld[nCoord], E[nCoord];

float kp[nCoord], ki[nCoord], kd[nCoord];

float pos[nCoord], motorStep = 0.002;
int pixelPrecision = 5;

const int sizeStored = 15;
float lastPos[nCoord][sizeStored];// 2-> x and y | 20-> number of positions recorded for each
int ite = 0;
int refCount = 0;

int main()
{
    usbSerial.rise(&interruptionIn);

    myservo1.calibrate(0.001, 180.0);
    myservo2.calibrate(0.001, 180.0);

    kp[0] = 1;
    kp[1] = 1;

    ki[0] = 0.05;
    ki[1] = 0.05;

    kd[0] = 1;
    kd[1] = 1;

    eOld[0] = eOld[1] = E[0] = E[1] = 0;

    ref[0] = ref[1] = 255;

    pos[0] = 0.3;
    pos[1] = 0.7;
    myservo1 = pos[0];
    myservo2 = pos[1];

    while(1) {
        myled = !myled;

        wait(1);
    }
}

void interruptionIn()
{
    char c = pc.getc();
    //pc.printf("Valor de c: %c\n", c);

    // 's' indicates the actual position ------------------------ int here have 4 bytes
    if(c == 's') {
        ite++;
        ite = ite % sizeStored;
        refCount = 0; // makes this counter 0 indicating that the laser isnt out of screen

        //updates x and y, receiving them as char and transforming in int
        for(int i = 0; i < nCoord; i++) {
            coord[i] = 0;
            charU[i] = pc.getc();
            if(charU[i] != 'z') { // if value is zero can jump to last step
                coord[i] = (int)charU[i];
                coord[i] = coord[i] << (sizeof(coord[i])*8*1/4); // sizeof returns char, so I multiply by 8 to have bits and divide by 2 to shift half way
            }
            charL[i] = pc.getc();
            coord[i] += (int)charL[i];
        }
        //pc.printf("X Upper Value: %d, X Lower Value: %d\n", charU[0], charL[0]);
        //lcd.printf("%d %d", charU[0], charL[0]);

        posMotor(0);
        posMotor(1);

        return;
    }

    // 'r' indicates the reference position
    if(c == 'r') {
        refCount++;

        //updates x and y, receiving them as char and transforming in int
        for(int i = 0; i < nCoord; i++) {
            ref[i] = 0;
            charU[i] = pc.getc();
            if(charU[i] != 'z') { // if value is zero can jump to last step
                ref[i] = (int)charU[i];
                ref[i] = ref[i] << (sizeof(ref[i])*8*1/4); // sizeof returns char, so I multiply by 8 to have bits and divide by 2 to shift half way
            }
            charL[i] = pc.getc();
            ref[i] += (int)charL[i];
        }
        //pc.printf("Reference X Upper Value: %c, reference X Lower Value: %c\n", charU[0], charL[0]);
//        pc.printf("Reference Y Upper Value: %c, reference Y Lower Value: %c\n", charU[1], charL[1]);

        if(refCount >= 3) outOfSight();

        return;
    }
}

int calculateInput(int i)
{
    int error = ref[i] - coord[i];

    int errorDot = error - eOld[i];
    eOld[i] = error;// updates the value

    E[i] = E[i] + error;

    return kp[i]*error + ki[i]*E[i] + kd[i]*errorDot;
}

float secureValue(float v)
{
    if(v > 0.9) v = 0.9;
    if(v < 0.1) v = 0.1;
    return v;
}

void cleanBuffer()
{
    while(pc.readable()) pc.getc();
}

void posMotor(int i)
{
    int a = (ref[i]-coord[i]);
    if( (a > 0) && (a < pixelPrecision) ) return;
    if( (a < 0) && (a > -pixelPrecision) ) return;

    if( a > 0 ) {
        //pos[i] += motorStep;
        pos[i] += (calculateInput(i)*motorStep/20); // divided by alpha = 10, this may need to vary
        lastPos[i][ite] = pos[i];
        if(i == 0) myservo1 = secureValue(pos[i]);
        else myservo2 = secureValue(pos[i]);
    } else {
        //pos[i] -= motorStep;
        pos[i] += (calculateInput(i)*motorStep/20);
        lastPos[i][ite] = pos[i];
        if(i == 0) myservo1 = secureValue(pos[i]);
        else myservo2 = secureValue(pos[i]);
    }
}

void outOfSight()
{
    eOld[0] = eOld[1] = E[0] = E[1] = 0;
    ite--;
    if(ite < 0) ite = sizeStored - 1;
    myservo1 = secureValue(lastPos[0][ite]);
    myservo2 = secureValue(lastPos[1][ite]);
    //lcd.printf(" (%f %f) ", lastPos[0][ite], lastPos[1][ite]);
}