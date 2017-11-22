#include "SigPlot.h"

void Sig1Dplot::paint(Graphics& g) 
{
	g.fillAll(Colours::darkslateblue);

	// draw the grid line
	g.setColour(Colours::white);
	g.setOpacity(0.5f);
	auto w = static_cast<float>(getWidth());
	auto h = static_cast<float>(getHeight());
	auto lineHorizontal = Line<float>(0.f, h * 0.5f, w, h * 0.5f);
	auto lineVertical = Line<float>(w * 0.5f, h, w * 0.5f, 0.f);
	float dashes[] = { 3, 2 };
	g.drawArrow(lineHorizontal, 2.0f, w/20, w/30);
	//g.drawArrow(lineVertical, 2.0f, w / 20, w / 30);
	g.drawDashedLine(lineVertical, dashes, 2);
	g.setFont(Font("Arial", jmax((int)(w*0.05),8), Font::plain).withTypefaceStyle("Italic"));
	g.drawSingleLineText("tau", (int)(w*0.9f),(int)(h*0.6));
	/*if (rowN > 0)
	{
		for (int row = 0;row < rowN;++row)
		{
			int colorN = sizeof(colourList) / sizeof(colourList[0]);
			g.setColour(colourList[row % colorN]);
			drawRowData(g, plotData[row], sampleNlist[row]);
		}
	}*/

	if (rowN > 0)
	{
		int colorN = sizeof(colourList) / sizeof(colourList[0]);
		g.setColour(colourList[0]);
		drawRowData(g, plotData[0], sampleNlist[0]);

		g.setColour(Colours::lightgrey);
		g.setOpacity(0.5f);
		drawRowData_Bar(g, plotData[1], sampleNlist[1]);
	}
	
}


void Sig1Dplot::drawRowData(Graphics& g, float* buffer, int sampleN) //draw one row of data
{
	// You can add your drawing code here!
	// First do the mapping
	auto w = static_cast<float>(getWidth());
	auto h = static_cast<float>(getHeight());
	auto delta_w = 0.04f*w, delta_h = 0.04f*h;
	float* xarray = new float[sampleN];
	float* yarray = new float[sampleN];
	auto scale_x = (w - delta_w*2.0f) / (sampleN-1), shift_x = delta_w;
	auto scale_y = -(h - delta_h*2.0f) / 2.0f, shift_y = 0.5f*h;
	for (int i = 0;i < sampleN;++i)
	{
		xarray[i] = (float)i*scale_x + shift_x;
		yarray[i] = (float)buffer[i]*scale_y + shift_y;
	}
	

	Path myPath;
	for (int i = 0;i < sampleN;++i)
	{
		if (i == 0)
		{
			myPath.startNewSubPath(xarray[0], yarray[0]);        
		}
		else
		{
			myPath.lineTo(xarray[i], yarray[i]);
		}
	}

	// double the width of the whole thing..
	// myPath.applyTransform(AffineTransform::scale(2.0f, 1.0f));

	// and draw it to a graphics context with a 5-pixel thick outline.
	g.strokePath(myPath, PathStrokeType(3.0f));

}

void Sig1Dplot::drawRowData_Bar(Graphics& g, float* buffer, int sampleN) //draw bar plot of one row of data
{
	// You can add your drawing code here!
	// First do the mapping
	auto w = static_cast<float>(getWidth());
	auto h = static_cast<float>(getHeight());
	auto delta_w = 0.04f*w, delta_h = 0.04f*h;
	float* xarray = new float[sampleN];
	float* yarray = new float[sampleN];
	auto scale_x = (w - delta_w*2.0f) / (sampleN - 1), shift_x = delta_w;
	auto scale_y = -(h - delta_h*2.0f) / 2.0f, shift_y = 0.5f*h;
	auto barWidth = (w - delta_w*2.0f) / (sampleN - 1)*0.8f;

	for (int i = 0;i < sampleN;++i)
	{
		xarray[i] = (float)i*scale_x + shift_x - barWidth*0.5f;
		yarray[i] = (float)buffer[i] * scale_y + shift_y;
	}

	

	RectangleList<float> myRecList;
	//RectangleList< float > *myRecList = new RectangleList< float >[sampleN];

	for (int i = 0;i < sampleN;++i)
	{
		auto barHeight = 0.5f*h - yarray[i]; //Note the bar height must be bigger than 0, otherwise, the rectangle is not valid.
		Rectangle<float> currentRec;
		if (barHeight>0)
		{ 
			currentRec = Rectangle<float>(xarray[i], yarray[i], barWidth, barHeight);
		}
		else
		{
			if(barHeight==0)
				currentRec = Rectangle<float>(xarray[i], yarray[i], barWidth, 1.0f);
			else
				currentRec = Rectangle<float>(xarray[i], 0.5f*h, barWidth, -barHeight);
		}
		myRecList.add(currentRec);
	}

	g.fillRectList(myRecList);

	
	auto it = myRecList.begin();
	
	// iterate over the list to plot the bound
	g.setColour(Colours::red);
	for (auto i = myRecList.begin();i!=myRecList.end();++i)
	{
		auto currentRec = *i;
		g.drawRect(currentRec, 2.0f);
	}

	///*g.setColour(Colours::red);
	//for (int i = 0;i < sampleN;++i)
	//{
	//	Rectangle<float> currentRec = Rectangle<float>(xarray[i], yarray[i], barWidth, 0.5*h - yarray[i]);
	//	g.drawRect(currentRec,2.0f);
	//}*/

}
