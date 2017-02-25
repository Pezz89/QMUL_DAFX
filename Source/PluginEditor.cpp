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
#include <cstring>

#ifdef _MSC_VER
#define snprintf _snprintf_s //support for pre-2014 versions of Visual Studio
#endif // _MSC_VER

//==============================================================================
Assignment1ProcessorEditor::Assignment1ProcessorEditor (Assignment1Processor* ownerFilter)
    : AudioProcessorEditor (ownerFilter),
      centreFrequencyLabel_("", "Centre frequency (Hz):"),
      qLabel_("", "Q:"),
      gainDecibelsLabel_("", "Gain (dB):"),
      bandwidthLabel_("", "Bandwidth:"),
      buttonONOFF (L"on off button"),
      label ("", L"Compressor"),
      sliderThreshold (L"new slider"),
      label2 (L"new label", L"Threshold"),
      sliderRatio (L"new slider"),
      label3 (L"new label", L"Ratio"),
      sliderGain (L"new slider"),
      label7 (L"new label", L"Gain"),
      sliderAttack (L"new slider"),
      label5 (L"new label", L"Attack Time (ms)"),
      sliderRelease (L"new slider"),
      label6 (L"new label", L"Release Time (ms)")
{
    addAndMakeVisible (&buttonONOFF);
    buttonONOFF.setButtonText (L"ON / OFF");
    buttonONOFF.addListener (this);
    buttonONOFF.setColour (TextButton::buttonColourId, Colour (0xff615a5a));
    buttonONOFF.setColour (TextButton::buttonOnColourId, Colours::chartreuse);

    addAndMakeVisible (&label);
    label.setFont (Font (22.0000f, Font::bold));
    label.setJustificationType (Justification::centred);
    label.setEditable (false, false, false);
    label.setColour (TextEditor::textColourId, Colours::black);
    label.setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (&sliderThreshold);
    sliderThreshold.setRange (-60, 0, -60);
    sliderThreshold.setSliderStyle (Slider::RotaryVerticalDrag);
    //sliderThreshold.setTextBoxStyle (Slider::TextBoxBelow, false, 80, 20);
    sliderThreshold.addListener (this);

    addAndMakeVisible (&label2);
    label2.setFont (Font (15.0000f, Font::plain));
    label2.setJustificationType (Justification::centredLeft);
    label2.setEditable (false, false, false);
    label2.setColour (TextEditor::textColourId, Colours::black);
    label2.setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (&sliderRatio);
    sliderRatio.setRange (1, 100, 0.1);
    sliderRatio.setSliderStyle (Slider::RotaryVerticalDrag);
    //sliderRatio.setTextBoxStyle (Slider::TextBoxBelow, false, 80, 20);
    sliderRatio.addListener (this);

    addAndMakeVisible (&label3);
    label3.setFont (Font (15.0000f, Font::plain));
    label3.setJustificationType (Justification::centredLeft);
    label3.setEditable (false, false, false);
    label3.setColour (TextEditor::textColourId, Colours::black);
    label3.setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (&sliderGain);
    sliderGain.setRange (0, 40, 1);
    sliderGain.setSliderStyle (Slider::RotaryVerticalDrag);
    //sliderGain.setTextBoxStyle (Slider::TextBoxBelow, false, 80, 20);
    sliderGain.addListener (this);

    addAndMakeVisible (&label7);
    label7.setFont (Font (15.0000f, Font::plain));
    label7.setJustificationType (Justification::centredLeft);
    label7.setEditable (false, false, false);
    label7.setColour (TextEditor::textColourId, Colours::black);
    label7.setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (&sliderAttack);
    sliderAttack.setRange (0.1, 80, 0.1);
    sliderAttack.setSliderStyle (Slider::RotaryVerticalDrag);
    //sliderAttack.setTextBoxStyle (Slider::TextBoxBelow, false, 80, 20);
    sliderAttack.addListener (this);

    addAndMakeVisible (&label5);
    label5.setFont (Font (15.0000f, Font::plain));
    label5.setJustificationType (Justification::centredLeft);
    label5.setEditable (false, false, false);
    label5.setColour (TextEditor::textColourId, Colours::black);
    label5.setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (&sliderRelease);
    sliderRelease.setRange (0.1, 1000, 0.1);
    sliderRelease.setSliderStyle (Slider::RotaryVerticalDrag);
    //sliderRelease.setTextBoxStyle (Slider::TextBoxBelow, false, 80, 20);
    sliderRelease.addListener (this);

    addAndMakeVisible (&label6);
    label6.setFont (Font (15.0000f, Font::plain));
    label6.setJustificationType (Justification::centredLeft);
    label6.setEditable (false, false, false);
    label6.setColour (TextEditor::textColourId, Colours::black);
    label6.setColour (TextEditor::backgroundColourId, Colour (0x0));

    // Set up the sliders
    addAndMakeVisible(&centreFrequencySlider_);
    centreFrequencySlider_.setSliderStyle(Slider::Rotary);
    centreFrequencySlider_.addListener(this);
    centreFrequencySlider_.setRange(10.0, 20000.0, 0.1);

    // Make the centre frequency slider (approximately) logarithmic for a
    // more natural feel
    centreFrequencySlider_.setSkewFactorFromMidPoint(sqrt(10.0 * 20000.0));

    addAndMakeVisible(&qSlider_);
    qSlider_.setSliderStyle(Slider::Rotary);
    qSlider_.addListener(this);
    qSlider_.setRange(0.1, 20.0, 0.01);

    addAndMakeVisible(&gainDecibelsSlider_);
    gainDecibelsSlider_.setSliderStyle(Slider::Rotary);
    gainDecibelsSlider_.addListener(this);
    gainDecibelsSlider_.setRange(-12.0, 12.0, 0.1);

    // This label is informational and exists apart from other controls
    // The other labels are attached to sliders and combo boxes
    addAndMakeVisible(&bandwidthLabel_);
    bandwidthLabel_.setFont(Font (12.0f));

    centreFrequencyLabel_.attachToComponent(&centreFrequencySlider_, false);
    centreFrequencyLabel_.setFont(Font (11.0f));

    qLabel_.attachToComponent(&qSlider_, false);
    qLabel_.setFont(Font (11.0f));

    gainDecibelsLabel_.attachToComponent(&gainDecibelsSlider_, false);
    gainDecibelsLabel_.setFont(Font (11.0f));

    // add the triangular resizer component for the bottom-right of the UI
    resizeLimits_.setSizeLimits(550, 100, 1000, 1000);
    addAndMakeVisible(resizer_ = new ResizableCornerComponent (this, &resizeLimits_));

    // set our component's initial size to be the last one that was stored in the filter's settings
    /*
    setSize(ownerFilter->lastUIWidth_,
            ownerFilter->lastUIHeight_);
            */
    setSize(1000, 500);

    buttonONOFF.setClickingTogglesState(true);

    startTimer(50);
}

