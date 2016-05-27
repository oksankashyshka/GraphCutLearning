#ifndef _GRAPH__INITIALIZATION_HPP_
#define _GRAPH__INITIALIZATION_HPP_

#include <opencv2/highgui/highgui.hpp>
#include <vector>

#include "Graph.hpp"

struct myPoint
{
	int y, x;
	myPoint(int Y_, int X_) : y(Y_), x(X_){}
};

typedef int number;
typedef Graph::weightT weightT;
typedef signed short int coordinateT;
typedef unsigned char colourT;

typedef cv::Vec3b rgbT;
//typedef cv::Point xyT;
typedef myPoint xyT;
typedef std::vector < xyT > vec_pointT;
typedef std::pair < xyT, rgbT > pixelT;

typedef cv::Mat imgT;

//F stands for Function
typedef weightT(*linkWeightF) (const pixelT&, const pixelT&, const weightT);
//F stands for Function
typedef weightT(*terminalWeightF) (const pixelT&, const weightT);

// default function that calculates weight between pixel and source
weightT sourceDefWeight(const pixelT& pixel, const weightT lambda);

// default function that calculates weight between pixel and sink
weightT sinkDefWeight(const pixelT& pixel, const weightT lambda);


weightT DefaultSinkWghtFunc(const coordinateT  x, const coordinateT  y, const colourT  r, const colourT  g, const colourT  b, const weightT lambda);
weightT DefaultSourceWghtFunc(const coordinateT  x, const coordinateT  y, const colourT  r, const colourT  g, const colourT  b, const weightT lambda);

// default function that calculates weight between two pixels
weightT linkDefWeight(const pixelT& pixel_I, const pixelT& pixel_II, const weightT sqr_sigmaX2);

Graph imageToGraph(const imgT& image, const vec_pointT& vecObjSeeds,
	const vec_pointT& vecBkgSeeds, vec_pointT& vecNeighbors,
	const weightT lambda = 0.3, const weightT sigma = 10.0,
	terminalWeightF weightSourceF = sourceDefWeight,
	terminalWeightF weightSinkF = sinkDefWeight,
	linkWeightF linkWghtF = linkDefWeight);

#endif//_GRAPH_INITIALIZATION_HPP_