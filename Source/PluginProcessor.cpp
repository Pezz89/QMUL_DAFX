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

#include "PluginProcessor.h"
#include <memory>

#include <string>

//==============================================================================
Assignment1Processor::Assignment1Processor()
{
    // Define the number of crossover filters and compressors on each channel
    numXOverPerChannel = 3;
    numCompPerChannel = numXOverPerChannel+1;

    // Allocate memory to store parameters recieved from the UI
    crossoverFreq.resize(numXOverPerChannel);
    compressorThresh.resize(numCompPerChannel);
    compressorGain.resize(numCompPerChannel);
    compressorRatio.resize(numCompPerChannel);
    compressorActive.resize(numCompPerChannel);
    compressorAttack.resize(numCompPerChannel);
    compressorRelease.resize(numCompPerChannel);

    // Add crossover parameters to the UI using JUCE's generic GUI editior
    for(int i = 0; i < numXOverPerChannel; i++) {
        std::string s1 = "crossover" + std::to_string(i+1) + "Freq";
        std::string s2 = "Crossover " + std::to_string(i+1) + " Frequency";
        addParameter (crossoverFreq[i] = new AudioParameterFloat (s1, s2, NormalisableRange<float>(20.0f, 20000.0f, 0.0f, 1.0f),
                    round((20000.0f / (1+numXOverPerChannel))*(i+1))));
    }

    // Add compressors to the UI
    for(int i = 0; i < numCompPerChannel; i++) {
        std::string s1 = "comp" + std::to_string(i+1) + "active";
        std::string s2 = "Compressor " + std::to_string(i+1) + " Active";
        addParameter (compressorActive[i] = new AudioParameterBool (s1, s2, false));

        s1 = std::string("comp" + std::to_string(i+1) + "thresh");
        s2 = std::string("Compressor " + std::to_string(i+1) + " Threshold");
        addParameter (compressorThresh[i] = new AudioParameterFloat (s1, s2, NormalisableRange<float>(-60.0f, 0.0f, 1.0f, 1.0f), 0.0f));

        s1 = std::string("comp" + std::to_string(i+1) + "ratio");
        s2 = std::string("Compressor " + std::to_string(i+1) + " Ratio");
        addParameter (compressorRatio[i] = new AudioParameterFloat (s1, s2, NormalisableRange<float>(1.0f, 100.0f, 0.1f, 1.0f), 1.0f));

        s1 = std::string("comp" + std::to_string(i+1) + "gain");
        s2 = std::string("Compressor " + std::to_string(i+1) + " Gain");
        addParameter (compressorGain[i] = new AudioParameterFloat (s1, s2, NormalisableRange<float>(0, 40, 1, 1.0f), 0.0f));

        s1 = std::string("comp" + std::to_string(i+1) + "attack");
        s2 = std::string("Compressor " + std::to_string(i+1) + " Attack");
        addParameter (compressorAttack[i] = new AudioParameterFloat (s1, s2, NormalisableRange<float>(0.1, 80, 0.1, 1.0f), 0.0f));

        s1 = std::string("comp" + std::to_string(i+1) + "release");
        s2 = std::string("Compressor " + std::to_string(i+1) + " release");
        addParameter (compressorRelease[i] = new AudioParameterFloat (s1, s2, NormalisableRange<float>(0.1, 1000, 0.1, 1.0f), 0.0f));
    }
}

// Empty destructor as all dynamic memory is handled using std containers and
// unique_ptrs
Assignment1Processor::~Assignment1Processor()
{
}

//==============================================================================
// Getter and setter function for the processor class 
//==============================================================================
const String Assignment1Processor::getName() const
{
    return JucePlugin_Name;
}

const String Assignment1Processor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String Assignment1Processor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool Assignment1Processor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool Assignment1Processor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool Assignment1Processor::silenceInProducesSilenceOut() const
{
#if JucePlugin_SilenceInProducesSilenceOut
    return true;
#else
    return false;
#endif
}

double Assignment1Processor::getTailLengthSeconds() const
{
    return 0.0;
}

bool Assignment1Processor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool Assignment1Processor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

int Assignment1Processor::getNumPrograms()
{
    return 0;
}

int Assignment1Processor::getCurrentProgram()
{
    return 0;
}

