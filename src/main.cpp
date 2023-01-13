#include <GoSdk/GoSdk.h>

#include <memory>
#include <raw_message.hpp>
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

int main(int argc, char** argv) {
    kAssembly api = kNULL;
    kStatus status;
    unsigned int i, j, k, arrayIndex;
    GoSystem system = kNULL;
    GoSensor sensor = kNULL;
    GoDataSet dataset = kNULL;
    std::vector<gocator::Message> messages;

    GoStamp* stamp = kNULL;
    GoDataMsg dataObj;
    kIpAddress ipAddress;
    GoSetup setup = kNULL;
    uint32_t count;

    // construct Gocator API Library
    if ((status = GoSdk_Construct(&api)) != kOK) {
        printf("Error: GoSdk_Construct:%d\n", status);
        return 0;
    }

    // construct GoSystem object
    if ((status = GoSystem_Construct(&system, kNULL)) != kOK) {
        printf("Error: GoSystem_Construct:%d\n", status);
        return 0;
    }

    // Parse IP address into address data structure
    kIpAddress_Parse(&ipAddress, SENSOR_IP);

    // obtain GoSensor object by sensor IP address
    if ((status = GoSystem_FindSensorByIpAddress(system, &ipAddress, &sensor)) != kOK) {
        printf("Error: GoSystem_FindSensor:%d\n", status);
        return 0;
    }

    // create connection to GoSensor object
    if ((status = GoSensor_Connect(sensor)) != kOK) {
        printf("Error: GoSensor_Connect:%d\n", status);
        return 0;
    }
    printf("Connect status %d\n", status);

    // enable sensor data channel
    if ((status = GoSystem_EnableData(system, kTRUE)) != kOK) {
        printf("Error: GoSensor_EnableData:%d\n", status);
        return 0;
    }

    // retrieve setup handle
    if ((setup = GoSensor_Setup(sensor)) == kNULL) {
        printf("Error: GoSensor_Setup: Invalid Handle\n");
    }

    // retrieve total number of profile points prior to starting the sensor
    if (GoSetup_UniformSpacingEnabled(setup)) {
        // Uniform spacing is enabled. The number is based on the X Spacing setting
        count = GoSetup_XSpacingCount(setup, GO_ROLE_MAIN);
    } else {
        // non-uniform spacing is enabled. The max number is based on the number of columns used in the camera.
        count = GoSetup_FrontCameraWidth(setup, GO_ROLE_MAIN);
    }

    // Reserve space
    messages.reserve(3000);

    // start Gocator sensor
    if ((status = GoSystem_Start(system)) != kOK) {
        printf("Error: GoSensor_Start:%d\n", status);
        return 0;
    }

    while (true) {
        if (GoSystem_ReceiveData(system, &dataset, RECEIVE_TIMEOUT) == kOK) {
            printf("Data message received:\n");
            printf("Dataset count: %u\n", (k32u)GoDataSet_Count(dataset));
            // each result can have multiple data items
            // loop through all items in result message
            messages.emplace_back();
            auto message = messages.back();
            message.points.reserve(count);

            for (i = 0; i < GoDataSet_Count(dataset); ++i) {
                dataObj = GoDataSet_At(dataset, i);
                // Retrieve GoStamp message
                switch (GoDataMsg_Type(dataObj)) {
                    case GO_DATA_MESSAGE_TYPE_STAMP: {
                        GoStampMsg stampMsg = dataObj;

                        printf("Stamp Message batch count: %u\n", (k32u)GoStampMsg_Count(stampMsg));
                        for (j = 0; j < GoStampMsg_Count(stampMsg); ++j) {
                            stamp = GoStampMsg_At(stampMsg, j);
                            printf("  Timestamp: %llu\n", stamp->timestamp);
                            printf("  Encoder: %lld\n", stamp->encoder);
                            printf("  Frame index: %llu\n", stamp->frameIndex);
                        }
                    } break;
                    case GO_DATA_MESSAGE_TYPE_UNIFORM_PROFILE: {
                        GoResampledProfileMsg profileMsg = dataObj;

                        printf("Resampled Profile Message batch count: %u\n",
                               (k32u)GoResampledProfileMsg_Count(profileMsg));

                        for (k = 0; k < GoResampledProfileMsg_Count(profileMsg); ++k) {
                            unsigned int validPointCount = 0;
                            short* data = GoResampledProfileMsg_At(profileMsg, k);
                            double XResolution = NM_TO_MM(GoResampledProfileMsg_XResolution(profileMsg));
                            double ZResolution = NM_TO_MM(GoResampledProfileMsg_ZResolution(profileMsg));
                            double XOffset = UM_TO_MM(GoResampledProfileMsg_XOffset(profileMsg));
                            double ZOffset = UM_TO_MM(GoResampledProfileMsg_ZOffset(profileMsg));

                            // translate 16-bit range data to engineering units and copy profiles to memory array
                            for (arrayIndex = 0; arrayIndex < GoResampledProfileMsg_Width(profileMsg); ++arrayIndex) {
                                if (data[arrayIndex] != INVALID_RANGE_16BIT) {
                                    double x = XOffset + XResolution * arrayIndex;
                                    double z = ZOffset + ZResolution * data[arrayIndex];
                                    validPointCount++;
                                } else {
                                    double x = XOffset + XResolution * arrayIndex;
                                    double z = INVALID_RANGE_DOUBLE;
                                }
                            }
                            printf("  Profile Valid Point %d out of max %d\n", validPointCount, count);
                        }
                    } break;
                    case GO_DATA_MESSAGE_TYPE_PROFILE_POINT_CLOUD:  // Note this is NON resampled profile
                    {
                        GoProfileMsg profileMsg = dataObj;

                        printf("Profile Message batch count: %u\n", (k32u)GoProfileMsg_Count(profileMsg));

                        for (k = 0; k < GoProfileMsg_Count(profileMsg); ++k) {
                            kPoint16s* data = GoProfileMsg_At(profileMsg, k);
                            unsigned int validPointCount = 0;
                            double XResolution = NM_TO_MM(GoProfileMsg_XResolution(profileMsg));
                            double ZResolution = NM_TO_MM(GoProfileMsg_ZResolution(profileMsg));
                            double XOffset = UM_TO_MM(GoProfileMsg_XOffset(profileMsg));
                            double ZOffset = UM_TO_MM(GoProfileMsg_ZOffset(profileMsg));

                            // translate 16-bit range data to engineering units and copy profiles to memory array
                            for (arrayIndex = 0; arrayIndex < GoProfileMsg_Width(profileMsg); ++arrayIndex) {
                                if (data[arrayIndex].x != INVALID_RANGE_16BIT) {
                                    double x = XOffset + XResolution * data[arrayIndex].x;
                                    double z = ZOffset + ZResolution * data[arrayIndex].y;
                                    validPointCount++;
                                } else {
                                    double x = INVALID_RANGE_DOUBLE;
                                    double z = INVALID_RANGE_DOUBLE;
                                }
                            }
                            printf("  Profile Valid Point %d out of max %d\n", validPointCount, count);
                        }
                    } break;
                    case GO_DATA_MESSAGE_TYPE_PROFILE_INTENSITY: {
                        // kSize validPointCount = 0;
                        GoProfileIntensityMsg intensityMsg = dataObj;
                        printf("Intensity Message batch count: %u\n", (k32u)GoProfileIntensityMsg_Count(intensityMsg));

                        for (k = 0; k < GoProfileIntensityMsg_Count(intensityMsg); ++k) {
                            unsigned char* data = GoProfileIntensityMsg_At(intensityMsg, k);
                            for (arrayIndex = 0; arrayIndex < GoProfileIntensityMsg_Width(intensityMsg); ++arrayIndex) {
                                unsigned char intensity = data[arrayIndex];
                            }
                        }
                    } break;
                }
            }
            GoDestroy(dataset);
        } else {
            printf("Error: No data received during the waiting period\n");
            break;
        }
    }

    // stop Gocator sensor
    if ((status = GoSystem_Stop(system)) != kOK) {
        printf("Error: GoSensor_Stop:%d\n", status);
        return 0;
    }

    // destroy handles
    GoDestroy(system);
    GoDestroy(api);

    printf("Press any key to continue...\n");
    getchar();
    return 0;
}
