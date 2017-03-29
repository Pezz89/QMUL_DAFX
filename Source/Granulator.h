#ifndef __GRANULATOR_H_4693CB6E__
#define __GRANULATOR_H_4693CB6E__

#include "../JuceLibraryCode/JuceHeader.h"
#include <vector>
#include <random>

class Granulator
{
    public:
        Granulator(const int maxBufSize);
        ~Granulator() {};
        void updateParameters(const unsigned int grainSize, const unsigned int hopSize);
        void applyShuffle(const float* const in, float* const out, const int numSamples) noexcept;
    private:
        AudioSampleBuffer grainBuf_;
        std::vector<const float*> grainBufReadPointers_;
        std::vector<int> grainBufReadPointersPosition_;
        float* grainBufWritePointer_;

        unsigned int grainSize_;
        unsigned int hopSize_;

        std::vector<float> winowBuf_;

        // Code adapted from: http://stackoverflow.com/questions/5743678/generate-random-number-between-0-and-10
        template <typename T>
        T randomFrom(const T min, const T max)
        {
            static std::random_device rdev;
            static std::default_random_engine re(rdev());
            typedef typename std::conditional<
                std::is_floating_point<T>::value,
                std::uniform_real_distribution<T>,
                std::uniform_int_distribution<T>>::type dist_type;
            dist_type uni(min, max);
            return static_cast<T>(uni(re));
        }

        JUCE_LEAK_DETECTOR (Granulator);
};
#endif  // __GRANULATOR_H_4693CB6E__
