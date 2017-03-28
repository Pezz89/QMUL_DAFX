/*
 ==============================================================================

 This file is part of the JUCE library.
 Copyright (c) 2015 - ROLI Ltd.

 Permission is granted to use this software under the terms of either:
 a) the GPL v2 (or any later version)
 b) the Affero GPL v3

 Details of these licenses can be found at: www.gnu.org/licenses

 JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

 ------------------------------------------------------------------------------

 To release a closed-source product which uses JUCE, commercial licenses are
 available: visit www.juce.com for more information.

 ==============================================================================
 */

class GenericEditor : public AudioProcessorEditor,
                      public SliderListener,
                      public Button::Listener,
                      private Timer
{
public:
    enum
    {
        kParamSliderHeight = 25,
        kParamLabelWidth = 130,
        kParamSliderWidth = 300
    };

    GenericEditor (AudioProcessor& parent)
        : AudioProcessorEditor (parent),
          noParameterLabel ("noparam", "No parameters available")
    {
        const OwnedArray<AudioProcessorParameter>& params = parent.getParameters();
        for (int i = 0; i < params.size(); ++i)
        {
            if (const AudioParameterFloat* param = dynamic_cast<AudioParameterFloat*> (params[i]))
            {
                Slider* aSlider;
                paramSliders.add (aSlider = new Slider (param->name));
                aSlider->setRange (param->range.start, param->range.end, 0);
                aSlider->setSliderStyle (Slider::LinearHorizontal);
                aSlider->setValue (*param);
                aSlider->addListener (this);
                addAndMakeVisible (aSlider);
                Label* aLabel;
                paramLabels.add (aLabel = new Label (param->name, param->name));
                addAndMakeVisible (aLabel);
                controls.add (aSlider);  // [11]
            }
            else if (const AudioParameterBool* param = dynamic_cast<AudioParameterBool*> (params[i])) // [10]
            {
                ToggleButton* aButton;
                paramToggles.add (aButton = new ToggleButton (param->name));
                aButton->setToggleState (*param, dontSendNotification);
                aButton->addListener (this);
                addAndMakeVisible (aButton);
                controls.add (aButton);
            }
        }

        noParameterLabel.setJustificationType (Justification::horizontallyCentred | Justification::verticallyCentred);
        noParameterLabel.setFont (noParameterLabel.getFont().withStyle (Font::italic));

        setSize (kParamSliderWidth + kParamLabelWidth,
                 jmax (1, kParamSliderHeight * (paramSliders.size() + paramToggles.size())));

        if (paramSliders.size() == 0)
            addAndMakeVisible (noParameterLabel);
        else
            startTimer (100);
    }

    ~GenericEditor()
    {
    }

    void resized() override
    {
        Rectangle<int> r = getLocalBounds();
        noParameterLabel.setBounds (r);
        for (int i = 0; i < controls.size(); ++i)
        {
            Rectangle<int> paramBounds = r.removeFromTop (kParamSliderHeight);
            if (Slider* aSlider = dynamic_cast<Slider*> (controls[i]))
            {
                Rectangle<int> labelBounds = paramBounds.removeFromLeft (kParamLabelWidth);
                const int sliderIndex = paramSliders.indexOf (aSlider);
                paramLabels[sliderIndex]->setBounds (labelBounds);
                aSlider->setBounds (paramBounds);
            }
            else if (ToggleButton* aButton = dynamic_cast<ToggleButton*> (controls[i]))
            {
                aButton->setBounds (paramBounds);
            }
        }
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::white);
    }

    //==============================================================================
    void sliderValueChanged (Slider* slider) override
    {
        if (AudioParameterFloat* param = dynamic_cast<AudioParameterFloat*>(getParameterForSlider (slider)))
        {
            /*
            if (slider->isMouseButtonDown())
                param->setValueNotifyingHost ((float) slider->getValue());
            else
                param->setValue ((float) slider->getValue());
            */
            *param = (float) slider->getValue();
        }
    }

    void sliderDragStarted (Slider* slider) override
    {
        if (AudioProcessorParameter* param = getParameterForSlider (slider))
            param->beginChangeGesture();
    }

    void sliderDragEnded (Slider* slider) override
    {
        if (AudioProcessorParameter* param = getParameterForSlider (slider))
            param->endChangeGesture();
    }


    void buttonClicked (Button* button) override
    {
        if (AudioParameterBool* param = getParameterForButton (button))
        {
            param->beginChangeGesture();
            *param = button->getToggleState();
            param->endChangeGesture();
        }
    }
private:
    void timerCallback() override
    {
        const OwnedArray<AudioProcessorParameter>& params = getAudioProcessor()->getParameters();
        for (int i = 0; i < controls.size(); ++i)
        {
            if (Slider* slider = dynamic_cast<Slider*> (controls[i]))
            {
                AudioParameterFloat* param = static_cast<AudioParameterFloat*> (params[i]);
                slider->setValue ((double) *param, dontSendNotification);
            }
            else if (Button* button = dynamic_cast<Button*> (controls[i]))
            {
                AudioParameterBool* param = static_cast<AudioParameterBool*> (params[i]);
                button->setToggleState (*param, dontSendNotification);
            }
        }
    }

    AudioProcessorParameter* getParameterForSlider (Slider* slider)
    {
        const OwnedArray<AudioProcessorParameter>& params = getAudioProcessor()->getParameters();
        return dynamic_cast<AudioParameterFloat*> (params[controls.indexOf (slider)]);
    }

    AudioParameterBool* getParameterForButton (Button* button)
    {
        const OwnedArray<AudioProcessorParameter>& params = getAudioProcessor()->getParameters();
        return dynamic_cast<AudioParameterBool*> (params[controls.indexOf (button)]);
    }

    Label noParameterLabel;
    OwnedArray<Slider> paramSliders;
    OwnedArray<Label> paramLabels;
    OwnedArray<Button> paramToggles;  // [8]
    Array<Component*> controls;
};
