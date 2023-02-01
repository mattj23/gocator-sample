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

#define NM_TO_MM(VALUE) (((k64f)(VALUE)) / 1000000.0)
#define UM_TO_MM(VALUE) (((k64f)(VALUE)) / 1000.0)

typedef struct ProfilePoint {
    double x;  // x-coordinate in engineering units (mm) - position along laser line
    double z;  // z-coordinate in engineering units (mm) - height (at the given x position)
    unsigned char intensity;
} ProfilePoint;

int main(int argc, char** argv) {
    //    kAssembly api = kNULL;
    //    kStatus status;
//    unsigned int i, j, k, arrayIndex;
    //    GoSystem system = kNULL;
    //    GoSensor sensor = kNULL;
//    GoDataSet dataset = kNULL;
//    ProfilePoint* profileBuffer = NULL;
//    GoStamp* stamp = kNULL;
//    GoDataMsg dataObj;
    //    kIpAddress ipAddress;
    //    GoSetup setup = kNULL;
    //    k32u profilePointCount;

//    std::vector<gocator::Message> messages;

    //    // construct Gocator API Library
    //    if ((status = GoSdk_Construct(&api)) != kOK) {
    //        printf("Error: GoSdk_Construct:%d\n", status);
    //        return 1;
    //    }
    //
    //    // construct GoSystem object
    //    if ((status = GoSystem_Construct(&system, kNULL)) != kOK) {
    //        printf("Error: GoSystem_Construct:%d\n", status);
    //        return 1;
    //    }
    //
    //    // Parse IP address into address data structure
    //    kIpAddress_Parse(&ipAddress, SENSOR_IP);
    //
    //    // obtain GoSensor object by sensor IP address
    //    if ((status = GoSystem_FindSensorByIpAddress(system, &ipAddress, &sensor)) != kOK) {
    //        printf("Error: GoSystem_FindSensor:%d\n", status);
    //        return 1;
    //    }
    //
    //    // create connection to GoSensor object
    //    if ((status = GoSensor_Connect(sensor)) != kOK) {
    //        printf("Error: GoSensor_Connect:%d\n", status);
    //        return 1;
    //    }
    //
    //    // enable sensor data channel
    //    if ((status = GoSystem_EnableData(system, kTRUE)) != kOK) {
    //        printf("Error: GoSensor_EnableData:%d\n", status);
    //        return 1;
    //    }
    //
    //    // retrieve setup handle
    //    if ((setup = GoSensor_Setup(sensor)) == kNULL) {
    //        printf("Error: GoSensor_Setup: Invalid Handle\n");
    //    }
    //
    //    // retrieve total number of profile points prior to starting the sensor
    //    if (GoSetup_UniformSpacingEnabled(setup)) {
    //        // Uniform spacing is enabled. The number is based on the X Spacing setting
    //        profilePointCount = GoSetup_XSpacingCount(setup, GO_ROLE_MAIN);
    //    } else {
    //        // non-uniform spacing is enabled. The max number is based on the number of columns used in the camera.
    //        profilePointCount = GoSetup_FrontCameraWidth(setup, GO_ROLE_MAIN);
    //    }
    //
    //    //    if ((profileBuffer = malloc(profilePointCount * sizeof(ProfilePoint))) == kNULL)
    //    if ((profileBuffer = new ProfilePoint()) == kNULL) {
    //        printf("Error: Cannot allocate profileData, %d points\n", profilePointCount);
    //        return 1;
    //    }
    //
    //    // start Gocator sensor
    //    if ((status = GoSystem_Start(system)) != kOK) {
    //        printf("Error: GoSensor_Start:%d\n", status);
    //        return 1;
    //    }
    gocator::Gocator sensor(SENSOR_IP);
    sensor.start_sensor();

    //    if (GoSystem_ReceiveData(system, &dataset, RECEIVE_TIMEOUT) == kOK) {
    //    if (sensor.receive(&dataset, RECEIVE_TIMEOUT)) {
    //        printf("Data message received:\n");
    //        printf("Dataset count: %u\n", (k32u)GoDataSet_Count(dataset));
    //        messages.emplace_back();
    //        auto& working_message = messages.back();
    //
    //        // each result can have multiple data items
    //        // loop through all items in result message
    //        for (i = 0; i < GoDataSet_Count(dataset); ++i) {
    //            dataObj = GoDataSet_At(dataset, i);
    //            // Retrieve GoStamp message
    //            switch (GoDataMsg_Type(dataObj)) {
    //                case GO_DATA_MESSAGE_TYPE_STAMP: {
    //                    GoStampMsg stampMsg = dataObj;
    //
    //                    printf("Stamp Message batch count: %u\n", (k32u)GoStampMsg_Count(stampMsg));
    //                    for (j = 0; j < GoStampMsg_Count(stampMsg); ++j) {
    //                        stamp = GoStampMsg_At(stampMsg, j);
    //                        printf("  Timestamp: %llu\n", stamp->timestamp);
    //                        printf("  Encoder: %lld\n", stamp->encoder);
    //                        printf("  Frame index: %llu\n", stamp->frameIndex);
    //
    //                        working_message.timestamp = stamp->timestamp;
    //                        working_message.encoder = stamp->encoder;
    //                        working_message.frame_index = stamp->frameIndex;
    //                    }
    //                } break;
    //                case GO_DATA_MESSAGE_TYPE_UNIFORM_PROFILE: {
    //                    GoResampledProfileMsg profileMsg = dataObj;
    //
    //                    printf("Resampled Profile Message batch count: %u\n",
    //                           (k32u)GoResampledProfileMsg_Count(profileMsg));
    //
    //                    //                    for (k = 0; k < GoResampledProfileMsg_Count(profileMsg); ++k) {
    //                    //                        unsigned int validPointCount = 0;
    //                    //                        short* data = GoResampledProfileMsg_At(profileMsg, k);
    //                    //                        double XResolution =
    //                    //                        NM_TO_MM(GoResampledProfileMsg_XResolution(profileMsg)); double
    //                    //                        ZResolution =
    //                    NM_TO_MM(GoResampledProfileMsg_ZResolution(profileMsg));
    //                    //                        double XOffset =
    //                    UM_TO_MM(GoResampledProfileMsg_XOffset(profileMsg));
    //                    //                        double ZOffset =
    //                    UM_TO_MM(GoResampledProfileMsg_ZOffset(profileMsg));
    //                    //
    //                    //                        // translate 16-bit range data to engineering units and copy
    //                    profiles to
    //                    //                        memory array for (arrayIndex = 0; arrayIndex <
    //                    //                        GoResampledProfileMsg_Width(profileMsg); ++arrayIndex) {
    //                    //                            if (data[arrayIndex] != INVALID_RANGE_16BIT) {
    //                    //                                profileBuffer[arrayIndex].x = XOffset + XResolution *
    //                    arrayIndex;
    //                    //                                profileBuffer[arrayIndex].z = ZOffset + ZResolution *
    //                    //                                data[arrayIndex]; validPointCount++;
    //                    //                            } else {
    //                    //                                profileBuffer[arrayIndex].x = XOffset + XResolution *
    //                    arrayIndex;
    //                    //                                profileBuffer[arrayIndex].z = INVALID_RANGE_DOUBLE;
    //                    //                            }
    //                    //                        }
    //                    //                        printf("  Profile Valid Point %d out of max %d\n", validPointCount,
    //                    //                        profilePointCount);
    //                    //                    }
    //                } break;
    //                case GO_DATA_MESSAGE_TYPE_PROFILE_POINT_CLOUD:  // Note this is NON resampled profile
    //                {
    //                    GoProfileMsg profileMsg = dataObj;
    //
    //                    printf("Profile Message batch count: %u\n", (k32u)GoProfileMsg_Count(profileMsg));
    //
    //                    // Reserve and create points
    //                    size_t point_count = GoProfileMsg_Width(profileMsg);
    //
    //                    if (working_message.points.size() < point_count) {
    //                        working_message.points.reserve(point_count);
    //                        for (size_t pc_ = 0; pc_ < point_count; ++pc_) {
    //                            working_message.points.emplace_back();
    //                        }
    //                    }
    //
    //                    for (k = 0; k < GoProfileMsg_Count(profileMsg); ++k) {
    //                        kPoint16s* data = GoProfileMsg_At(profileMsg, k);
    //                        unsigned int validPointCount = 0;
    //                        double XResolution = NM_TO_MM(GoProfileMsg_XResolution(profileMsg));
    //                        double ZResolution = NM_TO_MM(GoProfileMsg_ZResolution(profileMsg));
    //                        double XOffset = UM_TO_MM(GoProfileMsg_XOffset(profileMsg));
    //                        double ZOffset = UM_TO_MM(GoProfileMsg_ZOffset(profileMsg));
    //
    //                        working_message.x_res = XResolution;
    //                        working_message.z_res = ZResolution;
    //                        working_message.x_offset = XOffset;
    //                        working_message.z_offset = ZOffset;
    //
    //                        // translate 16-bit range data to engineering units and copy profiles to memory array
    //                        for (arrayIndex = 0; arrayIndex < GoProfileMsg_Width(profileMsg); ++arrayIndex) {
    //                            working_message.points[arrayIndex].x = data[arrayIndex].x;
    //                            working_message.points[arrayIndex].z = data[arrayIndex].y;
    //
    //                            //                            if (data[arrayIndex].x != INVALID_RANGE_16BIT) {
    //                            //                                profileBuffer[arrayIndex].x = XOffset + XResolution
    //                            *
    //                            //                                data[arrayIndex].x; profileBuffer[arrayIndex].z =
    //                            ZOffset
    //                            //                                + ZResolution * data[arrayIndex].y;
    //                            validPointCount++;
    //                            //                            } else {
    //                            //                                profileBuffer[arrayIndex].x = INVALID_RANGE_DOUBLE;
    //                            //                                profileBuffer[arrayIndex].z = INVALID_RANGE_DOUBLE;
    //                            //                            }
    //                        }
    //                        //                        printf("  Profile Valid Point %d out of max %d\n",
    //                        validPointCount,
    //                        //                        profilePointCount);
    //                    }
    //                } break;
    //                case GO_DATA_MESSAGE_TYPE_PROFILE_INTENSITY: {
    //                    // kSize validPointCount = 0;
    //                    GoProfileIntensityMsg intensityMsg = dataObj;
    //                    printf("Intensity Message batch count: %u\n",
    //                    (k32u)GoProfileIntensityMsg_Count(intensityMsg));
    //
    //                    // Reserve and create points
    //                    size_t point_count = GoProfileIntensityMsg_Count(intensityMsg);
    //
    //                    if (working_message.points.size() < point_count) {
    //                        working_message.points.reserve(point_count);
    //                        for (size_t pc_ = 0; pc_ < point_count; ++pc_) {
    //                            working_message.points.emplace_back();
    //                        }
    //                    }
    //
    //                    for (k = 0; k < GoProfileIntensityMsg_Count(intensityMsg); ++k) {
    //                        unsigned char* data = GoProfileIntensityMsg_At(intensityMsg, k);
    //                        for (arrayIndex = 0; arrayIndex < GoProfileIntensityMsg_Width(intensityMsg); ++arrayIndex)
    //                        {
    //                            working_message.points[arrayIndex].i = data[arrayIndex];
    //                            //                            profileBuffer[arrayIndex].intensity = data[arrayIndex];
    //                        }
    //                    }
    //                } break;
    //            }
    //        }
    //        GoDestroy(dataset);
    //    } else {
    //        printf("Error: No data received during the waiting period\n");
    //    }

    sensor.receive_one(RECEIVE_TIMEOUT);

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

    sensor.stop_sensor();

    //    // stop Gocator sensor
    //    if ((status = GoSystem_Stop(system)) != kOK) {
    //        printf("Error: GoSensor_Stop:%d\n", status);
    //        return 1;
    //    }
    //
    //    // destroy handles
    //    GoDestroy(system);
    //    GoDestroy(api);
    //    free(profileBuffer);

    printf("Press any key to continue...\n");
    getchar();
    return 0;
}
