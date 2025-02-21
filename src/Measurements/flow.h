#ifndef FLOW_H
#define FLOW_H

#define FLOW_CHANGE_THRESHOLD 1.0f // Define a threshold for flow rate change

class Flow {
public:
    Flow();
    float flowCalc();
    bool flowIsChanged();
    bool flowDataAvailable(); 
    void flowPulseChange();
    void resetFlowChanged(); 

private:
    float currentFlow, antFlow;
    volatile unsigned long flowPulse;
    unsigned long flowPulseAnt;
    unsigned long flowInterval; // Should be unsigned long for millis()
    const float flowPulsePerLiter = 384; // Keep const if it is a constant value
    bool flowChanged, flowDataAvail; // Add a new variable to check if data is available
    const float millisInMin=60000;
};

#endif // FLOW_H