Assignment1ProcessorEditor::~Assignment1ProcessorEditor()
{
}

//==============================================================================
void Assignment1ProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::grey);
}

void Assignment1ProcessorEditor::resized()
{
    centreFrequencySlider_.setBounds(20, 20, 150, 40);
    qSlider_.setBounds(200, 20, 150, 40);
    gainDecibelsSlider_.setBounds(380, 20, 150, 40);
    bandwidthLabel_.setBounds(20, 70, 350, 20);


    getProcessor()->lastUIWidth_ = getWidth();
    getProcessor()->lastUIHeight_ = getHeight();

    buttonONOFF.setBounds (17, 55, 125, 25);
    label.setBounds (-3, 0, 500, 49);
    sliderThreshold.setBounds (580, 20, 150, 40);
    label2.setBounds (588, 152, 91, 24);
    sliderRatio.setBounds (780, 20, 150, 40);
    label3.setBounds (132, 152, 50, 24);
    sliderGain.setBounds (595, 77, 150, 40);
    label7.setBounds (604, 52, 52, 24);
    sliderAttack.setBounds (196, 182, 150, 40);
    label5.setBounds (229, 152, 120, 24);
    sliderRelease.setBounds (365, 182, 150, 40);
    label6.setBounds (391, 152, 120, 24);

    resizer_->setBounds(550, 100, 1000, 1000);
}

//==============================================================================
// This timer periodically checks whether any of the filter's parameters have changed...
void Assignment1ProcessorEditor::timerCallback()
{
    Assignment1Processor* ourProcessor = getProcessor();

    centreFrequencySlider_.setValue(ourProcessor->centreFrequency_, dontSendNotification);
    qSlider_.setValue(ourProcessor->q_, dontSendNotification);
    gainDecibelsSlider_.setValue(ourProcessor->gainDecibels_, dontSendNotification);
    updateBandwidthLabel();
}

// This is our Slider::Listener callback, when the user drags a slider.
void Assignment1ProcessorEditor::sliderValueChanged (Slider* slider)
{
    // It's vital to use setParameterNotifyingHost to change any parameters that are automatable
    // by the host, rather than just modifying them directly, otherwise the host won't know
    // that they've changed.

    if (slider == &centreFrequencySlider_)
    {
        getProcessor()->setParameterNotifyingHost (Assignment1Processor::kCentreFrequencyParam,
                                                   (float)centreFrequencySlider_.getValue());
        updateBandwidthLabel();
    }
    else if (slider == &qSlider_)
    {
        getProcessor()->setParameterNotifyingHost (Assignment1Processor::kQParam,
                                                   (float)qSlider_.getValue());
        updateBandwidthLabel();
    }
    else if (slider == &gainDecibelsSlider_)
    {
        getProcessor()->setParameterNotifyingHost (Assignment1Processor::kGainDecibelsParam,
                                                   (float)gainDecibelsSlider_.getValue());
    }
}

void Assignment1ProcessorEditor::updateBandwidthLabel()
{
    char str[64];
    Assignment1Processor* ourProcessor = getProcessor();

    snprintf(str, 64, "Bandw %.1f Hz", ourProcessor->centreFrequency_ / ourProcessor->q_);

    bandwidthLabel_.setText(str, dontSendNotification);
}
void Assignment1ProcessorEditor::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == &buttonONOFF)
    {
        //[UserButtonCode_buttonONOFF] -- add your button handler code here..
        getProcessor()->compressorONOFF = buttonONOFF.getToggleState();
        //[/UserButtonCode_buttonONOFF]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}
