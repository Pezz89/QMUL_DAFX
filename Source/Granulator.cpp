#define _USE_MATH_DEFINES
 
#include "Granulator.h"
#include <cmath>
Granulator::Granulator(const int maxBufSize) 
{
    // Create buffer to store a set amount of audio - this will be used as the
    // source for grains of output
    grainBuf_ = AudioSampleBuffer(1, maxBufSize);
    // Create a write pointer to use for writing audio to the buffer
    grainBufWritePointer_ = grainBuf_.getWritePointer(0);
}
Granulator::updateParameters(const unsigned int grainSize, const unsigned int hopSize) : hopSize_(hopSize), grainSize_(grainSize) 
{
    // For the number of overlapping grains at any time
    for(int i=0; i<ceil(float(grainSize_)/float(hopSize_)); i++) {
        // Create a read pointer
        grainBufReadPointers_.push_back(grainBuf_.getReadPointer(0));
        // A position of -1 indicates that the pointer is inactive
        grainBufReadPointersPosition_.push_back(-1);
    }
    // For the number of samples in a grain...
    for(int n=0; n<grainSize_; n++) {
        //TODO: Check accuracy of this is correct
        // Fill a vector with values for a hanning window at the current grain
        // size
        windowBuf.push_back();
    }
}

void Granulator::applyShuffle (const float* const in, float* const out, const int numSamples) noexcept {
    // For each sample in the current input block...
    for(int i = 0; i < numSamples; ++i) {
        // Write input sample to current point in buffer
        *grainBufWritePointer_ = in[i];

        // For each read pointer for the current channel...
        for(int j=0; j<grainBufReadPointers_.size(); j++) {
            // If the read pointer isn't currently active...
            if(grainBufReadPointersPosition_[j] < 0) {
                // Generate new index at random within the input sample buffer
                int randNum = randomFrom<int>(0, int(grainBuf.size()));
                // Set read position of the current pointer to that index
                grainBufReadPointers_[j] = grainBuf_.getReadPointer(0, randNum);
            }

            // Generate value for hann window at the current index
            float winVal = 0.5*(1.0-cos((2*M_PI*grainBufReadPointersPosition_[j])/(grainSize_-1)
            // read current sample into output and multiply by the window
            out[i] = inSamp;
            // function at the current index
            // Increment read pointer and check that it hasn't exceeded the size of
            // the grain
            // If it has then reset it's position to inactive
        }
        // Increment the write pointer
        (inputDelayBufWritePtr+inputDelaySize)%inputDelaySize;

    }
}
