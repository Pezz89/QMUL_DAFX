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
#include "Granulator.h"
#include "GenericEditor.h"
#include <vector>
#include <array>

//==============================================================================
/**
*/
class Assignment2Processor  : public AudioProcessor
{
public:
    //==============================================================================
    Assignment2Processor();
    // Empty destructor as all dynamic memory is handled using std containers and
    // unique_ptrs
    ~Assignment2Processor() {};

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);

    //==============================================================================
    AudioProcessorEditor* createEditor();
    bool hasEditor() const;

    //==============================================================================

    const String getName() const {return JucePlugin_Name;}
    const String getInputChannelName (int channelIndex) const {return String (channelIndex + 1);}
    const String getOutputChannelName (int channelIndex) const {return String (channelIndex + 1);}
    bool isInputChannelStereoPair (int index) const {return true;}
    bool isOutputChannelStereoPair (int index) const {return true;}
    double getTailLengthSeconds() const{return 0.0;}
    int getNumPrograms(){return 0;}
    int getCurrentProgram(){return 0;}
    void setCurrentProgram (int index){}
    const String getProgramName (int index){return String::empty;}
    void changeProgramName (int index, const String& newName){}

    bool silenceInProducesSilenceOut() const;
    bool acceptsMidi() const;
    bool producesMidi() const;

    //==============================================================================
    bool hasEditor() const{return true;}
    AudioProcessorEditor* createEditor() { return new GenericEditor (*this); }

    //==============================================================================

    // these are used to persist the UI's size - the values are stored along with the
    // filter's other parameters, and the UI component will update them when it gets
    // resized.
    int lastUIWidth_, lastUIHeight_;
    // Can be used to persist states between instances. Removed for simplicity.
    void getStateInformation (MemoryBlock& destData){}
    void setStateInformation (const void* data, int sizeInBytes){}

private:
    unsigned int numInputChannels_;
    unsigned int numOutputChannels_;   // How many output channels for our effect?
    void updateGranulator(float sampleRate) {};

    std::vector<std::vector<std::unique_ptr<Granulator>>> granulators_;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Assignment2Processor);
};

#endif  // __PLUGINPROCESSOR_H_4693CB6E__
