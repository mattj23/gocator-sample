#include <GoSdk/GoSdk.h>
#include <stdio.h>
#include <stdlib.h>

#include <gocator.hpp>
#include <iostream>
#include <vector>

#define RECEIVE_TIMEOUT (20000000)
#define INVALID_RANGE_16BIT \
    ((signed short)0x8000)  // gocator transmits range data as 16-bit signed integers. 0x8000 signifies invalid range
                            // data.
#define DOUBLE_MAX ((k64f)1.7976931348623157e+308)  // 64-bit double - largest positive value.
#define INVALID_RANGE_DOUBLE ((k64f)-DOUBLE_MAX)    // floating point value to represent invalid range data.
#define SENSOR_IP "192.168.1.10"

int main(int argc, char** argv) {
    gocator::Gocator sensor(SENSOR_IP);

    sensor.start_sensor();
    sensor.receive_one(RECEIVE_TIMEOUT);
    sensor.stop_sensor();

    for (const auto& m : sensor.messages()) {
        printf("Timestamp  = %lu\n", m.timestamp);
        printf("Frame      = %lu\n", m.frame_index);
        printf("Encoder    = %lu\n", m.encoder);

        printf("X Res      = %0.06f\n", m.x_res);
        printf("Z Res      = %0.06f\n", m.z_res);
        printf("X Offset   = %0.06f\n", m.x_offset);
        printf("Z Offset   = %0.06f\n", m.z_offset);

        for (const auto& p : m.points) {
            //            printf(" * %u %u %u\n", p.x, p.z, p.i);
            if (p.x == INVALID_RANGE_16BIT) continue;
            printf(" * %0.06f %0.06f %u\n", p.x * m.x_res + m.x_offset, p.z * m.z_res + m.z_offset, p.i);
        }
    }
    std::cout << std::endl;

    return 0;
}
