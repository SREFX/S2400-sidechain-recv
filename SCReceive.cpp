#include "SCReceive.hpp"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

START_NAMESPACE_DISTRHO

SCReceive::SCReceive()
    : Plugin(DISTRHO_PLUGIN_NUM_PARAMS, 0, 0)
{
    // open shared RAM block as read only
    fSharedFd = shm_open("/s2400_sb", O_RDONLY, 0666);
    
    if (fSharedFd >= 0) {
        // map to pointer
        fSharedControl = (float*)mmap(0, sizeof(float) * 2, PROT_READ, MAP_SHARED, fSharedFd, 0);
    } else {
        fSharedControl = (float*)MAP_FAILED;
    }
}

SCReceive::~SCReceive() {
    // close memory safely when plugin is deleted
    if (fSharedControl != (float*)MAP_FAILED) {
        munmap(fSharedControl, sizeof(float)* 2);
    }
    if (fSharedFd >= 0) {
        close(fSharedFd);
    }
}

void SCReceive::run(const float** inputs, float** outputs, uint32_t frames) {
    // monoswap safety
    const float* inL = inputs[0]; 
    const float* inR = (inputs[1] != nullptr) ? inputs[1] : inputs[0]; 
    float* outL = outputs[0]; 
    bool hasRightOutput = (outputs[1] != nullptr);

    // fetch vol from SCSend, default to 1.0 if failure
    float incomingL = 1.0f;
    float incomingR = 1.0f;

    if (fSharedControl != (float*)MAP_FAILED) {
        incomingL = fSharedControl[0];
        incomingR = fSharedControl[1];
    }

    // mult volume by SCSend values from RAM
    for (uint32_t i = 0; i < frames; ++i) {
        outL[i] = inL[i] * incomingL;
        if (hasRightOutput) {
            outputs[1][i] = inR[i] * incomingR;
        }
    }
}

Plugin* createPlugin() { return new SCReceive(); }
END_NAMESPACE_DISTRHO