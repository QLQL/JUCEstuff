#pragma once

#ifndef SIGPLOT_H
#define SIGPLOT_H

#include "../JuceLibraryCode/JuceHeader.h"

class Sig1Dplot : public Component
{
public:
	Sig1Dplot() {};
	~Sig1Dplot() {};
	void paint (Graphics& g) override;

	int rowN = 0;
	int* sampleNlist; // sampleN_1,sampleN_2....sampleN_rowN
	float ** plotData; // rows of data to be plotted
	
private:
	
	Colour colourList[6] = { Colours::cyan, Colours::blue, Colours::magenta, Colours::yellow, Colours::red, Colours::white };
	// all the data is in the x range of (0-w normalised to 0-1), and y range of (h-0 normalised to -1 1)
	void drawRowData(Graphics& g, float* buffer, int sampleN); //draw one row of data
	void drawRowData_Bar(Graphics& g, float* buffer, int sampleN);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Sig1Dplot);
};


#endif
