#include <iostream>

#include <opencv2/highgui/highgui.hpp>
#include <string>

#include "ProcessImage.hpp"

#include "Proftimer.hpp"

#include "Graph_Initialization.hpp"
#include "Graphcut.hpp"

using namespace std;
using namespace cv;

typedef Graph::tagT tagT;
typedef Graph::weightT weightT;
typedef Graph::arraySizeT arraySizeT;
typedef Graph::Nbhd Nbhd;

const cv::Scalar RED = Scalar(0, 0, 255),
GREEN = cv::Scalar(0, 255, 0),
BLUE  = cv::Scalar(255, 0, 0),
BLACK = cv::Scalar(0, 0, 0),
WHITE = cv::Scalar(255, 255, 255);
const int RADIUS = 2, LSIZE = -1;

const std::string wName = "image";
HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
cv::Mat image, view;
//std::vector<cv::Point> objPIX, bkgPIX;
std::vector<myPoint> objPx, bkgPx;
int mouseAct = 0;

enum ConsoleColor
{
	Black = 0,
	Blue = 1,
	Green = 2,
	Cyan = 3,
	Red = 4,
	Magenta = 5,
	Brown = 6,
	LightGray = 7,
	DarkGray = 8,
	LightBlue = 9,
	LightGreen = 10,
	LightCyan = 11,
	LightRed = 12,
	LightMagenta = 13,
	Yellow = 14,
	White = 15
};

void SetColor(ConsoleColor text, ConsoleColor background)
{
	SetConsoleTextAttribute(hStdOut, (WORD)((background << 4) | text));
}

void redraw()
{
	static Mat view;
	image.copyTo(view);
	for (auto p : objPx)
		circle(view, cv::Point(p.y, p.x), RADIUS, GREEN, LSIZE);
	for (auto p : bkgPx)
		circle(view, cv::Point(p.y, p.x), RADIUS, BLUE, LSIZE);

	imshow(wName, view);
}

void mouseProc(int event, int x, int y, int flags, void* param)
{
	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN:
		mouseAct = 1;
		if (x >= 0 && y >= 0 && x < image.cols && y < image.rows)
		{
			objPx.emplace_back(myPoint(x, y));
			redraw();
		}
		break;
	case CV_EVENT_RBUTTONDOWN:
		mouseAct = 2;
		if (x >= 0 && y >= 0 && x < image.cols && y < image.rows)
		{
			bkgPx.emplace_back(myPoint(x, y));
			redraw();
		}
		break;
	case CV_EVENT_LBUTTONUP:
	case CV_EVENT_RBUTTONUP:
		mouseAct = 0;
		break;
	case CV_EVENT_MOUSEMOVE:
		if (mouseAct)
			if (x >= 0 && y >= 0 && x < image.cols && y < image.rows)
			{
				if (mouseAct == 1)
					objPx.emplace_back(myPoint(x, y));
				else if (mouseAct == 2)
					bkgPx.emplace_back(myPoint(x, y));
				redraw();
			}
		break;
	default:
		break;
	}
}

Mat toMat(const tagT * const p, const tagT tag)
{
	Mat res = Mat(image.size(), CV_8U);
	for (int j = 0; j < res.rows; ++j)
	{
		for (int i = 0; i < res.cols; ++i)
		{
			res.at<bool>(j, i) = (p[j * res.cols + i + 2] == tag) ? 1 : 0;
		}
	}
	return res;
}

int main()
{
	//std::size_t start{ 0 }, end{ 1 };
	//const int shift_per_channel = 6;
	//	markImages(start, end);
	//getPrOfLabels(start, end, shift_per_channel);
	//std::cout << "All Good";

	std::string filename{ "50x28.jpg" };

	image = imread(filename);
	if (image.empty())
	{
		cout << "\n Durn, couldn't read image filename " << filename << endl;
		return 1;
	}
	namedWindow(wName, WINDOW_NORMAL);
	setMouseCallback(wName, mouseProc, 0);

	redraw();
	waitKey(0);
	std::vector<myPoint> vecNeighborsP;
//	std::vector<cv::Point> vecNeighborsP;
	vecNeighborsP.emplace_back(1, 0);
	vecNeighborsP.emplace_back(0, 1);

	setMouseCallback(wName, mouseProc, 0);

	for (int i = 0; i < objPx.size(); i++)
		for (int j = i + 1; j < objPx.size(); j++)
		{ // stupid delete for equal pixels
			while ((j < objPx.size()) && (objPx[i].y == objPx[j].y) && (objPx[i].x == objPx[j].x))
				objPx.erase(objPx.begin() + j);
		}

	for (int i = 0; i < bkgPx.size(); i++)
		for (int j = i + 1; j < bkgPx.size(); j++)
		{ // another stupid delete for equal pixels
			while ((j < bkgPx.size()) && (bkgPx[i].y == bkgPx[j].y) && (bkgPx[i].x == bkgPx[j].x))
				bkgPx.erase(bkgPx.begin() + j);
		}

	int i = 0;
	std::cout << "ObjPicselsP\n";
	for (auto pix : objPx)
	{
		std::cout << i << ". x =  " << pix.x << ", y = " << pix.y << std::endl;
		++i;
	}

	i = 0;
	std::cout << "\nBkgPicsels\n";
	for (auto pix : bkgPx)
	{
		std::cout << i << ". x =  " << pix.x << ", y = " << pix.y << std::endl;
		++i;
	}

	cout << "Graph creation...\n";
	ProfTimer t1;
	t1.start();
	Graph gr = imageToGraph(image, objPx, bkgPx, vecNeighborsP, 0.3, 10.0);
	t1.check();
	double time_create_gr = t1.getDur();

	std::cout << std::endl << "\ndone\n";
	ProfTimer t2;
	t2.start();
	GraphCut grc(gr);
	t2.check();
	double time_create_gr_cut = t2.getDur();

	ProfTimer t;

	cout << "mincut started...\n";
	t.start();
	grc.mincut();
	t.check();
	double time_mincut = t.getDur();
	cout << "done\n";

	cout << "time creating grph:   " << time_create_gr << "\n";
	cout << "time create gr cut:   " << time_create_gr_cut << "\n";
	cout << "time		 mincut:   " << time_create_gr_cut << "\n";
	double time_total = time_create_gr + time_create_gr_cut + time_mincut;
	cout << "TOTAL time =   " << time_total << std::endl;

	Mat obj, bgd;
	Mat mask = toMat(grc.marks(), -1);
	image.copyTo(obj, mask);
	mask = toMat(grc.marks(), -2);
	image.copyTo(bgd, mask);
	namedWindow("obj", WINDOW_NORMAL);
	namedWindow("bgd", WINDOW_NORMAL);
	imshow("obj", obj);
	imshow("bgd", bgd);

	waitKey(0);
	destroyWindow(wName);
	destroyWindow("obj");
	destroyWindow("bgd");
	destroyAllWindows();
	std::cin.get();

	//cv::Scalar WHITE{ 255, 255, 255 }, BLACK{ 0, 0, 0 };
	
	// point.x responds for number of number of column, 
	// and point.y responds for number of row.
	// For example numeration of 4 x 3 Mat
	// (0, 0) (1, 0) (2, 0) (3, 0)
	// (0, 1) (1, 1) (2, 1)	(3, 1)
	// (0, 2) (1, 2) (2, 2)	(3, 2)
	/*for (cv::Point p(0, 0); p.x < im_cols; ++p.x)
	{
		for (p.y = 0; p.y < im_rows - 1; ++p.y)
			cloyn1.at<cv::Vec3b>(p) = vec34;
	}*/
	
}
