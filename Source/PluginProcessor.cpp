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
    numCompPerChannel = 4;

    crossoverFreq.resize(numXOverPerChannel);
    compressorThresh.resize(numCompPerChannel);
    compressorRatio.resize(numCompPerChannel);
    compressorActive.resize(numCompPerChannel);

    for(int i = 0; i < numXOverPerChannel; i++) {
        std::string s1 = "crossover" + std::to_string(i+1) + "Freq";
        std::string s2 = "Crossover " + std::to_string(i+1) + " Frequency";
        addParameter (crossoverFreq[i] = new AudioParameterFloat (s1, s2, NormalisableRange<float>(20.0f, 20000.0f, 0.0f, 1.0f), 1000.0f));
    }

    for(int i = 0; i < numCompPerChannel; i++) {
        std::string s1 = "comp" + std::to_string(i+1) + "thresh";
        std::string s2 = "Compressor " + std::to_string(i+1) + " Threshold";
        addParameter (compressorThresh[i] = new AudioParameterFloat (s1, s2, 0.0f, 1.0f, 0.5f));

        s1 = "comp" + std::to_string(i+1) + "ratio";
        s2 = "Compressor " + std::to_string(i+1) + " Ratio";
        addParameter (compressorRatio[i] = new AudioParameterFloat (s1, s2, 0.0f, 1.0f, 0.5f));

        s1 = "comp" + std::to_string(i+1) + "active";
        s2 = "Compressor " + std::to_string(i+1) + " Active";
        addParameter (compressorActive[i] = new AudioParameterBool (s1, s2, false));
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
        for(int i = 0; i < numCrossoverFilters_; i++) {
            for(int j = 0; j < numXOverPerChannel*2; j+=2) {
                crossoverFilters_[i][j] = std::make_unique<CrossoverFilter>(false, false);
                crossoverFilters_[i][j+1] = std::make_unique<CrossoverFilter>(true, false);
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
        for(int i = 0; i < numChannels; i++) {
            for(int j = 0; j < numCompPerChannel; j++) {
            compressors_[i][j] = std::make_unique<Compressor>(bufferSize);
            }
        }
    }
    // Update the compressor settings to work with the current parameters and sample rate
    updateCompressor(sampleRate);
}

void Assignment1Processor::releaseResources()
{
    numCrossoverFilters_ = 0;
}

void Assignment1Processor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    // Helpful information about this block of samples:
    const int numInputChannels = getNumInputChannels();     // How many input channels for our effect?
    const int numOutputChannels = getNumOutputChannels();   // How many output channels for our effect?
    const int numSamples = buffer.getNumSamples();          // How many samples in the buffer for this block?
    const bool cOO = compressorONOFF;
    int channel;

    updateCompressor(getSampleRate());
    updateFilter(getSampleRate());
    // Go through each channel of audio that's passed in
    for (channel = 0; channel < jmin((int32)numInputChannels, numCrossoverFilters_); ++channel)
    {
        // channelData is an array of length numSamples which contains the audio for one channel
        float* channelData = buffer.getWritePointer(channel);

        for(int i = 0; i < numXOverPerChannel; i++) {
            crossoverFilters_[channel][i]->applyFilter(channelData, numSamples);
            if(crossoverFilters_[channel][i]->linkwitzRileyActive()) {
                crossoverFilters_[channel][i]->applyFilter(channelData, numSamples);
            }
        }
    }

    //TODO: Intergrate with code above
    for (int m = 0 ; m < numOutputChannels ; ++m) {
        for(int j = 0; j < numCompPerChannel; j+=2) {
            compressors_[m][j]->processSamples(buffer, numSamples);
        }
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
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    // Create an outer XML element..
    XmlElement xml("C4DMPLUGINSETTINGS");

    // add some attributes to it..
    xml.setAttribute("uiWidth", lastUIWidth_);
    xml.setAttribute("uiHeight", lastUIHeight_);
    xml.setAttribute("centreFrequency", centreFrequency_);
    xml.setAttribute("q", q_);
    xml.setAttribute("gainDecibels", gainDecibels_);
    xml.setAttribute("compressorONOFF", compressorONOFF);

    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary(xml, destData);
}

void Assignment1Processor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    // This getXmlFromBinary() helper function retrieves our XML from the binary blob..
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if(xmlState != 0)
    {
        // make sure that it's actually our type of XML object..
        if(xmlState->hasTagName("C4DMPLUGINSETTINGS"))
        {
            // ok, now pull out our parameters..
            lastUIWidth_  = xmlState->getIntAttribute("uiWidth", lastUIWidth_);
            lastUIHeight_ = xmlState->getIntAttribute("uiHeight", lastUIHeight_);

            centreFrequency_ = (float)xmlState->getDoubleAttribute("centreFrequency", centreFrequency_);
            q_ = (float)xmlState->getDoubleAttribute("q", q_);
            gainDecibels_ = (float)xmlState->getDoubleAttribute("gainDecibels", gainDecibels_);
            compressorONOFF = (bool)xmlState->getDoubleAttribute("compressorONOFF", compressorONOFF);
            updateFilter(getSampleRate());
            // Update the compressor settings to work with the current parameters and sample rate
            updateCompressor(getSampleRate());
        }
    }
}

//==============================================================================
// Update the coefficients of the parametric equaliser filter
void Assignment1Processor::updateFilter(float sampleRate)
{
    for(int i = 0; i < numChannels; i++) {
        for(int j = 0; j < numXOverPerChannel; j+=2) {
            crossoverFilters_[i][j]->makeCrossover(centreFrequency_, sampleRate, true, false);
            crossoverFilters_[i][j+1]->makeCrossover(centreFrequency_, sampleRate, false, false);
        }
    }
}
void Assignment1Processor::updateCompressor(float sampleRate)
{
    for(int i = 0; i < numChannels; i++) {
        for(int j = 0; j < numCompPerChannel; j+=2) {
            compressors_[i][j]->makeCompressor(sampleRate, compressorONOFF);
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Assignment1Processor();
}
