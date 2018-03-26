#ifndef AIRMASSESTIMATOR_H
#define AIRMASSESTIMATOR_H

#define DISCHARGE_COEFFICIENT_TABLE_ROWS 20
#define DISCHARGE_COEFFICIENT_TABLE_COLS 20

#define VOLUMETRIC_EFFICIENCY_TABLE_ROWS 20
#define VOLUMETRIC_EFFICIENCY_TABLE_COLS 20

class DataArray;

class AirMassEstimator
{
	public:
	
	AirMassEstimator(
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
		);
	~AirMassEstimator();
	
	double estimateAirMass();
	
	private:
	
	// Single Value Parameters
	double m_previousAirMass;
	double m_previousXAdjustment;
	double m_specificGasConstantAir;
	double m_specificHeatRatio;
	double m_intakeAirManifoldVolume;
	double m_cylinderDisplacement;
	double m_intakeDiameter;
	double m_intakeRadius;
	double m_intakeValveThickness;
	
	// Tabular Parameters
	double m_dischargeCoefficientTable[DISCHARGE_COEFFICIENT_TABLE_ROWS][DISCHARGE_COEFFICIENT_TABLE_COLS];
	double m_volumetricEfficiencyTable[VOLUMETRIC_EFFICIENCY_TABLE_ROWS][VOLUMETRIC_EFFICIENCY_TABLE_COLS];
	
	// Computation Functions
	double computeXAdjustment(
		double manifoldAirPressure, 
		double angularVelocity
		) const;
	double computeSSAdjustment();
	double computeSSCompressibleInletAirMassFlowRate(
		double reynoldsNumber, 
		double throttleAngle, 
		double inletPressure, 
		double inletTemperature, 
		double manifoldAirPressure
		) const;
	double computeOpenArea(
		double throttleAngle
		) const; 
	double computeFlowFunction(
		double inletPressure, 
		double manifoldAirPressure
		) const;
	
	// Lookup Functions
	double lookupVolumetricEfficiency(
		double manifoldAirPressure, 
		double angularVelocity
		) const;
	double lookupDischargeCoefficient(
		double reynoldsNumber
		) const;
	
	// Raw Data Inputs
	const DataArray* m_timeData;
	const DataArray* m_manifoldAirPressureData;
	const DataArray* m_angularVelocityData;
	const DataArray* m_reynoldsNumberData;
	const DataArray* m_throttleAngleData;
	const DataArray* m_inletPressureData;
	const DataArray* m_inletTemperatureData;
	
	// Computed Data
	DataArray* m_airMassFlowRateData;
	
};

#endif
