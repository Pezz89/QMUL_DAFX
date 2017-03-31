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
        void updateParameters(const unsigned int grainSize);
        void applyShuffle(const float* const in, float* const out, const int numSamples) noexcept;
    private:
        AudioSampleBuffer grainBuf_;
        std::vector<const float*> readPointers_;
        std::vector<int> readPointerGrainPosition_;
        std::vector<int> readPointersBufferPosition_;
        float* writePointer_;
        int writePointerPosition_;

        unsigned int grainSize_;
        unsigned int hopSize_;
        unsigned int prevHop_ = 0;
        unsigned int prevGSize = 0;
        unsigned int sampCounter = 0;

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
