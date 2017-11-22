#pragma once

#ifndef ICCCSPECTROGRAM_H
#define ICCCSPECTROGRAM_H

#define EPS 1e-9
#define _USE_MATH_DEFINES
#include <math.h>
#include <complex.h>
#include <numeric>
#include <iostream>
#include <algorithm>

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
		
		for (int frame = 0; frame < frameN; ++frame)
			icccSpectrogramImageVals[frame] = new float[delayCandN];

		mySig1Dplot.rowN = 0; // Do not sign any value before initialising the plotData !!! Otherwise, the addaddmakevisiable will behave funny
		mySig1Dplot.sampleNlist = new int[2];
		mySig1Dplot.sampleNlist[0] = delayCandN;
		mySig1Dplot.sampleNlist[1] = delayCandN;
		mySig1Dplot.plotData = new float*[2];
		mySig1Dplot.plotData[0] = new float[delayCandN];
		mySig1Dplot.plotData[1] = new float[delayCandN];
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

			float * temp = icccSpectrogramImageVals[0];
			for (int frame = 0; frame < frameN - 1; ++frame)
			{
				icccSpectrogramImageVals[frame] = icccSpectrogramImageVals[frame+1];
			}
			icccSpectrogramImageVals[frameN - 1] = temp;

			/*for (int frame = 0; frame < frameN -1; ++frame)
			{
				for (int i = 0; i < delayCandN; ++i)
				{
					icccSpectrogramImageVals[frame][i] = icccSpectrogramImageVals[frame+1][i];
				}
			}*/


			long len = 1024;
			ffft::FFTReal <float> fft_object(len);

			if (NumFFT > NumFFTLimit) // Too many blocks of FFT to calulate, where data is over written in the buffer
				NumFFT = NumFFTLimit;
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
				bool signalHasValues = false;
				for (int f = fStartInd; f < fEndInd; ++f)
				{
					if (f == 0 || f == fftSize / 2)
					{
						realPart = FFTDATA1[f] * FFTDATA2[f];
						imagPart = 0.0f;
					} else { // X1f*conj(X2f)  = (a1+b1j)(a2-b2j) = (a1a2+b1b2)+(-a1b2+b1a2)j
						realPart = FFTDATA1[f] * FFTDATA2[f] + FFTDATA1[f + fftSize / 2] * FFTDATA2[f + fftSize / 2];
						imagPart = -FFTDATA1[f] * FFTDATA2[f + fftSize / 2] + FFTDATA1[f + fftSize / 2] * FFTDATA2[f];
					}
					phaseArray[f] = atan2(imagPart, realPart);
					if (!signalHasValues)
					{
						if (imagPart != 0 || realPart != 0)
						{
							signalHasValues = true;
						}
					}
					
				}


				for (int i = 0; i < delayCandN; ++i) // for each candidate delay
				{
					float level;
					if (signalHasValues)
					{
						for (int f = fStartInd; f < fEndInd; ++f) // for each frequency bin
						{
							cosPhaseShiftArray[f] = cos(phaseArray[f] - jwtau[f][i]);
						}
						level = std::accumulate(cosPhaseShiftArray + fStartInd, cosPhaseShiftArray + fEndInd, 0.0f);
						level /= float(fUseNum);
					}
					else {
						level = 1.0f/ float(fUseNum);
					}
					
					//set this value in the image
					icccSpectrogramImage.setPixelAt(rightHandEdge, i, Colour::fromHSV(level, 1.0f, level, 1.0f));
					icccSpectrogramImageVals[frameN - 1][i] = level;
				}

				//// print out the ICCC values
				//message.clear();
				////String messageee;
				//message << "Following is the ICCC over different frames." << newLine;
				//Logger::getCurrentLogger()->writeToLog(message);
				//for (int frame = 0; frame < frameN; ++frame)
				//{
				//	message.clear();
				//	message << "Frame----" << frame + 1 << newLine;
				//	for (int i = 0; i < delayCandN; ++i)
				//	{
				//		message << icccSpectrogramImageVals[frame][i] << "   ";
				//	}
				//	message << newLine;
				//	Logger::getCurrentLogger()->writeToLog(message);
				//}
				
				

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


			 //Apply an average sampling to the ICCC scores at different frames
			for (int frame = 0; frame < frameN; ++frame)
			{
				if (frame == 0)
					FloatVectorOperations::copy(overallICCC, icccSpectrogramImageVals[frame], delayCandN);
				else
					FloatVectorOperations::add(overallICCC, icccSpectrogramImageVals[frame], delayCandN);
			}
			if (frameN > 1)
				FloatVectorOperations::multiply(overallICCC, 1.0 / frameN, delayCandN);

					
			peakFinder<float> myPeakFinder;
			myPeakFinder.findPeaks(icccSpectrogramImageVals, frameN, delayCandN);
			if (myPeakFinder.peakPos != NULL)
			{
				message.clear();
				message << "The peak delay is " << candiDelays[myPeakFinder.peakPos[0]] << " ms" << newLine;
				Logger::getCurrentLogger()->writeToLog(message);
			}
			

			mySig1Dplot.rowN = 2;
			// copy the results to plotbuffer
			std::copy(overallICCC, overallICCC + delayCandN, mySig1Dplot.plotData[0]);
			std::copy(myPeakFinder.overallBufferPlot, myPeakFinder.overallBufferPlot + delayCandN, mySig1Dplot.plotData[1]);
			float maxV = *std::max_element(mySig1Dplot.plotData[1], mySig1Dplot.plotData[1] + delayCandN);
			if (maxV > 0)
			{
				FloatVectorOperations::multiply(mySig1Dplot.plotData[1], 1.0f / maxV, delayCandN);
			}
			

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
		fStartInd = int(300.0f / Fs*fftSize);
		fEndInd = std::min(int(4000.0f / Fs*fftSize), fftSize/2);
		fUseNum = fEndInd - fStartInd;

		/*Candidate delays in ms*/
		for (int i = 0; i < delayCandN; ++i)
			candiDelays[i] = -maxTau + i*2.0*maxTau / (delayCandN - 1);
		for (int f = 0; f < fftSize / 2 + 1; ++f)
		{
			float w = 2 * M_PI*f / fftSize * Fs;
			w /= 1000.0f;
			for (int i = 0; i < delayCandN; ++i)
			{
				jwtau[f][i] = w*candiDelays[i];
			}
		}

		for (int frame = 0; frame < frameN; ++frame)
		{
			for (int i = 0; i < delayCandN; ++i)
			{
				icccSpectrogramImageVals[frame][i] = 0;
			}
				
		}
			

		

		/*message.clear();
		message << "The candidate delays in ms are " << newLine;
		for (int i = 0; i < delayCandN; ++i)
			message<< candiDelays[i] << " " << newLine;
		Logger::getCurrentLogger()->writeToLog(message);

		message.clear();
		message << "Following is the phase information of jwtau." << newLine;
		Logger::getCurrentLogger()->writeToLog(message);
		for (int f = 0; f < fftSize / 2 + 1; ++f)
		{
			message.clear();
			float w = 2 * M_PI*f / fftSize * Fs;
			message << "Angular frequency----" << f << "----" << w << newLine;
			for (int i = 0; i < delayCandN; ++i)
			{
				message << jwtau[f][i] << "   ";
			}
			message << newLine;
			Logger::getCurrentLogger()->writeToLog(message);
		}*/
		
	}


	enum
	{
		fftOrder = 10,
		fftSize = 1 << fftOrder
	};



	Sig1Dplot mySig1Dplot; // to plot the overall ICCC results
	
	

private:
	FFT forwardFFT;

	// This saves the buffer data for FFT calculation
	int NumFFTLimit = 40; // at most doing this much
	int bufferLength = fftSize * 40;
	float fifo1[fftSize * 40];
	float fifo2[fftSize * 40];

	float fftData1[fftSize]; // left channel batch of data
	float fftData2[fftSize]; // right channel batch of data
	float FFTDATA1[fftSize]; // left channel FFT result
	float FFTDATA2[fftSize]; // right channel FFT result

	int fifoIndex = 0;
	int startIndex = 0;// the starting index for a segment (fftSize*NumFFT) that start the fft
	int NumFFT = 0; // how many fftblocks do we need to perform when timer is called
	bool nextFFTBlockReady;

	double jwtau[fftSize/2+1][21]; // saves the angle information of exp(j*w*tau)
	float phaseArray[fftSize / 2 + 1], cosPhaseShiftArray[fftSize / 2 + 1];
	float realPart, imagPart;
	int fStartInd, fEndInd, fUseNum;

	String message;

	


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

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ICCCSpectrogramCal)
};



#endif