/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 5.0.2

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright (c) 2015 - ROLI Ltd.

  ==============================================================================
*/

#pragma once

//[Headers]     -- You can add your own extra header files here --
#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Projucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class S3AButtonGUI  : public AudioProcessorEditor,
                      public Timer,
                      public SliderListener
{
public:
    //==============================================================================
    S3AButtonGUI (S3abuttonAudioProcessor& p);
    ~S3AButtonGUI();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
	void timerCallback() override
	{
		//processor.noteOnVel = (float)slider->getValue();
		double newFs = processor.getSampleRate();
		if(newFs!=Fs)
		{
			Fs = newFs;
			String fsStr;
			fsStr << Fs;
			fsLabel->setText(fsStr,dontSendNotification);
		}
	}
    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;

    // Binary resources:
    static const char* cat_jpg;
    static const int cat_jpgSize;
    static const char* cat_jpg2;
    static const int cat_jpg2Size;


private:
    //[UserVariables]   -- You can add your own custom variables in this section.
	// This reference is provided as a quick way for your editor to access the processor object that created it
	S3abuttonAudioProcessor& processor;
	double Fs = 0.0;
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<ImageButton> imageButton;
    ScopedPointer<Label> label;
    ScopedPointer<Slider> slider;
    ScopedPointer<Label> label2;
    ScopedPointer<Label> label3;
    ScopedPointer<Label> fsLabel;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (S3AButtonGUI)
};

//[EndFile] You can add extra defines here...
//[/EndFile]
