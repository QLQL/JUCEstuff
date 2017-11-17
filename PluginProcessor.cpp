/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "EasyGUI.h"
//#include "SpectrogramPlot.h"
//using namespace std;

//==============================================================================
S3abuttonAudioProcessor::S3abuttonAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
	plotSpect.startTimer(100);
	iccc.startTimer(2000);
}

S3abuttonAudioProcessor::~S3abuttonAudioProcessor()
{
}

//==============================================================================
const String S3abuttonAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool S3abuttonAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool S3abuttonAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

double S3abuttonAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int S3abuttonAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int S3abuttonAudioProcessor::getCurrentProgram()
{
    return 0;
}

void S3abuttonAudioProcessor::setCurrentProgram (int index)
{
}

const String S3abuttonAudioProcessor::getProgramName (int index)
{
    return {};
}

void S3abuttonAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void S3abuttonAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
	iccc.initParams(sampleRate);
}

void S3abuttonAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool S3abuttonAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void S3abuttonAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
		for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
		{
			channelData[sample] *= noteOnVel;
		}

		// push the data to a buffer for FFT calculation
		if (channel == 0)
		{
			for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
			{
				plotSpect.pushNextSampleIntoFifo(channelData[sample]);
			}
				
		}

    }

	// push the data to calculate the ICCC
	if (totalNumInputChannels >= 2) // at least two channels
	{
		float* channelData1 = buffer.getWritePointer(0);
		float* channelData2 = buffer.getWritePointer(1);
		for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
		{
			iccc.pushNextSampleIntoFifo(channelData1[sample], channelData2[sample]);
		}
	}

}

//==============================================================================
bool S3abuttonAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* S3abuttonAudioProcessor::createEditor()
{
	return new S3AButtonGUI(*this);
}

//==============================================================================
void S3abuttonAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void S3abuttonAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new S3abuttonAudioProcessor();
}
