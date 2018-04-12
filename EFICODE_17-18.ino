#include "DataArray.h"
#include "AirMassEstimator.h"
#include "numericalIntegration.h"
#include "datalog.h"
#include "constants.h"
#include "TimerThree.h"

AirMassEstimator airmass;
DataArray MAP, CRANK, TPS, FPS, ECT, IAT, IAP;
datalog log;



void setup() {
  // put your setup code here, to run once:
	log = datalog(false, &MAP, &CRANK, &TPS, &FPS, &ECT, &IAT, &IAP);
	log.setup();

	// Attach the interrupt for INJ pulse modulation.
	// For some reason, the internal interrupt flags can end up defaulting
	// to a triggered state before they are attached. This causes them
	// to trigger once right when they are attached. Our current workaround
	// is to attach the interrupt to a dummy function first that triggers
	// if the interrupt is already set. Then, it is safe to attach the normal interrupt.
	Timer3.attachInterrupt(dummy);
	Timer3.detachInterrupt();
	Timer3.attachInterrupt(handle_pulseTimerTimeout);

	//The NULL and 1 parameters here still need to be filled
	airmass = AirMassEstimator(NULL, NULL, 1, 1, 1, 1, 1, 1, NULL, &MAP, &CRANK, null, &TPS, &IAP, &IAT);

	// For some reason, the internal interrupt flags can end up defaulting
	// to a triggered state before they are attached. This causes them
	// to trigger once right when they are attached. Our current workaround
	// is to attach the interrupt to a dummy function first that triggers
	// if the interrupt is already set. Then, it is safe to attach the normal interrupt.
	
	//IVO and IVC pins need to be defined
	attachInterrupt(digitalPinToInterrupt(IVO_Pin), dummy, RISING);
	detachInterrupt(digitalPinToInterrupt(IVO_Pin));
	attachInterrupt(digitalPinToInterrupt(IVO_Pin), handle_IVO, RISING);

	attachInterrupt(digitalPinToInterrupt(IVC_Pin), dummy, RISING);
	detachInterrupt(digitalPinToInterrupt(IVC_Pin));
	attachInterrupt(digitalPinToInterrupt(IVC_Pin), handle_IVC, RISING);

}

void loop() {
  // put your main code here, to run repeatedly:
	log.loopfunction();
}

//End Injection
void handle_pulseTimerTimeout()
{
	digitalWrite(INJ_Pin, LOW);
	Timer3.stop();
}

//Write handler for Intake Valve Open
void handle_IVO()
{

}

//Write handler for Intake Valve Closed
void handle_IVC()
{
	airmass = airmass.estimateAirMass();
	inject(airmass)
}

//Function that calculates injection time and injects fuel
void inject(double airmass)
{

}