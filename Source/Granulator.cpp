#define _USE_MATH_DEFINES
 
#include "Granulator.h"
#include <cmath>
#include <cstdio>
Granulator::Granulator(const int maxBufSize) 
{
    // Create buffer to store a set amount of audio - this will be used as the
    // source for grains of output
    grainBuf_ = AudioSampleBuffer(1, maxBufSize);
    // Create a write pointer to use for writing audio to the buffer
    writePointer_ = grainBuf_.getWritePointer(0);
    writePointerPosition_ = 0;

    // For the number of overlapping grains at any time
    for(int i=0; i<4; i++) {
        // Create a read pointer
        readPointers_.push_back(grainBuf_.getReadPointer(0));
        // A position of -1 indicates that the pointer is inactive
        readPointerGrainPosition_.push_back(-1);
        readPointerGrainSize_.push_back(0);
        readPointersBufferPosition_.push_back(0);
    }
}

void Granulator::updateParameters(const unsigned int grainSize)
{
    grainSize_ = grainSize;
    hopSize_ = ceil(grainSize_/4);
}

void Granulator::applyShuffle (const float* const in, float* const out, const int numSamples) noexcept {
    // For each sample in the current input block...
    int numSamps = grainBuf_.getNumSamples();
    for(int i = 0; i < numSamples; ++i) {
        // Write input sample to current point in buffer
        *writePointer_ = in[i];

        // For each read pointer for the current channel...
        for(int j=0; j<readPointers_.size(); j++) {
            // If the read pointer isn't currently active...
            if((j * hopSize_ == sampCounter) && (readPointerGrainPosition_[j] < 0)) {
            //if(readPointerGrainPosition_[j] < 0) {
                // Generate new index at random within the input sample buffer
                int randNum = randomFrom<int>(0, numSamps);
                // Set read position of the current pointer to that index
                readPointers_[j] = grainBuf_.getReadPointer(0, randNum);
                // Set grain position tracking of pointer to 0
                readPointerGrainPosition_[j] = 0;
                readPointerGrainSize_[j] = grainSize_;
                readPointersBufferPosition_[j] = randNum;
            }

            // Generate value for hann window at the current index
            float winVal = 0.5*(1.0-cos((2*M_PI*readPointerGrainPosition_[j])/(readPointerGrainSize_[j]-1)));
            // read current sample into output and multiply by the window
            // function at the current index
            out[i] += *readPointers_[j] * winVal;
            // Increment read pointer and check that it hasn't exceeded the size of
            // the grain
            readPointers_[j]++;
            readPointersBufferPosition_[j]++;
            if(readPointersBufferPosition_[j] == numSamps) {
                readPointers_[j] = grainBuf_.getReadPointer(0);
                readPointersBufferPosition_[j] = 0;
            }
            if(readPointerGrainPosition_[j] > -1) {
                readPointerGrainPosition_[j]++;
            }
            // If it has then reset it's position to inactive
            if(readPointerGrainPosition_[j] >= readPointerGrainSize_[j]) {
                readPointerGrainPosition_[j] = -1;
            }
        }
        // Increment the write pointer
        writePointer_++;
        writePointerPosition_++;
        if(writePointerPosition_ == numSamps) {
            writePointer_ = grainBuf_.getWritePointer(0);
            writePointerPosition_ = 0;
        }
        sampCounter++;
        sampCounter %= grainSize_;
    }
}
