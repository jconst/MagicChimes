void setupReceiver();
bool recvData(byte *data);
void setupMotors();
void moveAll(int vel);
void move(int motor, int8_t vel);

// ================================================================
// ===                       MAIN PROGRAM                       ===
// ================================================================

int8_t rot = 0;

void setup() {
    Serial.begin(57600);
    setupReceiver();
    setupMotors();
}

void loop() {
    bool success = recvData((byte *)&rot);
    if (success) {
        Serial.print("Got byte ");
        Serial.println(rot);
        moveAll(rot);
    }
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
    Mirf.csnPin = 8;
    Mirf.cePin = 9;

    // Set the SPI (Serial Port Interface) Driver.
    Mirf.spi = &MirfHardwareSpi;
    // Setup pins / SPI.
    Mirf.init();
    // Set the receiving address. Must be 5 bytes.
    Mirf.setRADDR((byte *)"mastr");

    // Set the payload length.
    // payload on client and server must be the same.
    Mirf.payload = sizeof(byte);
    Mirf.channel = 2;
    // Write channel and payload config then power up reciver.
    Mirf.config();
}

bool recvData(byte *data)
{
    if (!Mirf.dataReady()) return false;

    Mirf.getData(data);
    return true;
}

// ================================================================
// ===                          MOTORS                          ===
// ================================================================

const int numMotors = 3;

int STBY = 16; //standby

int IN1[numMotors] = {7, 2, 17}; //Direction
int IN2[numMotors] = {6, 3, 18}; //Direction
int PWM[numMotors] = {5, 4, 19}; //Speed control

void setupMotors()
{
    pinMode(STBY, OUTPUT);

    for (int i=0; i<3; i++) {
        pinMode(PWM[i], OUTPUT);
        pinMode(IN1[i], OUTPUT);
        pinMode(IN2[i], OUTPUT);
    }
}

void moveAll(int vel)
{
    digitalWrite(STBY, HIGH); //disable standby

    for (int i=0; i<3; i++) {
        move(i, vel);
    }
}

void move(int motor, int8_t vel)
{
    boolean inVal1 = !(vel > 0);
    boolean inVal2 =  (vel > 0);

    digitalWrite(IN1[motor], inVal1);
    digitalWrite(IN2[motor], inVal2);
    analogWrite(PWM[motor], abs(vel)*25);
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
    digitalWrite(STBY, LOW);
}
