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
    numXOverPerChannel = 3;
    numCompPerChannel = numXOverPerChannel+1;

    crossoverFreq.resize(numXOverPerChannel);
    compressorThresh.resize(numCompPerChannel);
    compressorGain.resize(numCompPerChannel);
    compressorRatio.resize(numCompPerChannel);
    compressorActive.resize(numCompPerChannel);
    compressorAttack.resize(numCompPerChannel);
    compressorRelease.resize(numCompPerChannel);

    for(int i = 0; i < numXOverPerChannel; i++) {
        std::string s1 = "crossover" + std::to_string(i+1) + "Freq";
        std::string s2 = "Crossover " + std::to_string(i+1) + " Frequency";
        addParameter (crossoverFreq[i] = new AudioParameterFloat (s1, s2, NormalisableRange<float>(20.0f, 20000.0f, 0.0f, 1.0f),
                    round((20000.0f / (1+numXOverPerChannel))*(i+1))));
    }

    for(int i = 0; i < numCompPerChannel; i++) {
        std::string s1 = "comp" + std::to_string(i+1) + "active";
        std::string s2 = "Compressor " + std::to_string(i+1) + " Active";
        addParameter (compressorActive[i] = new AudioParameterBool (s1, s2, false));

        s1 = std::string("comp" + std::to_string(i+1) + "thresh");
        s2 = std::string("Compressor " + std::to_string(i+1) + " Threshold");
        addParameter (compressorThresh[i] = new AudioParameterFloat (s1, s2, NormalisableRange<float>(-60.0f, 0.0f, 1.0f, 1.0f), 0.0f));

        s1 = std::string("comp" + std::to_string(i+1) + "ratio");
        s2 = std::string("Compressor " + std::to_string(i+1) + " Ratio");
        addParameter (compressorRatio[i] = new AudioParameterFloat (s1, s2, NormalisableRange<float>(1.0f, 100.0f, 0.1f, 1.0f), 5.0f));

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

Assignment1Processor::~Assignment1Processor()
{
}

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
void Assignment1Processor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    //////////////////////////////////////////////////////////////////////////
    // Crossover filter initialisation
    //////////////////////////////////////////////////////////////////////////
    // Create as many filters as we have input channels
    numChannels = getNumInputChannels();
    numCrossoverFilters_ = numChannels;
    crossoverFilters_.resize(numCrossoverFilters_);
    for( auto &it : crossoverFilters_ )
    {
        it.resize(numXOverPerChannel * 2);
    }
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
    numCompressors_ = numChannels * numCompPerChannel;

    compressors_.resize(numChannels);
    for( auto &it : compressors_ )
    {
        it.resize(numCompPerChannel);
    }
    int bufferSize = getBlockSize();
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
    // Helpful information about this block of samples:
    const int numInputChannels = getNumInputChannels();     // How many input channels for our effect?
    const int numOutputChannels = getNumOutputChannels();   // How many output channels for our effect?
    const int numSamples = buffer.getNumSamples();          // How many samples in the buffer for this block?
    const int numBands = numXOverPerChannel+1;
    int channel;
    AudioSampleBuffer input;
    input.makeCopyOf(buffer);
    AudioSampleBuffer output = AudioSampleBuffer(1, numSamples);
    buffer.clear();

    updateCompressor(getSampleRate());
    updateFilter(getSampleRate());
    // Go through each channel of audio that's passed in
    for (channel = 0; channel < numInputChannels; ++channel)
    {
        // channelData is an array of length numSamples which contains the audio for one channel
        float* in = input.getWritePointer(channel);
        float* out = output.getWritePointer(0);

        int i = 0;
        int j = 0;
        crossoverFilters_[channel][i]->applyFilter(in, out, numSamples);
        if(crossoverFilters_[channel][i]->linkwitzRileyActive()) {
            crossoverFilters_[channel][i]->applyFilter(out, out, numSamples);
        }
        compressors_[channel][j]->processSamples(output, output, numSamples, 0);
        for (int sample = 0; sample < numSamples; ++sample)
            buffer.getWritePointer(channel)[sample] += output.getReadPointer(0)[sample];

        output.clear();
        j = 1;
        i = 1;
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
        //jassert(i == numBands);
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
        //i = 0;
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
    for(int i = 0; i < numChannels; i++) {
        int j = 0;
        int k = 0;
        crossoverFilters_[i][j]->makeCrossover(*crossoverFreq[k], sampleRate, false, false);
        j = 1;
        k = 1;
        while(j < (numXOverPerChannel*2)-1) {
            crossoverFilters_[i][j]->makeCrossover(*crossoverFreq[k-1], sampleRate, false, true);
            crossoverFilters_[i][j+1]->makeCrossover(*crossoverFreq[k], sampleRate, false, false);
            j+=2;
            k++;
        }
        j = numXOverPerChannel*2-1;
        crossoverFilters_[i][j]->makeCrossover(*crossoverFreq[k-1], sampleRate, false, true);
    }
}
void Assignment1Processor::updateCompressor(float sampleRate)
{
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
