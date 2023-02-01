#include <gocator.hpp>

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

bool Gocator::receive(GoDataSet* data_set, uint64_t timeout) {
    return GoSystem_ReceiveData(system_, data_set, timeout) == kOK;
}

}  // namespace gocator