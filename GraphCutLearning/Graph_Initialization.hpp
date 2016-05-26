#ifndef _GRAPH__INITIALIZATION_HPP_
#define _GRAPH__INITIALIZATION_HPP_

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

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
typedef const weightT(*linkWeightFunc) (const coordinateT, const coordinateT,
	const colourT, const colourT, const colourT, const coordinateT, const coordinateT,
	const colourT, const colourT, const colourT, const weightT); // x1, y1, RGB1, x2, y2, RGB2, SQR_SIGMAx2
typedef const weightT(*terminalWeightFunc)  (const coordinateT, const coordinateT,
	const colourT, const colourT, const colourT, const weightT/*, vector<myPoint> &*/); // x, y, RGB, LAMBDA

const weightT DefaultSinkWghtFunc(const coordinateT  x, const coordinateT  y, const colourT  r, const colourT  g, const colourT  b, const weightT lambda);
const weightT DefaultSourceWghtFunc(const coordinateT  x, const coordinateT  y, const colourT  r, const colourT  g, const colourT  b, const weightT lambda);
const weightT DefaultLinkWghtFunc(const coordinateT x1, const coordinateT y1,
	const colourT r1, const colourT g1, const colourT b1, 
	const coordinateT x2, const coordinateT y2,
	const colourT r2, const colourT g2, const colourT b2, const weightT sqr_sigmaX2);

Graph imageToGraph(const cv::Mat& image, const std::vector<myPoint>& vecObjSeeds,
	const std::vector<myPoint>& vecBkgSeeds, std::vector<myPoint>& vecNeighbors,
	const weightT lambda = 0.7, const weightT sigma = 100.0, 
	terminalWeightFunc = DefaultSourceWghtFunc,	terminalWeightFunc = DefaultSinkWghtFunc, 
	linkWeightFunc = DefaultLinkWghtFunc);

#endif//_GRAPH_INITIALIZATION_HPP_