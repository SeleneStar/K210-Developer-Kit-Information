#ifndef _ANGLE_H_
#define _ANGLE_H_

#include "icm_math.h"
#include "stdint.h"
#include "icm20607.h"

/* 向量 */
typedef struct _vector_t
{
    float x;
    float y;
    float z;
} vector_t;

/* 姿态角 */
typedef struct _attitude_t
{
    float roll;
    float pitch;
    float yaw;
} attitude_t;

typedef struct _quaternion_t
{ //四元数
    float q0;
    float q1;
    float q2;
    float q3;
} quaternion_t;

typedef struct _icm_data_t
{
    int16_t accX;
    int16_t accY;
    int16_t accZ;
    int16_t gyroX;
    int16_t gyroY;
    int16_t gyroZ;

    int16_t Offset[6];
} icm_data_t;

extern attitude_t g_attitude;
extern icm_data_t g_icm20607;

//函数声明
void get_attitude_angle(icm_data_t *p_icm, attitude_t *p_angle, float dt);
void reset_quaternion(void);

#endif /* _ANGLE_H_ */
