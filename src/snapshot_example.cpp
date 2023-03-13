/*
    This is a sample file that at one point I used with one of the LMI snapshot scanners.

    I'm putting it here in case it might be useful, but it was several years ago and I don't remember exactly how it worked or why I made it the way I did.
*/

#include <GoSdk/GoSdk.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <string>
#include <sstream>
#include <ctime>

#include <open3d/Open3D.h>

using namespace std::chrono_literals;
using namespace open3d::geometry;
using open3d::io::WritePointCloudToPCD;
using PCOption = open3d::io::WritePointCloudOption;

std::atomic<bool> got_data{false};

struct Context {
    unsigned int exposure;
    std::string name;
};

#define NM_TO_MM(VALUE) (((k64f)(VALUE))/1000000.0)
#define UM_TO_MM(VALUE) (((k64f)(VALUE))/1000.0)
#define INVALID_RANGE_DOUBLE    ((k64f)-DOUBLE_MAX)             // floating point value to represent invalid range data.
#define INVALID_RANGE_16BIT     ((signed short)0x8000)          // gocator transmits range data as 16-bit signed integers. 0x8000 signifies invalid range data.

kStatus kCall handle_data(void* ctx, void* sys, void* dataset) {

    unsigned int i, j;
    auto context = reinterpret_cast<Context*>(ctx);

//    std::cout << " > message count: " << (k32u)GoDataSet_Count(dataset) << std::endl;

    auto cloud = std::make_shared<PointCloud>();

    for (i = 0; i < GoDataSet_Count(dataset); ++i)
    {
        GoDataMsg dataObj = GoDataSet_At(dataset, i);
        std::cout << " > message " << i << " type = " << GoDataMsg_Type(dataObj) << std::endl;
        // retrieve GoStamp message
        switch (GoDataMsg_Type(dataObj))
        {
            case GO_DATA_MESSAGE_TYPE_STAMP:
            {
                GoStampMsg stampMsg = dataObj;
                for (j = 0; j < GoStampMsg_Count(stampMsg); ++j)
                {
                    GoStamp *stamp = GoStampMsg_At(stampMsg, j);
                    std::cout << "   - timestamp: " << stamp->timestamp << std::endl;
                    std::cout << "   - encoder: " << stamp->encoder << std::endl;
                    std::cout << "   - frame index: " << stamp->frameIndex << std::endl;
                }
                break;
            }

            case GO_DATA_MESSAGE_TYPE_SURFACE_POINT_CLOUD:
            {

                GoSurfacePointCloudMsg message = dataObj;

                double x_res = NM_TO_MM(GoSurfacePointCloudMsg_XResolution(message));
                double y_res = NM_TO_MM(GoSurfacePointCloudMsg_YResolution(message));
                double z_res = NM_TO_MM(GoSurfacePointCloudMsg_ZResolution(message));
                double x_off = UM_TO_MM(GoSurfacePointCloudMsg_XOffset(message));
                double y_off = UM_TO_MM(GoSurfacePointCloudMsg_YOffset(message));
                double z_off = UM_TO_MM(GoSurfacePointCloudMsg_ZOffset(message));

                for (unsigned int row = 0; row < GoSurfacePointCloudMsg_Length(message); ++row) {
                    kPoint3d16s *row_data = GoSurfacePointCloudMsg_RowAt(message, row);
                    for (unsigned int col = 0; col <= GoSurfacePointCloudMsg_Width(message); ++col) {
                        double x = x_off + x_res * row_data[col].x;
                        double y = y_off + y_res * row_data[col].y;
                        if (row_data[col].z != INVALID_RANGE_16BIT) {
                            double z = z_off + z_res * row_data[col].z;
                            cloud->points_.emplace_back(x, y, z);
                        }
                    }
                }

                std::cout << "   - points: " << cloud->points_.size() << std::endl;
                break;
            }
        }
        // Refer to example ReceiveRange, ReceiveProfile, ReceiveMeasurement and ReceiveWholePart on how to receive data
    }
    GoDestroy(dataset);

    // Output file
    std::ostringstream name_stream{};
    auto now = std::chrono::system_clock::now();
    auto time_result = std::time(nullptr);
    name_stream << "/root/data/" << context->name;
    name_stream << "-" << time_result;
    name_stream << "-" << context->exposure;
    name_stream << ".pcd";

    WritePointCloudToPCD(name_stream.str(), *cloud,
                         {PCOption::IsAscii::Binary, PCOption::Compressed::Compressed, false, {}});

    got_data = true;
    return kOK;
}

void display_result(kStatus status, const char* message) {
    std::cout << message << " result: " << status << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return -1;
    }

    Context context{};
    context.name = argv[1];

    std::cout << "Test set = " << context.name << std::endl;

    kAssembly api = nullptr;
    kIpAddress ip_address;
    GoSystem system;
    GoSensor sensor;
    GoSetup setup;
    GoAdvanced advanced;

    std::vector<unsigned int> exposure_times{500, 1000, 2000, 4000, 8000, 16000, 32000, 64000, 128000};

    // Construct the API
    int api_status;
    if ((api_status = GoSdk_Construct(&api)) != kOK) {
        std::cout << "Error during API construction: " << api_status << std::endl;
        return -1;
    }

    // Construct the system and connect to the sensor
    GoSystem_Construct(&system, nullptr);
    kIpAddress_Parse(&ip_address, "192.168.1.10");
    display_result(GoSystem_FindSensorByIpAddress(system, &ip_address, &sensor), "Find sensor");
    display_result(GoSensor_Connect(sensor), "Connect to sensor");

    display_result(GoSensor_Stop(sensor), "Stop sensor");

    // Get setup
    setup = GoSensor_Setup(sensor);
    advanced = GoSetup_Advanced(setup, GO_ROLE_MAIN);
    display_result(GoSetup_SetTriggerSource(setup, GO_TRIGGER_SOFTWARE), "Set trigger");
    display_result(GoSetup_SetScanMode(setup, GO_MODE_SURFACE), "Set surface mode");
    display_result(GoSetup_EnableUniformSpacing(setup, false), "Remove uniform surface mode");
    display_result(GoSetup_SetExposureMode(setup, GO_ROLE_MAIN, GO_EXPOSURE_MODE_SINGLE), "Set exposure mode");

    display_result(GoSystem_SetDataHandler(system, handle_data, &context), "Set handler");
    display_result(GoSystem_EnableData(system, true), "Enable data");

    for (auto exposure : exposure_times) {
        context.exposure = exposure;
        std::cout << "Test exposure time " << exposure << std::endl;
        display_result(GoSetup_SetExposure(setup, GO_ROLE_MAIN, exposure), " > Set exposure");
        display_result(GoSensor_Start(sensor), " > Start sensor");

        got_data = false;
        std::this_thread::sleep_for(1000ms);
        display_result(GoSensor_Trigger(sensor), " > Trigger");

        while (!got_data) {
            std::this_thread::sleep_for(1000ms);
            std::cout << "  ...waiting" << std::endl;
        }

        std::cout << " > delay" << std::endl;
        std::this_thread::sleep_for(5000ms);
        display_result(GoSensor_Stop(sensor), " > Stop sensor");
    }

    std::cout << "Complete" << std::endl;
    kObject_Destroy(system);
    kObject_Destroy(api);

    return 0;
}
