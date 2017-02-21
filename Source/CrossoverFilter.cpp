/*
  This code accompanies the textbook:

  Digital Audio Effects: Theory, Implementation and Application
  Joshua D. Reiss and Andrew P. McPherson

  ---

  Parametric EQ: parametric equaliser adjusting frequency, Q and gain
  See textbook Chapter 4: Filter Effects

  Code by Andrew McPherson, Brecht De Man and Joshua Reiss

  ---

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "CrossoverFilter.h"
#include <cmath>
#include <vector>

/* The actual audio processing is handled by the Juce IIRFilter parent
 * class. This subclass is used to define the coefficients for our
 * implementation of a parametric equaliser.
 */

CrossoverFilter::CrossoverFilter(bool highpass, bool linkwitzRiley) noexcept {
    numerator.resize(3);
    denominator.resize(3);
    // Allocate memory for delay line based on the number of
    // coefficients generated. Initialize vectors with values of 0.
    inputDelayBuf.assign(int(numerator.size()), 0.0);
    outputDelayBuf.assign(int(denominator.size()), 0.0);
    // Store the delay size of delay buffers
    inputDelaySize = inputDelayBuf.size();
    outputDelaySize = outputDelayBuf.size();
}

void CrossoverFilter::makeCrossover(
        const double crossoverFrequency,
        const bool linkwitzRiley,
        const bool highpass
    ) noexcept
{
    jassert (crossoverFrequency > 0);

    // This code was adapted from code originally submitted by the author for
    // the Real-time DSP module.

    // Deifine Q as the square root of 2
    static const double q = sqrt(2.0);

    // Warp the frequency to convert from continuous to discrete time cutoff
    const double wd1 = 1.0 / tan(M_PI*crossoverFrequency);

    // Calculate coefficients from equation and store in a vector
    numerator[0] = 1.0 / (1.0 + q*wd1 + pow(wd1, 2));
    numerator[1] = 2 * numerator[0];
    numerator[2] = numerator[0];
    denominator[0] = 1.0;
    denominator[1] = 2.0 * (pow(wd1, 2) - 1.0) * numerator[0];
    denominator[2] = -(1.0 - q * wd1 + pow(wd1, 2)) * numerator[0];

    inputDelayBuf = {0};
    outputDelayBuf = {0};
    inputDelayBufWritePtr = 0;
    outputDelayBufWritePtr = 0;
    // If the filter is a high pass filter, convert numerator
    // coefficients to reflect this
    /*
    if(highpass) {
        numerator[0] = numerator[0] * pow(wd1, 2);
        numerator[1] = -numerator[1] * pow(wd1, 2);
        numerator[2] = numerator[2] * pow(wd1, 2);
    }
    */
    // If the filter is using the Linkwitz-Riley filter structure,
    // convolve the numerator and denominator generated for the 2nd
    // order butterworth filter with themselves. This creates the 5
    // coefficients of 2 cascaded 2nd order butterworth filters needed
    // for this filter structure.
    /*
    if(linkwitzRiley) {
        numerator = convolveCoefficients(numerator, numerator);
        denominator = convolveCoefficients(denominator, denominator);
    }
    */

    /* Limit the bandwidth so we don't get a nonsense result from tan(B/2) */
    /*
    const double bandwidth = jmin(discreteFrequency / Q, M_PI * 0.99);
    const double two_cos_wc = -2.0*cos(discreteFrequency);
    const double tan_half_bw = tan(bandwidth / 2.0);
    const double g_tan_half_bw = gainFactor * tan_half_bw;
    const double sqrt_g = sqrt(gainFactor);
    */

    /* setCoefficients() takes arguments: b0, b1, b2, a0, a1, a2
     * It will normalise the filter according to the value of a0
     * to allow standard time-domain implementations
     */

    /*
    coefficients = IIRCoefficients(
            sqrt_g + g_tan_half_bw, // b0
            sqrt_g * two_cos_wc, // b1
            sqrt_g - g_tan_half_bw, // b2
            sqrt_g + tan_half_bw, // a0
            sqrt_g * two_cos_wc, // a1
            sqrt_g - tan_half_bw // a2
        );
    */

    //setCoefficients(coefficients);
    //

}

void CrossoverFilter::applyFilter(float* const samples, const int numSamples) noexcept {
    const SpinLock::ScopedLockType sl (processLock);
    for(int i = 0; i < numSamples; ++i) {
        const float in = samples[i];
        // Increment the write pointer of the delay buffer storing input
        // samples
        ++inputDelayBufWritePtr;
        // Wrap values to withink size of buffer. Prevents an integer
        // overflow
        inputDelayBufWritePtr = (inputDelayBufWritePtr+inputDelaySize)%inputDelaySize;

        // Increment the write pointer of the delay buffer storing output
        // samples
        ++outputDelayBufWritePtr;
        // Wrap values to withink size of buffer. Prevents an integer
        // overflow
        outputDelayBufWritePtr = (outputDelayBufWritePtr+outputDelaySize)%outputDelaySize;

        // Set the current value of the input delay buffer to the value of
        // the sample provided to the function
        inputDelayBuf[(inputDelayBufWritePtr+inputDelaySize)%inputDelaySize] = in;

        // Initialize a variable to store an output value
        float y = 0;
        // Accumulate each sample in the input delay buffer, multiplied by
        // it's corresponding coefficient
        for(unsigned int j = 0; j < inputDelaySize; j++) {
            y += inputDelayBuf[(inputDelayBufWritePtr-j+inputDelaySize)%inputDelaySize] * numerator[j];
        }
        // decumulate each sample in the output delay buffer (aside from
        // the current index), multiplied by it's corresponding coefficient
        for(unsigned int j = 1; j < outputDelaySize; j++) {
            y -= outputDelayBuf[(outputDelayBufWritePtr-j+outputDelaySize)%outputDelaySize] * denominator[j];
        }
        // Scale by first coefficient in the denominator (always 1 in
        // current implementation, so added only for generalization of the
        // method for future use)
        y /= denominator[0];

        // Store the calculated output sample in the output sample delay
        // buffer
        outputDelayBuf[(outputDelayBufWritePtr+outputDelaySize)%outputDelaySize] = y;

        samples[i] = y;
    }
}

std::vector<double> CrossoverFilter::convolveCoefficients(std::vector<double> const &f, std::vector<double> const &g) {
    // Calculate the size of input vectors
    const int nf = f.size();
    const int ng = g.size();
    // Calculate the size of output vector as the combined size of both
    // input vectors, minus 1
    const int n  = nf + ng - 1;
    // Initialize vector of the same input type as input vectors
    // Allocate memory for all elements of the output to be calculated
    std::vector<double> out(n, double());
    // For each output element...
    for(auto i(0); i < n; ++i) {
        // Calculate minimum and maximum indexes to iterate over each
        // vector
        const int jmn = (i >= ng - 1)? i - (ng - 1) : 0;
        const int jmx = (i <  nf - 1)? i : nf - 1;
        // Accumulate the multiplication of elements in both vectors,
        // based on the indexes calculated, to give the output value at
        // the current output index
        for(auto j(jmn); j <= jmx; ++j) {
            out[i] += (f[j] * g[i - j]);
        }
    }
    return out;
}

/* Copy coefficients from another object of the same class */

/*
void CrossoverFilter::copyCoefficientsFrom (const CrossoverFilter& other) noexcept
{
    setCoefficients(other.coefficients);
    active = other.active;
}
*/

