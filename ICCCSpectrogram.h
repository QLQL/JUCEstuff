#pragma once

#ifndef ICCCSPECTROGRAM_H
#define ICCCSPECTROGRAM_H

#define EPS 1e-9
#define _USE_MATH_DEFINES
#include <math.h>
#include <complex.h>
#include <numeric>
#include <iostream>

#include "../JuceLibraryCode/JuceHeader.h"
#include "./ffft/FFTReal.h"
//using namespace std;

#include "SigPlot.h"
#include "MyHelpFuncs.h"



class ICCCSpectrogramCal : public Timer
{
public:
	//ScopedPointer<ffft::FFTReal <float>> fft_object = &(ffft::FFTReal <float>(1024));
	ICCCSpectrogramCal()
		: forwardFFT(fftOrder, false)
	{
		//const int frameN = 100;
		//icccSpectrogramImage = Image(Image::RGB, frameN, 21, true);
		for (int frame = 0; frame < frameN; ++frame)
		{
			icccSpectrogramImageVals[frame] = new float[delayCandN];
		}
	};
	~ICCCSpectrogramCal() {};

	//==============================================================================

	void timerCallback() override
	{
		if (nextFFTBlockReady)
		{
			// first, shuffle our image leftwards by 1 pixel..
			const int rightHandEdge = icccSpectrogramImage.getWidth() - 1;
			const int imageHeight = icccSpectrogramImage.getHeight();
			icccSpectrogramImage.moveImageSection(0, 0, 1, 0, rightHandEdge, imageHeight);
			int rowN = frameN; int colN = delayCandN;
			for (int row = 0; row < rowN-1; ++row)
			{
				for (int col = 0; col < colN; ++col)
				{
					icccSpectrogramImageVals[row][col] = icccSpectrogramImageVals[row+1][col];
				}
			}


			long len = 1024;
			ffft::FFTReal <float> fft_object(len);

			if (NumFFT > 10) // Too many blocks of FFT to calulate, where data is over written in the buffer
				NumFFT = 10;
			int tempNumFFT = NumFFT;
			for (int i = 0; i < tempNumFFT; ++i)
			{

				/*
				FFTReal output		| Positive FFT equiv.
				----------------------------------------- 
				f[0]				| Real(bin 0)				
				f[...]				| Real(bin ...)				
				f[length / 2]		| Real(bin length / 2)		
				f[length / 2 + 1]	| Imag(bin 1)				
				f[...]				| Imag(bin ...)				
				f[length - 1]		| Imag(bin length / 2 - 1)	
				*/

				/*Left channel*/
				zeromem(fftData1, sizeof(fftData1));
				memcpy(fftData1, &fifo1[startIndex], sizeof(float)*fftSize);
				fft_object.do_fft(FFTDATA1, fftData1);

				///*to do calculate the magnitude and compare with fftData11*/
				//float fftData11[2 * fftSize]; // left channel
				//zeromem(fftData11, sizeof(fftData11));
				//memcpy(fftData11, &fifo1[startIndex], sizeof(float)*fftSize);
				//// then render our FFT data..
				//forwardFFT.performFrequencyOnlyForwardTransform(fftData11);

				//float magnitude[fftSize / 2 + 1];
				//for (int f = 0; f < fftSize / 2 + 1; ++f)
				//{
				//	if (f == 0 || f == fftSize / 2)
				//	{
				//		magnitude[f] = abs(FFTDATA1[f]);
				//	}
				//	else 
				//	{
				//		magnitude[f] = sqrt(pow(FFTDATA1[f], int(2)) + pow(FFTDATA1[f + fftSize / 2], int(2)));
				//	}
				//}

				/*Right channel*/
				zeromem(fftData2, sizeof(fftData2));
				memcpy(fftData2, &fifo2[startIndex], sizeof(float)*fftSize);
				fft_object.do_fft(FFTDATA2, fftData2);

				/*atan2(imag,real)*/
				/*float aa = atan2(0.0f, 1.0f);
				std::cout << aa << newLine;
				aa = atan2(1.0f, 0.0f);
				std::cout << aa << newLine;
				aa = atan2(1.0f, 1.0f);
				std::cout << aa << newLine;*/

				/*Calculate the phase of each fft point using atan2(imag,real)*/
				for (int f = 0; f < fftSize / 2 + 1; ++f)
				{
					if (f == 0 || f == fftSize / 2)
					{
						phaseArray[f] = atan2(0.0f, FFTDATA1[f] * FFTDATA2[f]);
					} else {
						realPart = FFTDATA1[f] * FFTDATA2[f] - FFTDATA1[f + fftSize / 2] * FFTDATA2[f + fftSize / 2];
						imagPart = FFTDATA1[f] * FFTDATA2[f + fftSize / 2] + FFTDATA1[f + fftSize / 2] * FFTDATA2[f];
						phaseArray[f] = atan2(imagPart, realPart);
					}
				}


				for (int i = 0; i < delayCandN; ++i) // for each candidate delay
				{
					for (int f = 0; f < fftSize / 2 + 1; ++f) // for each frequency bin
					{
						cosPhaseShiftArray[f] = cos(phaseArray[f] - jwtau[f][i]);
					}
					float level = std::accumulate(cosPhaseShiftArray, cosPhaseShiftArray + fftSize / 2 + 1, 0.0f);
					level /= float(fftSize / 2 + 1);
					//set this value in the image
					icccSpectrogramImage.setPixelAt(rightHandEdge, i, Colour::fromHSV(level, 1.0f, level, 1.0f));
					icccSpectrogramImageVals[rowN - 1][i] = level;
				}
				
				

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

			//update the overall ICCC results as the average over 100 frames
			/*for (int i = 0; i < delayCandN; ++i)
			{
				float temp = std::accumulate(icccSpectrogramImageVals[i], icccSpectrogramImageVals[i] + 100, 0.0f);
				temp /= 100.0f;
				overallICCC[i] = temp;
			}*/

			// Apply an average sampling to the ICCC scores at different frames
			//int frameN = 10;
			for (int frame = 0; frame < frameN; ++frame)
			{
				if (frame == 0)
				{
					FloatVectorOperations::copy(overallICCC, icccSpectrogramImageVals[frame], delayCandN);
				}
				else
				{
					FloatVectorOperations::add(overallICCC, icccSpectrogramImageVals[frame], delayCandN);
				}
			}
			if (frameN > 1)
			{
				FloatVectorOperations::multiply(overallICCC, 1.0 / frameN, delayCandN);
			}

			//repaint();
			
			String message;
			peakFinder<float> myPeakFinder;
			myPeakFinder.findPeaks(icccSpectrogramImageVals, frameN, delayCandN);
			if (myPeakFinder.peakPos != NULL)
			{
				message.clear();
				message << "The peak delay is " << candiDelays[myPeakFinder.peakPos[0]] << " ms" << newLine;
				Logger::getCurrentLogger()->writeToLog(message);
			}
			

			// copy the results to plotbuffer
			float** plotBuffer = new float*[2]; // here we plot only one row of data
			plotBuffer[0] = new float[delayCandN];
			plotBuffer[1] = new float[delayCandN];
			std::copy(overallICCC, overallICCC + delayCandN, plotBuffer[0]);
			std::copy(myPeakFinder.overallBufferPlot, myPeakFinder.overallBufferPlot + delayCandN, plotBuffer[1]);
			float maxV = *std::max_element(plotBuffer[1], plotBuffer[1] + delayCandN);
			FloatVectorOperations::multiply(plotBuffer[1], 1.0f / maxV, delayCandN);

			mySig1Dplot.rowN = 2;
			mySig1Dplot.sampleNlist = new int[2];
			mySig1Dplot.sampleNlist[0] = delayCandN;
			mySig1Dplot.sampleNlist[1] = delayCandN;
			mySig1Dplot.plotData = new float*[2];
			mySig1Dplot.plotData[0] = new float[delayCandN];
			mySig1Dplot.plotData[1] = new float[delayCandN];
			FloatVectorOperations::copy(mySig1Dplot.plotData[0], plotBuffer[0], delayCandN);
			FloatVectorOperations::copy(mySig1Dplot.plotData[1], plotBuffer[1], delayCandN);
			mySig1Dplot.repaint();
		}
	}

	void pushNextSampleIntoFifo(float sample1, float sample2) noexcept
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

		fifo1[fifoIndex] = sample1;
		fifo2[fifoIndex] = sample2;
		fifoIndex++;
	}

