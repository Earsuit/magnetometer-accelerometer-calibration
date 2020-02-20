#include <Arduino.h>
#include <Wire.h>

#define WRITE 0x00
#define READ 0x01

//MPU9250
#define MPU9250_AD 0x68
#define FIFO_EN_AD 0x23
#define PWR_MGMT_1_AD 0x6B
#define ACCEL_XOUT_H_AD 0x3B
#define GYRO_XOUT_H_AD 0x43
#define EXT_SENS_DATA_00_AD 0x49
#define ACCEL_CONFIG_1_AD 0x1C
#define ACCEL_CONFIG_2_AD 0x1D
#define GYRO_CONFIG_AD 0x1B
#define CONFIG_AD 0x1A
#define I2C_MST_CTRL_AD 0x24
#define I2C_SLV0_ADDR_AD 0x25
#define I2C_SLV0_REG_AD 0x26
#define I2C_SLV0_CTRL_AD 0x27
#define INT_BYPASS_CONFIG_AD 0x37
#define USER_CTRL_AD 0x6A
#define ACCEL_SENS 8192.0f
#define GYRO_SENS 32.8f

//Magnetometer
#define MAG_AD 0xC
#define WIA_AD 0x00
#define INFO 0x01
#define STATUS_1_AD 0x02
#define HXL_AD 0x03    //X-axis measurement data lower 8bit
#define HXH_AD 0x04    //X-axis measurement data higher 8bit
#define HYL_AD 0x05    //Y-axis measurement data lower 8bit
#define HYH_AD 0x06    //Y-axis measurement data higher 8bit
#define HZL_AD 0x07    //Z-axis measurement data lower 8bit
#define HZH_AD 0x08    //Z-axis measurement data higher 8bit
#define STATUS_2_AD 0x09
#define CNTL1_AD 0x0A   //control 1
#define CNTL2_AD 0x0B   //control 2
#define ASTC_AD 0x0C    //Self-Test Control
#define TS1_AD 0x0D    //test 1
#define TS2_AD 0x0E   //test 2
#define I2CDIS_AD 0x0F    //I2C disable
#define ASAX_AD 0x10    //Magnetic sensor X-axis sensitivity adjustment value
#define ASAY_AD 0x11    //Magnetic sensor Y-axis sensitivity adjustment value
#define ASAZ_AD 0x12    //Magnetic sensor Z-axis sensitivity adjustment value
#define MAGNE_SENS 6.67f
#define SCALE 0.1499f  // 4912/32760 uT/tick

#define Rad2Deg 57.2958f

//data union for transfer
typedef union Float{
    volatile float data;
    uint8_t bytes[4];
}Float;

volatile float asax,asay,asaz;
static Float accelX,accelY,accelZ,gyroX,gyroY,gyroZ;
static Float magneX, magneY,magneZ;

TwoWire i2c(1,I2C_FAST_MODE);

uint32_t prev = 0;


void MPU9250Setup(){
    i2c.beginTransmission(MPU9250_AD);
    i2c.write(PWR_MGMT_1_AD);
    i2c.write(0x01); //set the clock reference to X axis gyroscope to get a better accuracy
    i2c.endTransmission();

    i2c.beginTransmission(MPU9250_AD);
    i2c.write(ACCEL_CONFIG_1_AD);
    i2c.write(0x08); //set the accel scale to 4g
    i2c.endTransmission();

    i2c.beginTransmission(MPU9250_AD);
    i2c.write(ACCEL_CONFIG_2_AD);
    i2c.write(0x03);     //turn on the internal low-pass filter for accel with 44.8Hz bandwidth
    i2c.endTransmission();

    i2c.beginTransmission(MPU9250_AD);
    i2c.write(GYRO_CONFIG_AD);
    i2c.write(0x10); //set the gyro scale to 1000 degree/s and FCHOICE_B on
    i2c.endTransmission();

    // turn on the internal low-pass filter for gyro with 41Hz bandwidth
    i2c.beginTransmission(MPU9250_AD);
    i2c.write(CONFIG_AD);
    i2c.write(0x03);
    i2c.endTransmission();

    /*
        disable the I2C Master I/F module; pins ES_DA and ES_SCL are isolated
        from pins SDA/SDI and SCL/ SCLK.
    */
    i2c.beginTransmission(MPU9250_AD);
    i2c.write(USER_CTRL_AD);
    i2c.write(0x00);
    i2c.endTransmission();

    /*
        When asserted, the i2c_master interface pins(ES_CL and ES_DA) will go
        into bypass mode when the i2c master interface is disabled.
    */
    i2c.beginTransmission(MPU9250_AD);
    i2c.write(INT_BYPASS_CONFIG_AD);
    i2c.write(0x02);
    i2c.endTransmission();

    // setup the Magnetometer to fuse ROM access mode to get the Sensitivity
    // Adjustment values and 16-bit output
    i2c.beginTransmission(MAG_AD);
    i2c.write(CNTL1_AD);
    i2c.write(0x1F);
    i2c.endTransmission();

    //wait for the mode changes
    delay(100);

    //read the Sensitivit Adjustment values
    i2c.beginTransmission(MAG_AD);
    i2c.write(ASAX_AD);
    i2c.endTransmission(false);
    i2c.requestFrom(MAG_AD,3);
    asax = (i2c.read()-128)*0.5/128+1;
    asay = (i2c.read()-128)*0.5/128+1;
    asaz = (i2c.read()-128)*0.5/128+1;

    //reset the Magnetometer to power down mode
    i2c.beginTransmission(MAG_AD);
    i2c.write(CNTL1_AD);
    i2c.write(0x00);
    i2c.endTransmission();

    //wait for the mode changes
    delay(100);

    //set the Magnetometer to continuous mode 2(100Hz) and 16-bit output
    i2c.beginTransmission(MAG_AD);
    i2c.write(CNTL1_AD);
    i2c.write(0x16);
    i2c.endTransmission();

    //wait for the mode changes
    delay(100);
}

