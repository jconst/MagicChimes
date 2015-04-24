void setupReceiver();
bool recvData(byte *data);
void setupMotors();
void moveAll(int vel);
void move(int motor, byte vel);

// ================================================================
// ===                       MAIN PROGRAM                       ===
// ================================================================

byte rot = 0;

void setup() {
    setupReceiver();
    setupMotors();
}

void loop() {
    bool success = recvData(&rot);
    if (success)
        moveAll(rot);
}

// ================================================================
// ===                         RECEIVER                         ===
// ================================================================

#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>

void setupReceiver()
{
    // Set the SPI (Serial Port Interface) Driver.
    Mirf.spi = &MirfHardwareSpi;
    // Setup pins / SPI.
    Mirf.init();
    // Set the receiving address. Must be 5 bytes.
    Mirf.setRADDR((byte *)"mastr");

    // Set the payload length.
    // payload on client and server must be the same.
    Mirf.payload = sizeof(int);
    // Write channel and payload config then power up reciver.
    Mirf.config();
}

bool recvData(byte *data)
{
    if (!Mirf.dataReady()) return false;

    Serial.println("Got packet");  
    Mirf.getData(data);
}

// ================================================================
// ===                          MOTORS                          ===
// ================================================================

const int numMotors = 3;

int STBY[numMotors] = {2, 2, 3}; //standby

int IN1[numMotors] = {4, 7, 10}; //Direction
int IN2[numMotors] = {5, 8, 11}; //Direction
int PWM[numMotors] = {6, 9, 12}; //Speed control

void setupMotors()
{
    for (int i=0; i<3; i++) {
        pinMode(STBY[i], OUTPUT);

        pinMode(PWM[i], OUTPUT);
        pinMode(IN1[i], OUTPUT);
        pinMode(IN2[i], OUTPUT);
    }
}

void moveAll(int vel)
{
    for (int i=0; i<3; i++) {
        move(i, vel);
    }
}

void move(int motor, byte vel)
{
    digitalWrite(STBY[motor], HIGH); //disable standby

    boolean inVal1 = !(vel > 0);
    boolean inVal2 =  (vel > 0);

    digitalWrite(IN1[motor], inVal1);
    digitalWrite(IN2[motor], inVal2);
    analogWrite(PWM[motor], abs(vel));
}

void brake()
{
    for (int i=0; i<3; i++) {
        digitalWrite(IN1[i], HIGH);
        digitalWrite(IN2[i], HIGH);
    }
}

void stop()
{
    for (int i=0; i<3; i++) {
        digitalWrite(IN1[i], LOW);
        digitalWrite(IN2[i], LOW);
    }
}

void standby()
{
    //enable standby  
    digitalWrite(STBY[i], LOW); 
}