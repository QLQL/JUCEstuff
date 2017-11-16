#pragma once

#ifndef ICCCSPECTROGRAM_H
#define ICCCSPECTROGRAM_H

#include "../JuceLibraryCode/JuceHeader.h"
#include "./ffft/FFTReal.h"
//using namespace std;


//long len = 1024;
//extern ffft::FFTReal <float> fft_object(len);

class ICCCSpectrogramCal : public Timer
{
public:
	//ScopedPointer<ffft::FFTReal <float>> fft_object = &(ffft::FFTReal <float>(1024));
	ICCCSpectrogramCal()
		: forwardFFT(fftOrder, false)//,
		//fft_object(&(ffft::FFTReal <float> (1024)))
	{
		const int frameN = 100;
		icccSpectrogramImage = Image(Image::RGB, frameN, 512, true);
		//long len2 = fft_object->get_length();
	};
	~ICCCSpectrogramCal() {};

	//==============================================================================
	

	void timerCallback() override
	{
		if (nextFFTBlockReady)
		{
			long len = 1024;
			ffft::FFTReal <float> fft_object(len);

			if (NumFFT > 10) // Too many blocks of FFT to calulate, where data is over written in the buffer
				NumFFT = 10;
			int tempNumFFT = NumFFT;
			for (int i = 0; i < tempNumFFT; ++i)
			{
				/*Left channel*/
				zeromem(fftData11, sizeof(fftData11));
				memcpy(fftData11, &fifo1[startIndex], sizeof(float)*fftSize);
				// then render our FFT data..
				forwardFFT.performFrequencyOnlyForwardTransform(fftData11);

				zeromem(fftData1, sizeof(fftData1));
				memcpy(fftData1, &fifo1[startIndex], sizeof(float)*fftSize);
				//float f[1024];
				fft_object.do_fft(FFTDATA1, fftData1);
				/*to do calculate the magnitude and compare with fftData11*/

				///*Right channel*/
				//zeromem(fftData2, sizeof(fftData2));
				//memcpy(fftData2, &fifo1[startIndex], sizeof(float)*fftSize);
				//// then render our FFT data..
				//forwardFFT.performFrequencyOnlyForwardTransform(fftData2);

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
		/*Candidate delays in ms*/
		for (int i = 0; i < delayCandN; ++i)
			candiDelays[i] = (int)(-maxTau + i*2.0*maxTau / (delayCandN - 1));
		
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
	

private:
	FFT forwardFFT;
	Image icccSpectrogramImage;

	//float fftData1[2 * fftSize]; // left channel
	//float fftData2[2 * fftSize]; // right channel
	float fftData11[2 * fftSize]; // left channel
	float fftData1[fftSize]; // left channel
	float fftData2[fftSize]; // right channel
	float FFTDATA1[fftSize]; // left channel
	float FFTDATA2[fftSize]; // right channel

	int fifoIndex = 0;
	int startIndex = 0;// the starting index for a segment (fftSize*NumFFT) that start the fft
	int NumFFT = 0; // how many fftblocks do we need to perform when timer is called
	bool nextFFTBlockReady;

	double jwtau[21][fftSize];


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ICCCSpectrogramCal)
};



#endif