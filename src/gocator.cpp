#include <gocator.hpp>

#define NM_TO_MM(VALUE) (((k64f)(VALUE)) / 1000000.0)
#define UM_TO_MM(VALUE) (((k64f)(VALUE)) / 1000.0)

namespace gocator {

Gocator::Gocator(const std::string& ip_addr) {
    kStatus status;
    kIpAddress ip_address;

    // construct Gocator API Library
    if ((status = GoSdk_Construct(&api_)) != kOK) {
        throw GocatorError("GoSdk_Construct", status);
    }

    // construct GoSystem object
    if ((status = GoSystem_Construct(&system_, kNULL)) != kOK) {
        throw GocatorError("GoSystem_Construct", status);
    }

    // Parse IP address into address data structure
    kIpAddress_Parse(&ip_address, ip_addr.c_str());

    // obtain GoSensor object by sensor IP address
    if ((status = GoSystem_FindSensorByIpAddress(system_, &ip_address, &sensor_)) != kOK) {
        throw GocatorError("GoSystem_FindSensor", status);
    }

    // create connection to GoSensor object
    if ((status = GoSensor_Connect(sensor_)) != kOK) {
        throw GocatorError("GoSensor_Connect", status);
    }

    // enable sensor data channel
    if ((status = GoSystem_EnableData(system_, kTRUE)) != kOK) {
        throw GocatorError("GoSystem_EnableData", status);
    }

    // retrieve setup handle
    if ((setup_ = GoSensor_Setup(sensor_)) == kNULL) {
        throw GocatorError("GoSensor_Setup invalid handle", 0);
    }

    // retrieve total number of profile points prior to starting the sensor
    if (GoSetup_UniformSpacingEnabled(setup_)) {
        // Uniform spacing is enabled. The number is based on the X Spacing setting
        point_width_ = GoSetup_XSpacingCount(setup_, GO_ROLE_MAIN);
    } else {
        // non-uniform spacing is enabled. The max number is based on the number of columns used in the camera.
        point_width_ = GoSetup_FrontCameraWidth(setup_, GO_ROLE_MAIN);
    }
}

Gocator::~Gocator() {
    GoDestroy(system_);
    GoDestroy(api_);
}

void Gocator::stop_sensor() {
    kStatus status;
    if ((status = GoSystem_Stop(system_)) != kOK) {
        throw GocatorError("GoSystem_Stop", status);
    }
}

void Gocator::start_sensor() {
    kStatus status;
    if ((status = GoSystem_Start(system_)) != kOK) {
        throw GocatorError("GoSystem_Start", status);
    }
}

bool Gocator::receive_one(uint64_t timeout) {
    GoDataSet dataset = nullptr;
    if (GoSystem_ReceiveData(system_, &dataset, timeout) == kOK) {
        messages_.emplace_back();
        for (size_t i = 0; i < GoDataSet_Count(dataset); ++i) {
            auto msg = GoDataSet_At(dataset, i);
            parse_message(msg);
        }
        GoDestroy(dataset);
        return true;
    }
    return false;
}

void Gocator::reserve_message_size(size_t count) { messages_.reserve(count); }

void Gocator::parse_message(GoDataMsg msg) {
    auto& working = messages_.back();

    switch (GoDataMsg_Type(msg)) {
        case GO_DATA_MESSAGE_TYPE_STAMP: {
            GoStampMsg msg_as_stamp = msg;

            // I'm not sure why there would be multiple of these
            for (size_t i = 0; i < GoStampMsg_Count(msg_as_stamp); ++i) {
                const auto& stamp = GoStampMsg_At(msg_as_stamp, i);
                working.timestamp = stamp->timestamp;
                working.encoder = stamp->encoder;
                working.frame_index = stamp->frameIndex;
            }

            break;
        }
        case GO_DATA_MESSAGE_TYPE_PROFILE_POINT_CLOUD: {
            GoProfileMsg msg_as_profile = msg;
            // Reserve and create points
            size_t point_count = GoProfileMsg_Width(msg_as_profile);

            if (working.points.size() < point_count) {
                working.points.reserve(point_count);
                for (size_t pc_ = 0; pc_ < point_count; ++pc_) {
                    working.points.emplace_back();
                }
            }

            for (size_t k = 0; k < GoProfileMsg_Count(msg_as_profile); ++k) {
                kPoint16s* data = GoProfileMsg_At(msg_as_profile, k);
                double XResolution = NM_TO_MM(GoProfileMsg_XResolution(msg_as_profile));
                double ZResolution = NM_TO_MM(GoProfileMsg_ZResolution(msg_as_profile));
                double XOffset = UM_TO_MM(GoProfileMsg_XOffset(msg_as_profile));
                double ZOffset = UM_TO_MM(GoProfileMsg_ZOffset(msg_as_profile));

                working.x_res = XResolution;
                working.z_res = ZResolution;
                working.x_offset = XOffset;
                working.z_offset = ZOffset;

                // translate 16-bit range data to engineering units and copy profiles to memory array
                for (size_t i = 0; i < GoProfileMsg_Width(msg_as_profile); ++i) {
                    working.points[i].x = data[i].x;
                    working.points[i].z = data[i].y;
                }
            }
            break;
        }
        case GO_DATA_MESSAGE_TYPE_PROFILE_INTENSITY:
            GoProfileIntensityMsg msg_as_intensity = msg;

            // Reserve and create points
            size_t point_count = GoProfileIntensityMsg_Count(msg_as_intensity);

            if (working.points.size() < point_count) {
                working.points.reserve(point_count);
                for (size_t pc_ = 0; pc_ < point_count; ++pc_) {
                    working.points.emplace_back();
                }
            }

            for (size_t k = 0; k < GoProfileIntensityMsg_Count(msg_as_intensity); ++k) {
                unsigned char* data = GoProfileIntensityMsg_At(msg_as_intensity, k);
                for (size_t i = 0; i < GoProfileIntensityMsg_Width(msg_as_intensity); ++i) {
                    working.points[i].i = data[i];
                }
            }
            break;
    }
}

const std::vector<Message>& Gocator::messages() const { return messages_; }

}  // namespace gocator