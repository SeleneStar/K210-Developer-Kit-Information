#include "icm20607.h"
#include "i2c_ctl.h"
#include "sleep.h"

uint16_t gyro_scale = 0;
uint16_t acc_scale = 0;


/* I2C写数据 */
static void icm_i2c_write(uint8_t reg, uint8_t data)
{
    i2c_hd_write(ICM_ADDRESS, reg, data);
}

/* I2C读数据 */
static void icm_i2c_read(uint8_t reg, uint8_t *data_buf, uint16_t length)
{
    i2c_hd_read(ICM_ADDRESS, reg, data_buf, length);
}

/* 读取陀螺仪X轴原始数据 */
int16_t getRawGyroscopeX(void) {
    uint8_t val[2] = {0};
    icm_i2c_read(GYRO_XOUT_H, val, 2);
    return ((int16_t)val[0] << 8) + val[1];
}

/* 读取陀螺仪Y轴原始数据 */
int16_t getRawGyroscopeY(void) {
    uint8_t val[2] = {0};
    icm_i2c_read(GYRO_YOUT_H, val, 2);
    return ((int16_t)val[0] << 8) + val[1];
}

/* 读取陀螺仪Z轴原始数据 */
int16_t getRawGyroscopeZ(void) {
    uint8_t val[2] = {0};
    icm_i2c_read(GYRO_ZOUT_H, val, 2);
    return ((int16_t)val[0] << 8) + val[1];
}

/* 读取加速度计X轴原始数据 */
int16_t getRawAccelerationX(void) {
    uint8_t val[2] = {0};
    icm_i2c_read(ACCEL_XOUT_H, val, 2);
    return ((int16_t)val[0] << 8) + val[1];
}

/* 读取加速度计Y轴原始数据 */
int16_t getRawAccelerationY(void) {
    uint8_t val[2] = {0};
    icm_i2c_read(ACCEL_YOUT_H, val, 2);
    return ((int16_t)val[0] << 8) + val[1];
}

/* 读取加速度计Z轴原始数据 */
int16_t getRawAccelerationZ(void) {
    uint8_t val[2] = {0};
    icm_i2c_read(ACCEL_ZOUT_H, val, 2);
    return ((int16_t)val[0] << 8) + val[1];
}

/* 判断是否是icm20607芯片 */
void icm20607_who_am_i(void)
{
    uint8_t val, state = 1;
    do
    {
        icm_i2c_read(WHO_AM_I, &val, 1);  // 读ICM20607的ID

        if (ICM20607_ID != val & state)  // 当ID不对时，只报一次。
        {
            printf("WHO_AM_I=0x%02x\n", val);
            state = 0;
        }
    } while(ICM20607_ID != val);
    printf("WHO_AM_I=0x%02x\n", val);
}

/* 初始化icm20607芯片 */
void icm20607_init(void)
{
    uint8_t val = 0x0, res = 1;
    i2c_hardware_init(ICM_ADDRESS); // 初始化
    msleep(10);

    icm_i2c_write(PWR_MGMT_1, 0x80); //复位设备
    msleep(100);
    icm20607_who_am_i();

    do
    { //等待复位成功
        icm_i2c_read(PWR_MGMT_1, &val, 1);
    } while(0x41 != val);

    icm_i2c_write(PWR_MGMT_1, 0x01);     //时钟设置
    icm_i2c_write(PWR_MGMT_2, 0x00);     //开启陀螺仪和加速度计
    icm_i2c_write(CONFIG, 0x01);         //176HZ 1KHZ
    icm_i2c_write(SMPLRT_DIV, 0x07);     //采样速率 SAMPLE_RATE = INTERNAL_SAMPLE_RATE / (1 + SMPLRT_DIV)
    icm_i2c_write(GYRO_CONFIG, 0x18);    //±2000 dps
    icm_i2c_write(ACCEL_CONFIG, 0x10);   //±8g
    icm_i2c_write(ACCEL_CONFIG_2, 0x23); //Average 8 samples   44.8HZ
    return res;
}
