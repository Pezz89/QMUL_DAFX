#ifndef __GRANULATOR_H_4693CB6E__
#define __GRANULATOR_H_4693CB6E__

#include "../JuceLibraryCode/JuceHeader.h"

class Granulator
{
    public:
        Granulator() {};
        ~Granulator() {};
        void applyShuffle(const float* const in, float* const out, const int numSamples) noexcept;
    private:
        JUCE_LEAK_DETECTOR (Granulator);
};
#endif  // __GRANULATOR_H_4693CB6E__
