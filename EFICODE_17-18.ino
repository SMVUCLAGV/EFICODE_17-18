#include "DataArray.h"
#include "AirMassEstimator.h"
#include "numericalIntegration.h"
#include "datalog.h"
#include "constants.h"
#include "TimerThree.h"

AirMassEstimator airmass;
FuelMassEstimator fuelmass;
DataArray Timestamps, MAP, CRANK, TPS, FPS, ECT, IAT, IAP, Reynolds;
datalog log;

void setup() {
	log = datalog(false, &MAP, &CRANK, &TPS, &FPS, &ECT, &IAT, &IAP);
	log.setup();

	//The NULL and 1 parameters here still need to be filled
	airmass = AirMassEstimator(NULL, NULL, 1, 1, 1, 1, 1, 1, Timestamps, &MAP, &CRANK, &Reynolds, &TPS, &IAP, &IAT);

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
	attachInterrupt(digitalPinToInterrupt(IVC_PIN), handle_IVC, RISING)

}

void loop() {
	log.loopfunction();
}

//End Injection
void handle_pulseTimerTimeout()
{
	digitalWrite(INJ_Pin, LOW);
	Timer3.stop();
}

//Mark new injection cycle when IVO
//TODO: RPM Calculation for logging
void handle_IVO()
{
	digitalWrite(INJ_Pin, LOW); //Cut off injection when IVO
	log.newcycle();
}

//On IVC, calculate how much fuel needs to be injected and tell the injector to inject for that period of time
void handle_IVC()
{
	double airmass = airmass.estimateAirMass();
	double fuelmass = fuelmass.estimateFuelMass(14.7, airmass);
	const double injectorFuelRate = 2.13333; //grams per second
	double injectionTime = airmass / (injectorFuelRate * 1000000); //in microseconds
	inject(injectionTime);
}

//Start fuel injection for specified time in microseconds
void inject(double injectionTime)
{
	Timer3.setPeriod(injectionTime);
	digitalWrite(INJ_Pin, HIGH);
	Timer3.start();
}