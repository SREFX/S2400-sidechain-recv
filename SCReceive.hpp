#ifndef SC_RECEIVE_HPP_INCLUDED
#define SC_RECEIVE_HPP_INCLUDED

#include "DistrhoPlugin.hpp"

START_NAMESPACE_DISTRHO

// struct to set RAM size
struct StereoSidechain {
    float left;
    float right;
};

class SCReceive : public Plugin {
public:
    SCReceive();
    ~SCReceive() override;

protected:
    const char* getLabel() const override { return "SCReceive"; }
    const char* getMaker() const override { return "Fxxxxx"; }
    const char* getLicense() const override { return "GPLv3"; }
    uint32_t getVersion() const override { return d_version(0, 0, 4); }
    int64_t getUniqueId() const override { return d_cconst('s', 'c', 'R', '4'); } 

    void initParameter(uint32_t index, Parameter& parameter) override;
    float getParameterValue(uint32_t index) const override;
    void setParameterValue(uint32_t index, float value) override;

    void run (const float** inputs, float** outputs, uint32_t frames) override;

private:
    //params
    float fBusParam;
    // POSIX shared memory variables
    StereoSidechain* fBuses[4];
    int fSharedFds[4];

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SCReceive)
};

END_NAMESPACE_DISTRHO
#endif