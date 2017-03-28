#include "PluginProcessor.h"
#include <memory>

#include <string>

//==============================================================================
Assignment2Processor::Assignment2Processor(){}

//==============================================================================
void Assignment2Processor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    numInputChannels_ = getNumInputChannels();
    // Create a new granulator object for each input channel
    for(int i=0; i<numInputChannels_; ++i){
        granulators_.push_back(std::make_unique<Granulator>());
    }
}

void Assignment2Processor::releaseResources(){}

void Assignment2Processor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    //////////////////////////////////////////////////////////////////////////
    // Audio processing block
    //////////////////////////////////////////////////////////////////////////
    // Helpful information about this block of samples:
    const int numSamples = buffer.getNumSamples();          // How many samples in the buffer for this block?
    // Create new buffer to store input
    AudioSampleBuffer input;
    // Copy input audio to a new buffer (input buffer will be used for output)
    input.makeCopyOf(buffer);
    // Clear output buffer ready for new output samples
    buffer.clear();

    // Store the number of the channel that is currently being processed
    int channel;
    // For each channel (Process channels individually)
    for (channel = 0; channel < numInputChannels_; ++channel)
    {
        // Get Read pointer for input and write pointer for output
        float* in = input.getReadPointer(channel);
        float* out = buffer.getWritePointer(channel);

        granulator.applyShuffle(in, out, numSamples)
    }
    // Clear any remaining channels to avoid output of garbage when output
    // channel > input channels
    while(channel < numOutputChannels_)
    {
        buffer.clear (channel++, 0, buffer.getNumSamples());
    }
}


// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Assignment2Processor();
}

//==============================================================================
// Getter and setter function for the processor class 
//==============================================================================

bool Assignment2Processor::silenceInProducesSilenceOut() const
{
#if JucePlugin_SilenceInProducesSilenceOut
    return true;
#else
    return false;
#endif
}

bool Assignment2Processor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool Assignment2Processor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}
