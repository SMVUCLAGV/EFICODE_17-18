#ifndef DataLog_h
#define DataLog_h


#include "SdFat.h"
#include "Arduino.h"
#include "constants.h"
#include "DataArray.h"

class datalog{
	public:

   
 
    datalog(bool wireless, 
      DataArray* Timestamps,
      DataArray* MAP, 
      DataArray* CRANK, 
      DataArray* TPS, 
      DataArray* FPS, 
      DataArray* ECT,
      DataArray* IAT,
      DataArray* IAP
      );
		void yield();
		void setup();
		void loopfunction();
		struct block_t* getEmptyBlock();
		void putCurrentBlock();
		void error(String msg);
		void acquireData(struct data_t* data);
		void newcycle();
    
    void blinkForever();

    
	private: //NEED TO CHANGE THIS

		// Use SdFatSdio (not SdFatSdioEX) because SdFatSdio spends more time in
		// yield(). For more information, see the TeensySdioDemo example from the 
		// SdFat library. 
		SdFat sd;

		File file;
    
  
    block_t block[BUFFER_BLOCK_COUNT]; 
		block_t* curBlock;

    block_t* emptyStack[BUFFER_BLOCK_COUNT];
		uint8_t emptyTop;
		uint8_t minTop;

    block_t* fullQueue[QUEUE_DIM];
		uint8_t fullHead;
		uint8_t fullTail;

		uint32_t nextSampleMicros;
		bool fileIsClosing;
		bool collectingData;
		bool isSampling;
		bool justSampled;
		bool newCycle;

    DataArray* Manifold_Air_Array;
    DataArray* Crank_Pos_Array;
    DataArray* Throttle_Pos_Array;
    DataArray* Fuel_Pressure_Array;
    DataArray* Engine_Temp_Array;
    DataArray* Intake_Air_Temp_Array;
    DataArray* Intake_Air_Pressure_Array;

};




#endif
