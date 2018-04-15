
#include "dataLog.h"
#include "Arduino.h"
#include "SdFat.h"
#include "constants.h"



datalog::datalog(bool wireless, DataArray* MAP, DataArray* CRANK, DataArray* TPS, DataArray* FPS, DataArray* ECT, DataArray* IAT, DataArray* IAP){
      curBlock = 0;
      emptyTop = 0;
      minTop = 0;
      fullHead = 0;
      fullTail = 0;
      nextSampleMicros = 0;
      fileIsClosing = false;
      collectingData = false;
      isSampling = false;
      justSampled = false;
      newCycle = true;
      Manifold_Air_Array = MAP;
      Crank_Pos_Array = CRANK;
      Throttle_Pos_Array = TPS;
      Fuel_Pressure_Array = FPS;
      Engine_Temp_Array = ECT;
      Intake_Air_Temp_Array = IAT;
      Intake_Air_Pressure_Array = IAP;
      prevMAP = 0;
      prevTime = 0;
      MAP_dip = true;
      MAP_increase = true;
      setup();
}



void datalog::yield(){
// This does the data collection. It is called whenever the teensy is not
// doing something else. The SdFat library will call this when it is waiting
// for the SD card to do its thing, and the loop() function will call this
// when there is nothing to be written to the SD card.

	if (!collectingData || isSampling)
		return;

	isSampling = true;

// If file is closing, add the current buffer to the head of the full queue
// and skip data collection.
	if (fileIsClosing) {
		if (curBlock != 0) {
			putCurrentBlock();
		}
		collectingData = false;
		return;
	}	
  
// If we don't have a buffer for data, get one from the top of the empty 
// stack.
	if (curBlock == 0) {
		curBlock = getEmptyBlock();
	}

// If it's time, record one data sample.
	if (micros() >= nextSampleMicros) {
		if (justSampled) {
			error("rate too fast");
		}
		acquireData(&curBlock->data[curBlock->count++]);
		nextSampleMicros += sampleIntervalMicros;
		justSampled = true;
	} else {
		justSampled = false;
	}

// If the current buffer is full, move it to the head of the full queue. We
// will get a new buffer at the beginning of the next yield() call.
	if (curBlock->count == DATA_DIM) {
		putCurrentBlock();
	}

	isSampling = false;
}

void datalog::loopfunction() {
// Write the block at the tail of the full queue to the SD card
	if (fullHead == fullTail) { // full queue is empty
		if (fileIsClosing){
			file.close();
			Serial.println("File complete.");
			blinkForever();
		} else {
			yield(); // acquire data etc.
		}
	} else { 
	// full queue not empty
	// write buffer at the tail of the full queue and return it to the top of
	// the empty stack.
		digitalWrite(LED_PIN, HIGH);
		block_t* pBlock = fullQueue[fullTail];
		fullTail = fullTail < QUEUE_LAST ? fullTail + 1 : 0;
		if ((int)BUF_DIM != file.write(pBlock, BUF_DIM)) {
			error("write failed");
		}
		emptyStack[emptyTop++] = pBlock;
		digitalWrite(LED_PIN, LOW);
	}

	fileIsClosing = Serial.available();
}

void datalog::setup() {
	Serial.begin(28800); //CHANGE TO VARIABLE IN ORDER TO ALLOW EASIER CHANGING
	while (!Serial) {
	}

	
	

	// Put all the buffers on the empty stack.
	for (int i = 0; i < BUFFER_BLOCK_COUNT; i++) {
		emptyStack[i] = &block[i - 1];
	}
	emptyTop = BUFFER_BLOCK_COUNT;

	sd.begin();
	if (!file.open("TeensyDemo.txt", O_RDWR | O_CREAT)) {
		error("open failed");
	}
  
	Serial.print("Block size: ");
	Serial.println(BUF_DIM);
	Serial.print("Record size: ");
	Serial.println(sizeof(data_t));
	Serial.print("Records per block: ");
	Serial.println(DATA_DIM);
	Serial.print("Record bytes per block: ");
	Serial.println(DATA_DIM*sizeof(data_t));
	Serial.print("Fill bytes per block: ");
	Serial.println(FILL_DIM);
	Serial.println("Recording. Enter any key to stop.");
	delay(100);
	collectingData=true;
	nextSampleMicros = micros() + sampleIntervalMicros;
}

struct block_t* datalog::getEmptyBlock() {
	/*
	* Takes a block form the top of the empty stack and returns it
	*/
	block_t* blk = 0;
	if (emptyTop > 0) { // if there is a buffer in the empty stack
		blk = emptyStack[--emptyTop];
		blk->count = 0;
	} else { // no buffers in empty stack
		error("All buffers in use");
	}
	return blk;
}
//-----------------------------------------------------------------------------
void datalog::putCurrentBlock() {
  /*
   * Put the current block at the head of the queue to be written to card
   */
  fullQueue[fullHead] = curBlock;
  fullHead = fullHead < QUEUE_LAST ? fullHead + 1 : 0;
  curBlock = 0;
}
//-----------------------------------------------------------------------------
void datalog::error(String msg) {
  Serial.print("ERROR: ");
  Serial.println(msg);
  //How to deal with when there is an error
}
//-----------------------------------------------------------------------------
void datalog::acquireData(struct data_t* data){
  data->time = micros();
  
 //Grab these values as fast as possible
  Timestamps->push(data->time);
  data->adc[0] = analogRead(MAP_PIN);
  // Add interrupt to make A23 go high
  double dMAP_dt = (data->adc[0] - prevMAP) / (data->time - prevTime);
  if(MAP_dip && dMAP_dt <= -20){
    digitalWrite(MAP_IVO_PIN, HIGH);
  }else if(MAP_increase && dMAP_dt >= 20){
    //digitalWrite(MAP_IVO_PIN, HIGH);
  }else{
    digitalWrite(MAP_IVO_PIN, LOW);
  }
  Manifold_Air_Array->push((double)(data->adc[0]));
  data->adc[1] = analogRead(CRANK_PIN);
  Crank_Pos_Array->push((double)(data->adc[1]));
 //grab all these values only at the start of a new cycle since they do not change quickly
 //and we only need these values at the beginning of each cycle
  if(newCycle){
  newCycle = false;
  data->adc[2] = analogRead(TPS_PIN);
  Throttle_Pos_Array->push((double)(data->adc[2]));
  data->adc[3] = analogRead(FPS_PIN);
  Fuel_Pressure_Array->push((double)(data->adc[3]));
  data->adc[4] = analogRead(ECT_PIN);
  Engine_Temp_Array->push((double)(data->adc[4]));
  data->adc[5] = analogRead(IAT_PIN);
  Intake_Air_Temp_Array->push((double)(data->adc[5]));
  data->adc[6] = analogRead(IAP_PIN);
  Intake_Air_Pressure_Array->push((double)(data->adc[6]));

  }
  
  
  

 //send to serial if connected
  if(Serial){
    Serial.println(data->adc[0]);
    Serial.println(data->adc[1]);
    Serial.println(data->adc[2]);
    Serial.println(data->adc[3]);
    Serial.println(data->adc[4]);
    Serial.println(data->adc[5]);
    Serial.println(data->adc[6]);
  }
}
//-----------------------------------------------------------------------------

void datalog::newcycle(){
  newCycle = true;
}
