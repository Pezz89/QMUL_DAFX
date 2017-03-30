#include "PluginProcessor.h"
#include <memory>

#include <string>

//==============================================================================
Assignment2Processor::Assignment2Processor()
{
    std::string s1 = "crossover" + std::to_string(1) + "Freq";
    std::string s2 = "Crossover " + std::to_string(1) + " Frequency";
    addParameter (new AudioParameterFloat (s1, s2, NormalisableRange<float>(20.0f, 20000.0f, 0.0f, 1.0f),
                    round((20000.0f / (1+1))*(1+1))));
}

//==============================================================================
void Assignment2Processor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    numInputChannels_ = getTotalNumInputChannels();
    numOutputChannels_ = getTotalNumOutputChannels();
    // Create a new granulator object for each input channel
    for(int i=0; i<numInputChannels_; ++i){
        granulators_.push_back(std::make_unique<Granulator>(int(sampleRate*5)));
        granulators_[i]->updateParameters(44100, 22050);
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
    numInputChannels_ = getTotalNumInputChannels();
    numOutputChannels_ = getTotalNumOutputChannels();
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
        const float* in = input.getReadPointer(channel);
        float* out = buffer.getWritePointer(channel);

        granulators_[channel]->applyShuffle(in, out, numSamples);
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
