#include <GoSdk/GoSdk.h>

#include <chrono>
#include <memory>
#include <raw_message.hpp>
#include <vector>

#define RECEIVE_TIMEOUT (20000000)
#define SENSOR_IP "192.168.1.10"

#define NM_TO_MM(VALUE) (((k64f)(VALUE)) / 1000000.0)
#define UM_TO_MM(VALUE) (((k64f)(VALUE)) / 1000.0)

using namespace std::chrono_literals;

int main(int argc, char** argv) {
    kAssembly api = kNULL;
    kStatus status;
    unsigned int i, j, k, array_index;
    GoSystem system = kNULL;
    GoSensor sensor = kNULL;
    GoDataSet dataset = kNULL;
    std::vector<gocator::Message> messages;

    GoStamp* stamp = kNULL;
    GoDataMsg data_object;
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
    printf("count: %i\n", count);

    // Reserve space
    messages.reserve(3000);

    // start Gocator sensor
    if ((status = GoSystem_Start(system)) != kOK) {
        printf("Error: GoSensor_Start:%d\n", status);
        return 0;
    }

    auto start = std::chrono::steady_clock::now();

    while (true) {
        if (GoSystem_ReceiveData(system, &dataset, RECEIVE_TIMEOUT) == kOK) {
            messages.emplace_back();
            auto message = messages.back();
            message.points.reserve(count);
            for (size_t ic = 0; ic < count; ++ic) {
                message.points.emplace_back();
            }

            for (i = 0; i < GoDataSet_Count(dataset); ++i) {
                data_object = GoDataSet_At(dataset, i);
                // Retrieve GoStamp message
                switch (GoDataMsg_Type(data_object)) {
                    case GO_DATA_MESSAGE_TYPE_STAMP: {
                        GoStampMsg stampMsg = data_object;

                        // Stamp messages should only be one if there's just one sensor
                        for (j = 0; j < GoStampMsg_Count(stampMsg); ++j) {
                            stamp = GoStampMsg_At(stampMsg, j);
                            message.timestamp = stamp->timestamp;
                            message.encoder = stamp->encoder;
                            message.frame_index = stamp->frameIndex;
                        }
                    } break;
                    case GO_DATA_MESSAGE_TYPE_PROFILE_POINT_CLOUD:  // Note this is NON resampled profile
                    {
                        GoProfileMsg profile_msg = data_object;

                        // There should only be one if there's just one sensor
                        for (k = 0; k < GoProfileMsg_Count(profile_msg); ++k) {
                            kPoint16s* data = GoProfileMsg_At(profile_msg, k);
                            message.x_res = NM_TO_MM(GoProfileMsg_XResolution(profile_msg));
                            message.z_res = NM_TO_MM(GoProfileMsg_ZResolution(profile_msg));
                            message.x_offset = UM_TO_MM(GoProfileMsg_XOffset(profile_msg));
                            message.z_offset = UM_TO_MM(GoProfileMsg_ZOffset(profile_msg));

                            for (array_index = 0; array_index < GoProfileMsg_Width(profile_msg); ++array_index) {
                                if (array_index >= count) {
                                    message.points.emplace_back();
                                }

                                auto& p = message.points[array_index];
                                p.x = data[array_index].x;
                                p.z = data[array_index].y;
                            }
                        }
                    } break;
                    case GO_DATA_MESSAGE_TYPE_PROFILE_INTENSITY: {
                        GoProfileIntensityMsg intensity_msg = data_object;

                        for (k = 0; k < GoProfileIntensityMsg_Count(intensity_msg); ++k) {
                            unsigned char* data = GoProfileIntensityMsg_At(intensity_msg, k);
                            for (array_index = 0; array_index < GoProfileIntensityMsg_Width(intensity_msg);
                                 ++array_index) {
                                if (array_index >= count) {
                                    message.points.emplace_back();
                                }

                                message.points[array_index].i = data[array_index];
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

        if (messages.size() >= 2000) {
            auto elapsed = std::chrono::steady_clock::now() - start;
            auto elapsed_ms =
                static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
            printf("Rx %0.2f\n", static_cast<double>(messages.size()) / (elapsed_ms / 1000.0));

            std::ofstream out("test.data", std::ios::binary | std::ios::out);
            for (const auto& m : messages) {
                m.to_stream(out);
            }
            out.close();

            // Check
            std::ifstream in_file("test.data", std::ios::binary | std::ios::in);
            std::vector<gocator::Message> recorded;
            while (!in_file.eof()) {
                recorded.push_back(gocator::Message::from_stream(in_file));
            }

            for (int l = 0; l < messages.size(); ++l) {
                if (recorded[l].timestamp != messages[l].timestamp) printf("mis-match\n");
                if (recorded[l].frame_index != messages[l].frame_index) printf("mis-match\n");
                if (recorded[l].encoder != messages[l].encoder) printf("mis-match\n");
                if (recorded[l].x_res != messages[l].x_res) printf("mis-match\n");
                if (recorded[l].z_res != messages[l].z_res) printf("mis-match\n");
                if (recorded[l].x_offset != messages[l].x_offset) printf("mis-match\n");
                if (recorded[l].z_offset != messages[l].z_offset) printf("mis-match\n");

                for (int m = 0; m < messages[l].points.size(); ++m) {
                    if (recorded[l].points[m].x != messages[l].points[m].x) printf("mis-match\n");
                    if (recorded[l].points[m].z != messages[l].points[m].z) printf("mis-match\n");
                    if (recorded[l].points[m].i != messages[l].points[m].i) printf("mis-match\n");
                }
            }

            messages.clear();
            start = std::chrono::steady_clock::now();
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
