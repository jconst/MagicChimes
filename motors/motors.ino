void setupReceiver();
bool recvData(byte *data);
void setupMotors();
void moveAll(int vel);
void move(int motor, int8_t vel);
float norm(float x);
int8_t denorm(float x);
void turn();

#define SIGN(X) ((X < 0) ? -1 : ((X > 0) ? 1 : 0))

// ================================================================
// ===                       MAIN PROGRAM                       ===
// ================================================================

int8_t recvd = 0;
float curSpeed = 0;

const int listSize = 10;
int8_t recentRecvs[listSize] = {0};
int ind = 0;

void setup() {
    Serial.begin(57600);
    setupReceiver();
    setupMotors();
}

void loop() {
    bool success = recvData((byte *)&recvd);
    if (success) {
        if (recvd == -128 || recvd == 127) return;
        recvd = -recvd; //CCW is + to CW is +
        
        Serial.println(recvd);
        
        recentRecvs[ind] = recvd;
        ind = (ind+1) % listSize;
        
        curSpeed += ((float)recvd / 60.0);
        curSpeed -= 0.07 * SIGN(curSpeed);
        curSpeed = constrain(curSpeed, -70.0, 70.0);
        float normSpeed = norm(curSpeed);
        float tapered = pow(abs(normSpeed), 0.4) * SIGN(curSpeed);
        int8_t final = denorm(tapered);
        // if they stood up:
//        if (recvd == -128) {
//            stop();
//            curSpeed = 0;
//        }
        // handle turns:
        if (SIGN(curSpeed) != SIGN(recvd) &&
            abs(curSpeed) > 15 && abs(recvd) > 1 &&
            signsMatch()) {
            Serial.println("TURN!");
            curSpeed = -curSpeed;        
            turn();
        } 
        // if they're moving too slow:
        else if (abs(final) < 10) {
            final = 0;
            standby();
        } else {
            moveAll(final);
        }
    }
}

bool signsMatch()
{
    int firstSign = SIGN(recentRecvs[0]);
    for (int i=0; i < listSize; ++i) {
        if (SIGN(recentRecvs[i]) != firstSign) {
            return false;
        } 
    }
    return true;
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
int IN2[numMotors] = {6, 4, 18}; //Direction
int PWM[numMotors] = {5, 3, 10}; //Speed control

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

void move(int motor, int vel)
{
    boolean inVal1 = !(vel > 0);
    boolean inVal2 =  (vel > 0);

    // big motor
    if (motor == 0) {
        vel *= 1.6;
    } else if (motor == 2) {
        vel *= 0.65;
    }

    digitalWrite(IN1[motor], inVal1);
    digitalWrite(IN2[motor], inVal2);
    analogWrite(PWM[motor], (int)(abs(vel)*2.55));
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

void turn()
{
    brake();
    delay(250);
}

// ================================================================
// ===                          HELPERS                         ===
// ================================================================

float norm(float x)
{
    return (float)x / 100.0;
}

int8_t denorm(float x)
{
    return (int8_t)(x * 100);
}
