// AirMassEstimator.cpp

#include "math.h"
#include "numericalIntegration.h"
#include "AirMassEstimator.h"
#include "DataArray.h"

AirMassEstimator::AirMassEstimator(
	double dischargeCoefficientTable[DISCHARGE_COEFFICIENT_TABLE_ROWS][DISCHARGE_COEFFICIENT_TABLE_COLS], 
	double volumetricEfficiencyTable[VOLUMETRIC_EFFICIENCY_TABLE_ROWS][VOLUMETRIC_EFFICIENCY_TABLE_COLS],
	double specificGasConstantAir,
	double specificHeatRatio,
	double airIntakeManifoldVolume,
	double cylinderDisplacement,
	double intakeDiameter,
	double intakeValveThickness,
	const DataArray* timeData,
	const DataArray* manifoldAirPressureData,
	const DataArray* angularVelocityData,
	const DataArray* reynoldsNumberData,
	const DataArray* throttleAngleData,
	const DataArray* inletPressureData,
	const DataArray* inletTemperatureData
	)
{
	// Populate Single Value Parameters
	m_previousAirMass = 0;
	m_previousXAdjustment = 0;
	m_specificGasConstantAir = specificGasConstantAir;
	m_specificHeatRatio = specificHeatRatio;
	m_intakeAirManifoldVolume = airIntakeManifoldVolume;
	m_cylinderDisplacement = cylinderDisplacement;
	m_intakeDiameter = intakeDiameter;
	m_intakeRadius = intakeDiameter / 2;
	m_intakeValveThickness = intakeValveThickness;
	
	// Populate discharge coefficient table
	for (int i = 0; i < DISCHARGE_COEFFICIENT_TABLE_ROWS; i++)
	{
		for (int j = 0; j <DISCHARGE_COEFFICIENT_TABLE_COLS; i++)
		{
			m_dischargeCoefficientTable[i][j] = dischargeCoefficientTable[i][j];
		}
	}
	
	// Populate volumetric efficiency table
	for (int i = 0; i < VOLUMETRIC_EFFICIENCY_TABLE_ROWS; i++)
	{
		for (int j = 0; j < VOLUMETRIC_EFFICIENCY_TABLE_COLS; i++)
		{
			m_volumetricEfficiencyTable[i][j] = volumetricEfficiencyTable[i][j];
		}
	}
	
	// Populate Data Array Pointers
	m_timeData = timeData;
	m_manifoldAirPressureData = manifoldAirPressureData;
	m_angularVelocityData = angularVelocityData;
	m_reynoldsNumberData = reynoldsNumberData;
	m_throttleAngleData = throttleAngleData;
	m_inletPressureData = inletPressureData;
	m_inletTemperatureData = inletTemperatureData;
	
	// Construct Intermediate Data Array Pointer
	m_airMassFlowRateData = new DataArray();
}

AirMassEstimator::~AirMassEstimator()
{
	delete m_airMassFlowRateData;
}

double AirMassEstimator::estimateAirMass()
{
	// compute (1 / (X[k] + 1))
	double xAdjustment = computeXAdjustment(
		m_manifoldAirPressureData->mean(), 
		m_angularVelocityData->mean()
		);
	double xFraction = (1.0 / (xAdjustment + 1.0));
	
	// estimate air mass that will enter during next intake valve open event
	double airMass = 
		(xFraction) * ((computeSSAdjustment()) +
			((m_previousXAdjustment) * (m_previousAirMass)));
	
	// save estimated air mass to use in next air mass computation
	m_previousAirMass = airMass;
	m_previousXAdjustment = xAdjustment;
	
	// return estimated air mass
	return airMass;
}

double AirMassEstimator::computeXAdjustment(
	double manifoldAirPressure, 
	double angularVelocity
	) const
{
	// Compute adjustment value depending on the volumetric efficiency
	// at the current manifold air pressure and RPM.
	return
		((m_intakeAirManifoldVolume) / (m_cylinderDisplacement)) *
		((1.0) / (lookupVolumetricEfficiency(manifoldAirPressure, angularVelocity)));
}

double AirMassEstimator::computeSSAdjustment()
{
	// compute steady state air mass flow rate at each time step
	for (int i = 0; i < m_timeData->size(); i++)
	{
		m_airMassFlowRateData->push
		(
			computeSSCompressibleInletAirMassFlowRate
			(
				m_reynoldsNumberData->operator[](i), 
				m_throttleAngleData->operator[](i),
				m_inletPressureData->mean(), 
				m_inletTemperatureData->mean(),
				m_manifoldAirPressureData->operator[](i)
			)
		);
	}
	
	// numerically integrate all inlet air mass flow rates
	double result = trapezoidalNumericalIntegration
		(
			m_airMassFlowRateData->getConstArrayPtr(), 
			m_timeData->getConstArrayPtr(), 
			m_timeData->size()
		);
	
	m_airMassFlowRateData->clear();
	
	return result;
}

double AirMassEstimator::computeSSCompressibleInletAirMassFlowRate(
	double reynoldsNumber, 
	double throttleAngle, 
	double inletPressure, 
	double inletTemperature, 
	double manifoldAirPressure
	) const
{
	return lookupDischargeCoefficient(reynoldsNumber) * 
		computeOpenArea(throttleAngle) * 
		inletPressure/sqrt(m_specificGasConstantAir * inletTemperature) * 
		computeFlowFunction(inletPressure, manifoldAirPressure);
}

double AirMassEstimator::computeOpenArea(
	double throttleAngle
	) const
{
	// throttle angle is in radians. 90 degrees is wide open throttle.
	double totalArea = M_PI * pow(m_intakeRadius, 2);
	double frontalArea = totalArea * cos(throttleAngle);
	double thicknessArea = m_intakeValveThickness * m_intakeDiameter * sin(throttleAngle);
	return totalArea - (frontalArea + thicknessArea);
}

double AirMassEstimator::computeFlowFunction(
	double inletPressure, 
	double manifoldAirPressure
	) const
{
	double sonicPressure = 
		pow
		(
			((2)/(m_specificHeatRatio + 1)),
			((m_specificHeatRatio)/(m_specificHeatRatio - 1))
		)
		* inletPressure;
		
	if (manifoldAirPressure < sonicPressure)	
	{
		return 
			sqrt
			(
				(m_specificHeatRatio) * 
				pow
				(
					((2) / (m_specificHeatRatio + 1)), 
					((m_specificHeatRatio + 1) / (m_specificHeatRatio - 1))
				)
			);
	}
	else
	{
		double pressureRatio = manifoldAirPressure / inletPressure;
		return 
			pow
			(
				(pressureRatio), 
				((1) / (m_specificHeatRatio))
			)
			* sqrt
			(
				((2 * m_specificHeatRatio) / (m_specificHeatRatio - 1)) * 
				((1) - pow((pressureRatio), ((m_specificHeatRatio - 1) / (m_specificHeatRatio))))
			);
	}
}

double AirMassEstimator::lookupVolumetricEfficiency(
	double manifoldAirPressure, 
	double angularVelocity
	) const
{
	// incomplete
	return 1;
}

double AirMassEstimator::lookupDischargeCoefficient(
	double reynoldsNumber
	) const
{
	// Incomplete
	return 1;
}
