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
#include "PluginEditor.h"
#include <memory>


//==============================================================================
Assignment1Processor::Assignment1Processor()
{
    // Set default values:
    centreFrequency_ = 1000.0;
    q_ = 2.0;
    gainDecibels_ = 0.0;

    // Initialise the filters later when we know how many channels
    numCrossoverFilters_ = 0;

    lastUIWidth_ = 550;
    lastUIHeight_ = 100;
}

Assignment1Processor::~Assignment1Processor()
{
}

//==============================================================================
const String Assignment1Processor::getName() const
{
    return JucePlugin_Name;
}

int Assignment1Processor::getNumParameters()
{
    return kNumParameters;
}

float Assignment1Processor::getParameter (int index)
{
    // This method will be called by the host, probably on the audio thread, so
    // it's absolutely time-critical. Don't use critical sections or anything
    // UI-related, or anything at all that may block in any way!
    switch (index)
    {
        case kCentreFrequencyParam: return centreFrequency_;
        case kQParam:               return q_;
        case kGainDecibelsParam:    return gainDecibels_;
        default:                    return 0.0f;
    }
}

void Assignment1Processor::setParameter (int index, float newValue)
{
    // This method will be called by the host, probably on the audio thread, so
    // it's absolutely time-critical. Don't use critical sections or anything
    // UI-related, or anything at all that may block in any way!
    switch (index)
    {
        case kCentreFrequencyParam:
            centreFrequency_ = newValue;
            updateFilter(getSampleRate());
            break;
        case kQParam:
            q_ = newValue;
            updateFilter(getSampleRate());
            break;
        case kGainDecibelsParam:
            gainDecibels_ = newValue;
            updateFilter(getSampleRate());
            break;
        default:
            break;
    }
}

const String Assignment1Processor::getParameterName (int index)
{
    switch (index)
    {
        case kCentreFrequencyParam:  return "centre frequency";
        case kQParam:                return "Q";
        case kGainDecibelsParam:     return "gain (dB)";
        default:                     break;
    }

    return String::empty;
}

const String Assignment1Processor::getParameterText (int index)
{
    return String (getParameter (index), 2);
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

    // Create as many filters as we have input channels
    numCrossoverFilters_ = getNumInputChannels();
    crossoverFilters_.resize(numCrossoverFilters_);
    if(crossoverFilters_.size() != 0) {
        for(int i = 0; i < numCrossoverFilters_; i++)
            crossoverFilters_[i] = std::make_unique<CrossoverFilter>(true, false);
    }

    // Update the filter settings to work with the current parameters and sample rate
    updateFilter(sampleRate);
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
    int channel;

    // Go through each channel of audio that's passed in

    for (channel = 0; channel < jmin((int32)numInputChannels, numCrossoverFilters_); ++channel)
    {
        // channelData is an array of length numSamples which contains the audio for one channel
        float* channelData = buffer.getWritePointer(channel);

        // Run the samples through the IIR filter whose coefficients define the parametric
        // equaliser. See juce_IIRFilter.cpp for the implementation.
        crossoverFilters_[channel]->processSamples(channelData, numSamples);
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

AudioProcessorEditor* Assignment1Processor::createEditor()
{
    return new Assignment1ProcessorEditor (this);
}

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
            updateFilter(getSampleRate());
        }
    }
}

//==============================================================================
// Update the coefficients of the parametric equaliser filter
void Assignment1Processor::updateFilter(float sampleRate)
{
    for(int i = 0; i < numCrossoverFilters_; i++)
        crossoverFilters_[i]->makeCrossover(centreFrequency_, sampleRate, false, false);
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Assignment1Processor();
}
