#include <SdFatConfig.h>
#include <FreeStack.h>
#include <MinimumSerial.h>
#include <SdFat.h>
#include <BlockDriver.h>
#include <SysCall.h>

#include <TimerThree.h>

#include "DataArray.h"
#include "AirMassEstimator.h"
#include "FuelMassEstimator.h"
#include "numericalIntegration.h"
#include "datalog.h"
#include "constants.h"


FuelMassEstimator fuelmass;
DataArray *Timestamps, *MAP, *CRANK, *TPS, *FPS, *ECT, *IAT, *IAP, *Reynolds;
datalog logger = datalog(false, Timestamps, MAP, CRANK, TPS, FPS, ECT, IAT, IAP);

//The NULL and 1 parameters here still need to be filled
AirMassEstimator airmass = AirMassEstimator(nullptr, nullptr, 1, 1, 1, 1, 1, 1, Timestamps, MAP, CRANK, Reynolds, TPS, IAP, IAT);

bool firstCycle = true;

void setup() {
	logger.setup();

	// Attach the interrupt for INJ pulse modulation.
	// For some reason, the internal interrupt flags can end up defaulting
	// to a triggered state before they are attached. This causes them
	// to trigger once right when they are attached. Our current workaround
	// is to attach the interrupt to a dummy function first that triggers
	// if the interrupt is already set. Then, it is safe to attach the normal interrupt.
	Timer3.attachInterrupt(dummy);
	Timer3.detachInterrupt();
	Timer3.attachInterrupt(handle_pulseTimerTimeout);

	// For some reason, the internal interrupt flags can end up defaulting
	// to a triggered state before they are attached. This causes them
	// to trigger once right when they are attached. Our current workaround
	// is to attach the interrupt to a dummy function first that triggers
	// if the interrupt is already set. Then, it is safe to attach the normal interrupt.

	//IVO and IVC pins need to be defined
	attachInterrupt(digitalPinToInterrupt(IVO_PIN), dummy, RISING);
	detachInterrupt(digitalPinToInterrupt(IVO_PIN));
	attachInterrupt(digitalPinToInterrupt(IVO_PIN), handle_IVO, RISING);

	attachInterrupt(digitalPinToInterrupt(IVC_PIN), dummy, RISING);
	detachInterrupt(digitalPinToInterrupt(IVC_PIN));
	attachInterrupt(digitalPinToInterrupt(IVC_PIN), handle_IVC, RISING);

}

void loop() {
	if (firstCycle)
	{
		digitalWrite(MAP_IVO_PIN, HIGH);
		digitalWrite(MAP_IVO_PIN, LOW);
		firstCycle = false;
	}
	logger.loopfunction();
}

//End Injection
void handle_pulseTimerTimeout()
{
	digitalWrite(INJ_PIN, LOW);
	Timer3.stop();
}

//Mark new injection cycle when IVO
//TODO: RPM Calculation for logging
void handle_IVO()
{
	digitalWrite(INJ_PIN, LOW); //Cut off injection when IVO
	logger.newcycle();
}

//On IVC, calculate how much fuel needs to be injected and tell the injector to inject for that period of time
void handle_IVC()
{
	double airMass = airmass.estimateAirMass();
	double fuelMass = fuelmass.estimateFuelMass(14.7, airMass);
	const double injectorFuelRate = 2.13333; //grams per second
	double injectionTime = fuelMass / (injectorFuelRate * 1000000); //in microseconds
	inject(injectionTime);
}

//Start fuel injection for specified time in microseconds
void inject(double injectionTime)
{
	Timer3.setPeriod(injectionTime);
	digitalWrite(INJ_PIN, HIGH);
	Timer3.start();
}

void dummy() {}
