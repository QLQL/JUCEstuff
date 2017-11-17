#pragma once

#ifndef SPECTROGRAMPLOT_H
#define SPECTROGRAMPLOT_H

#include <cmath>
#include "../JuceLibraryCode/JuceHeader.h"
//using namespace std;

class SpectrogramComponent : public Component,
							 public Timer
{
public:
	SpectrogramComponent()
		: forwardFFT(fftOrder, false)
	{
		const int frameN = 512;
		spectrogramImage = Image(Image::RGB, frameN, 512, true);
		zeromem(realMaxLevelArray, sizeof(realMaxLevelArray));
	};
	~SpectrogramComponent() {};

	//==============================================================================
	void paint(Graphics& g) override
	{
		g.fillAll(Colours::black);

		g.setOpacity(1.0f);
		g.drawImage(spectrogramImage, getLocalBounds().toFloat());
	}


	void timerCallback() override
	{
		if (nextFFTBlockReady)
		{
			if (NumFFT > 10) // Too many blocks of FFT to calulate, where data is over written in the buffer
				NumFFT = 10;
			int tempNumFFT = NumFFT;
			for (int i = 0; i < tempNumFFT; ++i)
			{
				zeromem(fftData, sizeof(fftData));
				memcpy(fftData, &fifo[startIndex], sizeof(float)*fftSize);
				//memcpy(fftData, fifo , sizeof(float)*fftSize);
				drawNextLineOfSpectrogram();
				startIndex += fftSize;
				if (startIndex >= bufferLength)
				{ 
					startIndex = 0;
				}
			}
			nextFFTBlockReady = false;

			//After the above processing, maybe NumFFT has changed
			if (tempNumFFT < NumFFT)
			{
				NumFFT -= tempNumFFT;
				nextFFTBlockReady = true;
			}
			
			repaint();
		}
	}

	void pushNextSampleIntoFifo(float sample) noexcept
	{
		// if the fifo contains enough data, set a flag to say
		// that the next line should now be rendered..
		if ((fifoIndex%fftSize == 0) && (fifoIndex > 0))
		{
			nextFFTBlockReady = true;
			if (fifoIndex == bufferLength)
			{
				fifoIndex = 0;
			}
			NumFFT += 1;
		}

		fifo[fifoIndex++] = sample;
	}

	void drawNextLineOfSpectrogram()
	{
		const int rightHandEdge = spectrogramImage.getWidth() - 1;
		const int imageHeight = spectrogramImage.getHeight();

		// first, shuffle our image leftwards by 1 pixel..
		spectrogramImage.moveImageSection(0, 0, 1, 0, rightHandEdge, imageHeight);

		// then render our FFT data..
		forwardFFT.performFrequencyOnlyForwardTransform(fftData);

		// find the range of values produced, so we can scale our rendering to
		// show up the detail clearly
		Range<float> maxLevel = FloatVectorOperations::findMinAndMax(fftData, fftSize / 2);
		float realMaxLevel = jmax(maxLevel.getEnd(), 1e-5f);
		//// shift the max values in the array to the left by 1 frame
		//int N = int (sizeof(realMaxLevelArray) / sizeof(realMaxLevelArray[0]));
		//for (int i = 0; i <  N - 1; ++i)
		//{
		//	realMaxLevelArray[i] = realMaxLevelArray[i + 1];
		//}
		//realMaxLevelArray[N-1] = realMaxLevel;
		//float tempMax = * (std::max_element(realMaxLevelArray, realMaxLevelArray + N));
		//if ( (tempMax>useMaxLevel) || (tempMax< useMaxLevel/4.0f) )
		//{
		//	useMaxLevel = tempMax * 1.5f;
		//}
		
	

		// print some temporary results
		/*String message;
		message.clear();
		message << "Following is the relationship between pixels and (frequency bins)." << newLine;*/
		for (int y = 1; y < imageHeight; ++y)
		{
			const float skewedProportionY = 1.0f - std::exp(std::log(y / (float)imageHeight) * 0.2f);
			const int fftDataIndex = jlimit(0, fftSize / 2, (int)(skewedProportionY * fftSize / 2));
			//const float level = jmap(fftData[fftDataIndex], 0.0f, useMaxLevel, 0.0f, 1.0f);
			const float level = jmap(fftData[fftDataIndex], 0.0f, realMaxLevel, 0.0f, 1.0f);
			//message << y << "(" << fftDataIndex << ")    ";
			spectrogramImage.setPixelAt(rightHandEdge, y, Colour::fromHSV(level, 1.0f, level, 1.0f));
		}
		/*message << newLine;
		Logger::getCurrentLogger()->writeToLog(message);
		message.clear();*/
	}


	enum
	{
		fftOrder = 10,
		fftSize = 1 << fftOrder
	};


	// This saves the buffer data for FFT calculation
	int bufferLength = fftSize * 10;
	float fifo[fftSize * 10];

private:
	FFT forwardFFT;
	Image spectrogramImage;
	float realMaxLevelArray[20]; // hold the maximum values in the last few frames
	float useMaxLevel = 1e-5f;

	float fftData[2 * fftSize];
	int fifoIndex = 0;
	int startIndex = 0;// the starting index for a segment (fftSize*NumFFT) that start the fft
	int NumFFT = 0; // how many fftblocks do we need to perform when timer is called
	bool nextFFTBlockReady;



	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrogramComponent)
};



#endif