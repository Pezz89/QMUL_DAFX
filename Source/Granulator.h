#ifndef __GRANULATOR_H_4693CB6E__
#define __GRANULATOR_H_4693CB6E__

class Granulator
{
    public:
        Granulator() {};
        ~Granulator() {};
        void applyShuffle(float* const samples, float* const output, const int numSamples) noexcept;
};
JUCE_LEAK_DETECTOR (CrossoverFilter);
#endif  // __GRANULATOR_H_4693CB6E__
