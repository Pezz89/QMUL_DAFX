#include "Granulator.h"
void Granulator::applyShuffle (const float* const in, float* const out, const int numSamples) noexcept {
    for(int i = 0; i < numSamples; ++i) {
        // Perform filtering using doubles for greater precision
        const double inSamp = in[i];
        out[i] = inSamp;
    }
}