void readAccel(){
    //read the accelerate
    i2c.beginTransmission(MPU9250_AD);
    i2c.write(ACCEL_XOUT_H_AD);
    i2c.endTransmission();  
    i2c.requestFrom(MPU9250_AD,6);
    accelX.data = int16_t((i2c.read()<<8) | i2c.read());
    accelY.data = int16_t((i2c.read()<<8) | i2c.read());
    accelZ.data = int16_t((i2c.read()<<8) | i2c.read());
}

void readGyro(){
    //read the gyro
    i2c.beginTransmission(MPU9250_AD);
    i2c.write(GYRO_XOUT_H_AD);
    i2c.endTransmission();   
    i2c.requestFrom(MPU9250_AD,6);
    gyroX.data = int16_t(i2c.read()<<8) | i2c.read();
    gyroY.data = int16_t(i2c.read()<<8) | i2c.read();
    gyroZ.data = int16_t(i2c.read()<<8) | i2c.read();
}

void readMagnetometer(){
    i2c.beginTransmission(MAG_AD);
    i2c.write(STATUS_1_AD);
    i2c.endTransmission();   
    i2c.requestFrom(MAG_AD,1);
    if((i2c.read() & 0x01) == 0x01){
        i2c.beginTransmission(MAG_AD);
        i2c.write(HXL_AD);
        i2c.endTransmission();   
        i2c.requestFrom(MAG_AD,7);

        unsigned char buffer[7] = {0};
        for(int i=0;i<7;i++){
            buffer[i] = i2c.read();
        }

        if(!(buffer[6] & 0x8)){
          magneX.data = int16_t(buffer[0]) | int16_t(buffer[1]<<8);
          magneY.data = int16_t(buffer[2]) | int16_t(buffer[3]<<8);
          magneZ.data = int16_t(buffer[4]) | int16_t(buffer[5]<<8);
        }
    }
}

void setup() { 
    i2c.begin();
    MPU9250Setup();
    delay(1000);
}

void loop(){
    uint32_t now = micros();
    if(now - prev >= 10000){    //it takes 5ms to complete on a 72MHz processor
        prev = micros();

        readAccel();
        readGyro();
        readMagnetometer();

        accelX.data = accelX.data/ACCEL_SENS;
        accelY.data = accelY.data/ACCEL_SENS;
        accelZ.data = accelZ.data/ACCEL_SENS;

        gyroX.data = gyroX.data/GYRO_SENS;
        gyroY.data = gyroY.data/GYRO_SENS;
        gyroZ.data = gyroZ.data/GYRO_SENS;

        magneX.data = magneX.data*asax*SCALE;
        magneY.data = magneY.data*asay*SCALE;
        magneZ.data = magneZ.data*asaz*SCALE;

        Serial.write(85);
        Serial.write( accelX.bytes,4 );
        Serial.write( accelY.bytes,4 );
        Serial.write( accelZ.bytes,4 );
        Serial.write( gyroX.bytes,4 );
        Serial.write( gyroY.bytes,4 );
        Serial.write( gyroZ.bytes,4 );
        Serial.write( magneX.bytes,4 );
        Serial.write( magneY.bytes,4 );
        Serial.write( magneZ.bytes,4 );
        Serial.write(86);
    }
}

