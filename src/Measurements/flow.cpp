//flow.cpp
#include <Arduino.h>
#include "flow.h"

Flow::Flow() : currentFlow(0.0f), antFlow(0.0f), flowPulse(0), flowPulseAnt(0), flowInterval(0), flowChanged(false), flowDataAvail(true) {}

void Flow::flowPulseChange() {
  flowPulse++;
  flowDataAvail = true;
}

bool Flow::flowDataAvailable() {
  return flowDataAvail;
}

float Flow::flowCalc() {
  antFlow = currentFlow;

  noInterrupts();
  unsigned long now = millis();
  if (flowInterval == 0) {
      flowInterval = now;
  }
  unsigned long deltaTime = now - flowInterval;
  unsigned long pulses = flowPulse;
  flowPulse = 0;
  interrupts();

  if (pulses == 0 && flowDataAvail) { // No new pulses, but new data available
      currentFlow = 0.0f;           // Explicitly set flow to zero
      flowChanged = true; // Always set flowChanged to true when going to zero
      flowInterval = now;
      flowDataAvail = false;
      return currentFlow;
  }

  if (pulses > 0 && flowDataAvail) { // New pulses, new data
      float flowRate = (float)((pulses / (float)flowPulsePerLiter) / (((float)deltaTime / millisInMin)));
      currentFlow = flowRate;
      flowChanged = (fabs(currentFlow - antFlow) > FLOW_CHANGE_THRESHOLD); // Check for significant change
      flowInterval = now;
      flowDataAvail = false;
      return currentFlow;
  }

  //No new pulses and no new data, keep the last value
  flowChanged = false;
  flowInterval = now;
  return currentFlow;
}



bool Flow::flowIsChanged() {
  return flowChanged;
}

void Flow::resetFlowChanged() {
    flowChanged = false;
}