void Assignment1Processor::setCurrentProgram (int index)
{
}

const String Assignment1Processor::getProgramName (int index)
{
    return String::empty;
}

void Assignment1Processor::changeProgramName (int index, const String& newName)
{
}
//==============================================================================

//==============================================================================
void Assignment1Processor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    //////////////////////////////////////////////////////////////////////////
    // Crossover filter initialisation
    //////////////////////////////////////////////////////////////////////////
    // Get info about host for compressor/filter initialization
    numChannels = getNumInputChannels();
    numCrossoverFilters_ = numChannels;
    int bufferSize = getBlockSize();

    // Allocate memory for filter objects
    crossoverFilters_.resize(numCrossoverFilters_);
    for( auto &it : crossoverFilters_ )
    {
        it.resize(numXOverPerChannel * 2);
    }

    // Fill vector with filter objects to be used for constructing crossovers
    if(crossoverFilters_.size() != 0) {
        std::vector<std::vector<std::unique_ptr<CrossoverFilter>> >::iterator row;
        std::vector<std::unique_ptr<CrossoverFilter>>::iterator col;
        for (row = crossoverFilters_.begin(); row != crossoverFilters_.end(); row++) {
            for (col = row->begin(); col != row->end(); col++) {
                *col = std::make_unique<CrossoverFilter>(false, true);
            }
        }
    }

    // Update the filter settings to work with the current parameters and sample rate
    updateFilter(sampleRate);

    //////////////////////////////////////////////////////////////////////////
    // Compressor initialisation
    //////////////////////////////////////////////////////////////////////////
    // Allocate memory for compressor objects
    for( auto &it : compressors_ )
    {
        it.resize(numCompPerChannel);
    }

    // Create required number of compressors
    if(compressors_.size() != 0) {
        std::vector<std::vector<std::unique_ptr<Compressor>> >::iterator row;
        std::vector<std::unique_ptr<Compressor>>::iterator col;
        for (row = compressors_.begin(); row != compressors_.end(); row++) {
            for (col = row->begin(); col != row->end(); col++) {
                *col = std::make_unique<Compressor>(bufferSize);
            }
        }
    }
    // Update the compressor settings to work with the current parameters and sample rate
    updateCompressor(sampleRate);
}

void Assignment1Processor::releaseResources()
{
    compressors_.clear();
    crossoverFilters_.clear();

}

