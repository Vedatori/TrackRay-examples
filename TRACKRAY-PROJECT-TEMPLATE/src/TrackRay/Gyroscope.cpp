#include "Gyroscope.h"
#include <MPU6050_6Axis_MotionApps20.h>
//#include "MPU6050_6Axis_MotionApps_V6_12.h"

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
    mpuInterrupt = true;
}

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for SparkFun breakout and InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 mpu;
//MPU6050 mpu(0x69); // <-- use for AD0 high

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
//float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

bool initiateGyroscope(const int16_t aOffsets[]) {
    mpu.initialize();   // initialize device
    delay(20);
    devStatus = mpu.dmpInitialize();

    // verify connection
    if(!mpu.testConnection()){
        Serial.println("MPU6050 connection failed");
        return false;
    }
    Serial.println("MPU6050 connection successful");

    // supply your own gyro offsets here, scaled for min sensitivity
    if(aOffsets != NULL) {
        mpu.setXAccelOffset(aOffsets[0]);
        mpu.setYAccelOffset(aOffsets[1]);
        mpu.setZAccelOffset(aOffsets[2]);
        mpu.setXGyroOffset(aOffsets[3]);
        mpu.setYGyroOffset(aOffsets[4]);
        mpu.setZGyroOffset(aOffsets[5]);
    }
    else {
        mpu.setXAccelOffset(0);
        mpu.setYAccelOffset(0);
        mpu.setZAccelOffset(0);
        mpu.setXGyroOffset(0);
        mpu.setYGyroOffset(0);
        mpu.setZGyroOffset(0);
    }

    // make sure it worked (returns 0 if so)
    if (devStatus == 0) {
        // turn on the DMP, now that it's ready
        mpu.setDMPEnabled(true);

        mpuIntStatus = mpu.getIntStatus();

        // set our DMP Ready flag so the main loop() function knows it's okay to use it
        dmpReady = true;

        // get expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
    } else {
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(devStatus);
        Serial.println(F(")"));
    }
    return true;
}
void updateGyroData(float * aYPR) {
    if (dmpReady)   // wait for MPU interrupt or extra packet(s) available
    {
        //while (!mpuInterrupt && fifoCount < packetSize) {}
        // reset interrupt flag and get INT_STATUS byte
        //mpuInterrupt = false;
        mpuIntStatus = mpu.getIntStatus();
        fifoCount = mpu.getFIFOCount();// get current FIFO count
    
        // check for overflow (this should never happen unless our code is too inefficient)
        if ((mpuIntStatus & 0x10) || fifoCount == 1024) 
        {
            // reset so we can continue cleanly
            mpu.resetFIFO();
    
        } 
        else if (mpuIntStatus & 0x02)   // otherwise, check for DMP data ready interrupt (this should happen frequently)
        {
            // wait for correct available data length, should be a VERY short wait
            while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();
      
            // read a packet from FIFO
            mpu.getFIFOBytes(fifoBuffer, packetSize);
            
            // track FIFO count here in case there is > 1 packet available
            // (this lets us immediately read more without waiting for an interrupt)
            fifoCount -= packetSize;
      
            // display Euler angles in degrees
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetGravity(&gravity, &q);
            mpu.dmpGetYawPitchRoll(aYPR, &q, &gravity);

            for(uint8_t i = 0; i < 3; ++i){
                aYPR[i] = aYPR[i] * 180/M_PI;
            }
        }
    }

    
    
    
}