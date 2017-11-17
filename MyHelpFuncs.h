#pragma once

#ifndef MYHELPFUNCS_H
#define MYHELPFUNCS_H
#include <stdio.h>

//!!!Note the template function can only be implemented in the header file rather than another cpp file being called!
// https://stackoverflow.com/questions/495021/why-can-templates-only-be-implemented-in-the-header-file/495056#495056
// unless you do explicit instantiation of all the types the template will be used with, 
// the linker will be able to find them as usual

// This find the summation over an array
template <typename T>
T arraySum(T* buffer, int num)
{
	T sumV = 0;
	for (int i = 0; i<num; ++i)
	{
		sumV += buffer[i];
	}
	return sumV;
}


#include <algorithm>    // std::min_element, std::max_element
// peak finder functions

template <typename T>
class peakFinder
{
public:
	int peakNum; // number of detected peaks
	int* peakPos; // positions(indices) of peaks
	int* peakVals; // peak values
	int* overallBufferPlot;
	void findPeaks(T** buffer, int frameN, int num);
};

template <typename T>
void peakFinder<T>::findPeaks(T** buffer, int frameN, int num) // suppose it is stationary process, we try to find the peak in the 2D array
{
	
	int maxPeakNum = 3;// at most two peaks
	int* tempPeakPos = new int[maxPeakNum];
	int* tempPeakVals = new int[maxPeakNum];

	int* overallBuffer = new int[num];
	for (int i = 0;i < num;++i)
		overallBuffer[i] = 0;
	for (int frame = 0;frame < frameN;++frame)
	{
		
		T* tempBuffer = new T[num];
		std::copy(buffer[frame], buffer[frame] + num, tempBuffer);

		// find the maximum value in the current frame
		T* peakPosition = std::max_element(tempBuffer, tempBuffer + num);
		int peakPositionIndex = peakPosition - tempBuffer;
		

		T maxV = *peakPosition;
		T newMaxV = maxV;
		while (newMaxV>=0.95*maxV)
		{
			overallBuffer[peakPositionIndex]++;
			tempBuffer[peakPositionIndex] = 0; // we assume the peak value is bigger than 0
			// find the new maximum
			peakPosition = std::max_element(tempBuffer, tempBuffer + num);
			peakPositionIndex = peakPosition - tempBuffer;
			newMaxV = *peakPosition;
		}

	}

	overallBufferPlot = new int[num];
	std::copy(overallBuffer, overallBuffer + num, overallBufferPlot);



	peakNum = 0;
	int maxV;
	for (int i = 0;i < maxPeakNum;++i)
	{
		// find the position of the maximum value in the buffer array
		int* pos1 = std::max_element(overallBuffer, overallBuffer + num);
		int val1 = *pos1; // the current peak value
		int centreIndex = pos1 - overallBuffer;
		
		

		if (i == 0)
		{
			if ((float)val1 > 0.3f*frameN)
			{
				tempPeakPos[i] = centreIndex;
				tempPeakVals[i] = val1;
				peakNum++;
				maxV = val1;
			}
			else
			{
				break;
			}
		}
		else
		{
			if ((float)val1 > 0.4f*maxV)
			{
				tempPeakPos[i] = centreIndex;
				tempPeakVals[i] = val1;
				peakNum++;
			}
			else
			{
				break;
			}
		}

		// remove the peak and its neighbouring region
		for (int j = centreIndex - 2;j < centreIndex + 3;++j)
		{
			if(j>=0 && j<num)
				overallBuffer[j] = 0;
		}


	}
	
	// copy the results to the class member variables
	if (peakNum > 0)
	{
		peakPos = new int[peakNum];
		peakVals = new int[peakNum];
		for (int i = 0;i < peakNum;++i)
		{
			peakPos[i] = tempPeakPos[i];
			peakVals[i] = tempPeakVals[i];
		}
	}
	else
	{
		peakPos = NULL;
		peakVals = NULL;
	}
}





#endif