void Assignment1Processor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    //////////////////////////////////////////////////////////////////////////
    // Audio processing block
    //////////////////////////////////////////////////////////////////////////
    // Helpful information about this block of samples:
    const int numInputChannels = getNumInputChannels();     // How many input channels for our effect?
    const int numOutputChannels = getNumOutputChannels();   // How many output channels for our effect?
    const int numSamples = buffer.getNumSamples();          // How many samples in the buffer for this block?
    // Calculate the total number of frequency bands
    const int numBands = numXOverPerChannel+1;
    // Store the number of the channel that is currently being processed
    int channel;
    // Copy input audio to a new buffer (input buffer will be used for output)
    AudioSampleBuffer input;
    input.makeCopyOf(buffer);
    // Stores output in an output buffer, then copies to the actual buffer.
    // This was possibly implemented due to an early bug and could be
    // refactored for improved memory efficiency.
    AudioSampleBuffer output = AudioSampleBuffer(1, numSamples);
    buffer.clear();

    // Update parameters of all effects on each block. This seems excessive to
    // perform in the processing block, but a function that is only called
    // reliably on parameter changes wasn't found.
    updateCompressor(getSampleRate());
    updateFilter(getSampleRate());
    // Go through each channel of audio that's passed in
    for (channel = 0; channel < numInputChannels; ++channel)
    {
        // in is an array of length numSamples which contains the audio for one channel
        float* in = input.getWritePointer(channel);
        float* out = output.getWritePointer(0);

        // Apply each filter to create sub-bands
        int i = 0;
        int j = 0;
        crossoverFilters_[channel][i]->applyFilter(in, out, numSamples);
        // If using the Linkwitz Riley filter structure, apply the 2nd order
        // butterworth twice to create a 4th order Linkwitz Riley filter
        if(crossoverFilters_[channel][i]->linkwitzRileyActive()) {
            crossoverFilters_[channel][i]->applyFilter(out, out, numSamples);
        }
        // Apply compressor to filtered subband
        compressors_[channel][j]->processSamples(output, output, numSamples, 0);
        // Add subband to output block
        for (int sample = 0; sample < numSamples; ++sample)
            buffer.getWritePointer(channel)[sample] += output.getReadPointer(0)[sample];

        // Clear the output block ready for the next subband
        output.clear();
        j = 1;
        i = 1;
        // Create band pass subbands by applying both low and high pass filters
        // to the input. Then apply compressor as with the previous subband and
        // add to the output
        while(i < (numXOverPerChannel*2)-1) {
            crossoverFilters_[channel][i]->applyFilter(in, out, numSamples);
            if(crossoverFilters_[channel][i]->linkwitzRileyActive()) {
                crossoverFilters_[channel][i]->applyFilter(out, out, numSamples);
            }

            crossoverFilters_[channel][i+1]->applyFilter(out, out, numSamples);
            if(crossoverFilters_[channel][i+1]->linkwitzRileyActive()) {
                crossoverFilters_[channel][i+1]->applyFilter(out, out, numSamples);
            }

            compressors_[channel][j]->processSamples(output, output, numSamples, 0);
            for (int sample = 0; sample < numSamples; ++sample)
                buffer.getWritePointer(channel)[sample] += output.getReadPointer(0)[sample];
            output.clear();
            j++;
            i+=2;
        }
        // Finally, apply the high pass to reate the final subband and compress
        // as before
        i = (numXOverPerChannel*2)-1;
        crossoverFilters_[channel][i]->applyFilter(in, out, numSamples);
        if(crossoverFilters_[channel][i]->linkwitzRileyActive()) {
            crossoverFilters_[channel][i]->applyFilter(out, out, numSamples);
        }
        compressors_[channel][j]->processSamples(output, output, numSamples, 0);
        for (int sample = 0; sample < numSamples; ++sample)
            buffer.getWritePointer(channel)[sample] += output.getReadPointer(0)[sample];
        output.clear();
        j = 0;
    }
    // Go through the remaining channels. In case we have more outputs
    // than inputs, or there aren't enough filters, we'll clear any
    // remaining output channels (which could otherwise contain garbage)
    while(channel < numOutputChannels)
    {
        buffer.clear (channel++, 0, buffer.getNumSamples());
    }
}

//==============================================================================
bool Assignment1Processor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* Assignment1Processor::createEditor() { return new GenericEditor (*this); }


//==============================================================================
// Can be used to persist states between instances. Removed for simplicity.
void Assignment1Processor::getStateInformation (MemoryBlock& destData)
{
}

void Assignment1Processor::setStateInformation (const void* data, int sizeInBytes)
{
}

//==============================================================================
// Update the coefficients of the parametric equaliser filter
void Assignment1Processor::updateFilter(float sampleRate)
{
    // Iterate over each filter object and apply relevant parameters from the
    // UI
    for(int i = 0; i < numChannels; i++) {
        int j = 0;
        int k = 0;
        crossoverFilters_[i][j]->makeCrossover(*crossoverFreq[k], sampleRate, true, false);
        j = 1;
        k = 1;
        while(j < (numXOverPerChannel*2)-1) {
            crossoverFilters_[i][j]->makeCrossover(*crossoverFreq[k-1], sampleRate, true, true);
            crossoverFilters_[i][j+1]->makeCrossover(*crossoverFreq[k], sampleRate, true, false);
            j+=2;
            k++;
        }
        j = (numXOverPerChannel*2)-1;
        crossoverFilters_[i][j]->makeCrossover(*crossoverFreq[k-1], sampleRate, true, true);
    }
}
void Assignment1Processor::updateCompressor(float sampleRate)
{
    // Iterate over each compressor object and apply relevant parameters from the
    // UI
    for(int i = 0; i < numChannels; i++) {
        for(int j = 0; j < numCompPerChannel; j++) {
            compressors_[i][j]->makeCompressor(
                    sampleRate,
                    *compressorActive[j],
                    *compressorRatio[j],
                    *compressorThresh[j],
                    *compressorAttack[j],
                    *compressorRelease[j],
                    *compressorGain[j]
                );
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Assignment1Processor();
}
