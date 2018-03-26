#ifndef FUELMASSESTIMATOR_H
#define FUELMASSESTIMATOR_H

class FuelMassEstimator
{
    public:
    
    FuelMassEstimator();
    ~FuelMassEstimator();

    double estimateFuelMass(
        double desiredAirFuelRatio, 
        double estimatedAirMass
        );
    
    private:

    
};

#endif