	void initParams(double Fs) noexcept
	{
		this->Fs = Fs;
		/*Candidate delays in ms*/
		for (int i = 0; i < delayCandN; ++i)
			candiDelays[i] = (int)(-maxTau + i*2.0*maxTau / (delayCandN - 1));
		for (int f = 0; f < fftSize / 2 + 1; ++f)
		{
			float w = 2 * M_PI*f / Fs;
			for (int i = 0; i < delayCandN; ++i)
			{
				jwtau[f][i] = w*candiDelays[i];
			}
		}
		
	}


	enum
	{
		fftOrder = 10,
		fftSize = 1 << fftOrder
	};


	// This saves the buffer data for FFT calculation
	int bufferLength = fftSize * 10;
	float fifo1[fftSize * 10];
	float fifo2[fftSize * 10];

	
	// we calculate the ICCC w.r.t candidates taus, then a max pooling process is applied to all the frames
	// ICCC is directly calculated in the time domain
	const double maxTau = 1.0;
	const int delayCandN = 21; // number of candidate delays
	double candiDelays[21];
	Image icccSpectrogramImage = Image(Image::RGB, 100, 21, true); // 100 frames
	//float icccSpectrogramImageVals[10][21] = { };
	//float overallICCC[21] = {};
	// allocate space to save the ICCC results
	int frameN = 40;
	float** icccSpectrogramImageVals = new float *[frameN];
	float* overallICCC = new float[delayCandN];
	double Fs = 48000;
	Sig1Dplot mySig1Dplot; // to plot the overall ICCC results
	
	

private:
	FFT forwardFFT;

	
	float fftData1[fftSize]; // left channel
	float fftData2[fftSize]; // right channel
	float FFTDATA1[fftSize]; // left channel
	float FFTDATA2[fftSize]; // right channel

	int fifoIndex = 0;
	int startIndex = 0;// the starting index for a segment (fftSize*NumFFT) that start the fft
	int NumFFT = 0; // how many fftblocks do we need to perform when timer is called
	bool nextFFTBlockReady;

	double jwtau[fftSize/2+1][21]; // saves the angle information of exp(j*w*tau)
	float phaseArray[fftSize / 2 + 1], cosPhaseShiftArray[fftSize / 2 + 1];
	float realPart, imagPart;


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ICCCSpectrogramCal)
};



#endif