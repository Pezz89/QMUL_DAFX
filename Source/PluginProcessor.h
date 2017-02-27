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

#ifndef __PLUGINPROCESSOR_H_4693CB6E__
#define __PLUGINPROCESSOR_H_4693CB6E__

#define _USE_MATH_DEFINES
#include "../JuceLibraryCode/JuceHeader.h"
#include "CrossoverFilter.h"
#include "Compressor.h"
#include "GenericEditor.h"
#include <vector>
#include <array>

//==============================================================================
/**
*/
class Assignment1Processor  : public AudioProcessor
{
public:
    //==============================================================================
    Assignment1Processor();
    ~Assignment1Processor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);

    //==============================================================================
    AudioProcessorEditor* createEditor();
    bool hasEditor() const;

    //==============================================================================
    const String getName() const;

    /*
    int getNumParameters();

    float getParameter (int index);
    void setParameter (int index, float newValue);

    const String getParameterName (int index);
    const String getParameterText (int index);
    */

    const String getInputChannelName (int channelIndex) const;
    const String getOutputChannelName (int channelIndex) const;
    bool isInputChannelStereoPair (int index) const;
    bool isOutputChannelStereoPair (int index) const;

    bool silenceInProducesSilenceOut() const;
    double getTailLengthSeconds() const;
    bool acceptsMidi() const;
    bool producesMidi() const;

    //==============================================================================
    int getNumPrograms();
    int getCurrentProgram();
    void setCurrentProgram (int index);
    const String getProgramName (int index);
    void changeProgramName (int index, const String& newName);

    //==============================================================================
    void getStateInformation (MemoryBlock& destData);
    void setStateInformation (const void* data, int sizeInBytes);

    //==============================================================================

    // these are used to persist the UI's size - the values are stored along with the
    // filter's other parameters, and the UI component will update them when it gets
    // resized.
    int lastUIWidth_, lastUIHeight_;

    enum Parameters
    {
        kCentreFrequencyParam = 0, /* Centre frequency in Hz */
        kQParam,                   /* Filter Q */
        kGainDecibelsParam,        /* Boost/cut in decibels */
        kCompressorONOFF,
        kNumParameters
    };

    float centreFrequency_, q_, gainDecibels_;
    bool compressorONOFF = false;
private:
    void updateFilter(float sampleRate);
    void updateCompressor(float sampleRate);

    std::vector<std::vector<std::unique_ptr<CrossoverFilter>>> crossoverFilters_;
    std::vector<std::vector<std::unique_ptr<Compressor>>> compressors_;

    int numChannels;
    int numCompPerChannel;
    int numXOverPerChannel;
    std::vector<AudioParameterFloat*> crossoverFreq;
    std::vector<AudioParameterFloat*> compressorThresh;
    std::vector<AudioParameterFloat*> compressorRatio;
    std::vector<AudioParameterBool*> compressorActive;
    std::vector<AudioParameterFloat*> compressorGain;
    std::vector<AudioParameterFloat*> compressorAttack;
    std::vector<AudioParameterFloat*> compressorRelease;


    int numCrossoverFilters_;
    int numCompressors_;
    bool linkwitzRiley_ = true;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Assignment1Processor);
};

#endif  // __PLUGINPROCESSOR_H_4693CB6E__
