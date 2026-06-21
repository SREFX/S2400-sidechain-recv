#include "SCReceive.hpp"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

START_NAMESPACE_DISTRHO

SCReceive::SCReceive()
    : Plugin(DISTRHO_PLUGIN_NUM_PARAMS, 0, 0),
    fBusParam(0.0f)
{
    // define 4 memory bus names
    const char* busNames[4] = {"/sidechain_bus_a", "/sidechain_bus_b", "/sidechain_bus_c", "/sidechain_bus_d"};

    // loop through and open all 4 busses
    for (int i = 0; i < 4; ++i) {
        // ask linux kernel to create/open a shared block of RAM
        fSharedFds[i] = shm_open(busNames[i], O_CREAT | O_RDWR, 0666);

        if (fSharedFds[i] >= 0) {
            // set the size to exactly 2 floats (8 bytes)
            ftruncate(fSharedFds[i], sizeof(StereoSidechain));

            // map that memory directly to pointer
            fBuses[i] = (StereoSidechain*)mmap(0, sizeof(StereoSidechain), PROT_READ | PROT_WRITE, MAP_SHARED, fSharedFds[i], 0);

            // init volume to 1.0 so SCRecv isn't muted by default
            if (fBuses[i] != (StereoSidechain*)MAP_FAILED) {
                fBuses[i]->left = 1.0f;
                fBuses[i]->right = 1.0f;
            }
        } else {
            fBuses[i] = (StereoSidechain*)MAP_FAILED;
        }
    }
}

SCReceive::~SCReceive() {
    // loop through and safely close all four buses
    for (int i = 0; i < 4; ++i) {
        // unlink and close memory safely when deleted
        if (fBuses[i] != (StereoSidechain*)MAP_FAILED) {
            munmap(fBuses[i], sizeof(StereoSidechain));
        }
        if (fSharedFds[i] >= 0) {
        close(fSharedFds[i]);
        }
    }
}

void SCReceive::initParameter(uint32_t index, Parameter& parameter) {
    parameter.hints = kParameterIsAutomatable;

    switch (index) {
        case 0:
        parameter.name = "Receive Bus";
        parameter.symbol = "receiveBus";

        parameter.hints = kParameterIsAutomatable | kParameterIsInteger;
        parameter.ranges.min = 0;
        parameter.ranges.max = 3;
        parameter.ranges.def = 0;

        parameter.enumValues.count = 4;
        parameter.enumValues.restrictedMode = true;
        parameter.enumValues.values = new ParameterEnumerationValue[4];

        parameter.enumValues.values[0].value = 0;
        parameter.enumValues.values[0].label = "A";

        parameter.enumValues.values[1].value = 1;
        parameter.enumValues.values[1].label = "B";

        parameter.enumValues.values[2].value = 2;
        parameter.enumValues.values[2].label = "C";

        parameter.enumValues.values[3].value = 3;
        parameter.enumValues.values[3].label = "D";
        break;
    }
}

float SCReceive::getParameterValue(uint32_t index) const {
    switch (index) {
        case 0: return fBusParam;
        default: return 0.0f;
    }
}

void SCReceive::setParameterValue(uint32_t index, float value) {
    switch (index) {
        case 0:
        fBusParam = value;
        break;
    }
}

void SCReceive::run(const float** inputs, float** outputs, uint32_t frames) {
    // monoswap safety
    const float* inL = inputs[0]; 
    const float* inR = (inputs[1] != nullptr) ? inputs[1] : inputs[0]; 
    float* outL = outputs[0]; 
    bool hasRightOutput = (outputs[1] != nullptr);

    // incoming bus
    int currentBus = (int)fBusParam;

    // fetch vol from SCSend, default to 1.0 if failure
    float scSignalL = 1.0f;
    float scSignalR = 1.0f;

    // read from active bus
    if (fBuses[currentBus] != (StereoSidechain*)MAP_FAILED) {
    scSignalL = fBuses[currentBus]->left;
    scSignalR = fBuses[currentBus]->right;
}

    // mult volume by SCSend values from RAM
    for (uint32_t i = 0; i < frames; ++i) {
        outL[i] = inL[i] * scSignalL;
        if (hasRightOutput) {
            outputs[1][i] = inR[i] * scSignalR;
        }
    }
}

Plugin* createPlugin() { return new SCReceive(); }
END_NAMESPACE_DISTRHO