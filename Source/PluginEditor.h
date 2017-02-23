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

#ifndef __PLUGINEDITOR_H_6E48F605__
#define __PLUGINEDITOR_H_6E48F605__

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"


//==============================================================================

class Assignment1ProcessorEditor  : public AudioProcessorEditor,
                                    public Timer,
                                    public SliderListener,
                                    public ButtonListener
{
public:
    Assignment1ProcessorEditor (Assignment1Processor* ownerFilter);
    ~Assignment1ProcessorEditor();

    //==============================================================================
    // This is just a standard Juce paint method...
    void timerCallback();
    void paint (Graphics& g);
    void resized();
    void sliderValueChanged (Slider*);
    void buttonClicked (Button* buttonThatWasClicked);

private:
    void updateBandwidthLabel();

    // Crossover filter UI objects
    Label centreFrequencyLabel_, qLabel_, gainDecibelsLabel_;
    Label bandwidthLabel_;
    Slider centreFrequencySlider_, qSlider_, gainDecibelsSlider_;

    // Compressor UI objects
    TextButton buttonONOFF;
    Label label, label2, label3, label4, label5, label6, label7;
    Slider sliderThreshold, sliderRatio, sliderGain, sliderAttack, sliderRelease;

    ScopedPointer<ResizableCornerComponent> resizer_;
    ComponentBoundsConstrainer resizeLimits_;

    Assignment1Processor* getProcessor() const
    {
        return static_cast <Assignment1Processor*> (getAudioProcessor());
    }
    Assignment1ProcessorEditor (const Assignment1ProcessorEditor&);
    const Assignment1ProcessorEditor& operator= (const Assignment1ProcessorEditor&);
};


#endif  // __PLUGINEDITOR_H_6E48F605__
