#include <iostream>

#include "ProcessImage.hpp"

#include <opencv2/highgui/highgui.hpp>
#include <string>


#include "Proftimer.hpp"

#include "Graph_Initialization.hpp"
#include "Graphcut.hpp"

using namespace std;
using namespace cv;

typedef Graph::tagT tagT;
typedef Graph::weightT weightT;
typedef Graph::arraySizeT arraySizeT;
typedef Graph::Nbhd Nbhd;

const Scalar RED = Scalar(0, 0, 255),
GREEN = Scalar(0, 255, 0),
BLUE = Scalar(255, 0, 0),
BLACK = Scalar(0, 0, 0),
WHITE = Scalar(255, 255, 255);
const int RADIUS = 3, LSIZE = -1;

const string wName = "image";
HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
Mat image, view;
std::vector<cv::Point> objPIX, bkgPIX;
vector<myPoint> objPx, bkgPx;
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
	for (auto p : objPIX)
		circle(view, p, RADIUS, GREEN, LSIZE);
	for (auto p : bkgPIX)
		circle(view, p, RADIUS, BLUE, LSIZE);

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
			objPIX.emplace_back(x, y);
			redraw();
		}
		break;
	case CV_EVENT_RBUTTONDOWN:
		mouseAct = 2;
		if (x >= 0 && y >= 0 && x < image.cols && y < image.rows)
		{
			bkgPIX.emplace_back(x, y);
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
					objPIX.emplace_back(x, y);
				else if (mouseAct == 2)
					objPIX.emplace_back(x, y);
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
	std::vector<myPoint> vecNeighbors;
	std::vector<cv::Point> vecNeighborsP;
	vecNeighbors.emplace_back(1, 0);
	vecNeighbors.emplace_back(0, 1);

	setMouseCallback(wName, mouseProc, 0);

	for (int i = 0; i < objPx.size(); i++)
		for (int j = i + 1; j < objPx.size(); j++)
		{ // stupid delete for equal pixels
			if ((objPx[i].y == objPx[j].y) && (objPx[i].x == objPx[j].x))
				objPx.erase(objPx.begin() + j);
		}

	for (int i = 0; i < bkgPx.size(); i++)
		for (int j = i + 1; j < bkgPx.size(); j++)
		{ // another stupid delete for equal pixels
			if ((bkgPx[i].y == bkgPx[j].y) && (bkgPx[i].x == bkgPx[j].x))
				bkgPx.erase(bkgPx.begin() + j);
		}

	std::cout << "ObjPicselsP\n";
	for (auto pix : objPIX)
	{
		std::cout << pix << std::endl;
	}

	std::cout << "\nBkgPicsels\n";
	for (auto pix : bkgPIX)
	{
		std::cout << pix << std::endl;
	}

	cout << "Graph creation...\n";
	ProfTimer t1;
	t1.start();
	Graph gr = ImageToGraph(image, objPx, bkgPx, vecNeighbors, 0.3, 10);
	t1.check();
	double time_create_gr = t1.getDur();

	cout << endl << "\ndone\n";
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
	//cv::Vec3b vec3 = { 100, 50, 155 };
	//cv::Vec3b vec34 = { 100, 50, 200 };
	//
	//int im_cols = 10, im_rows = 5;
	//
	//cv::Mat mask(im_rows, im_cols, CV_8UC3, cv::Scalar(0)); // create black rectangle
	//
	//const std::string w_name = "image.png";
	//const std::string jpg_name = "image.jpg";
	//const std::string w_name1 = "TEST";
	/*
	cv::Mat im = cv::imread(jpg_name, 0);

	cv::namedWindow(w_name, cv::WINDOW_NORMAL);
	cv::namedWindow(w_name1, cv::WINDOW_NORMAL);
	


	cv::imshow(w_name, im);
	cv::waitKey(0);

	for (cv::Point p(0, 0); p.x < im.cols - 250; ++p.x)
	{
		for (p.y = 50; p.y < im.rows - 350; ++p.y)
			im.at<uchar>(p) = 240;
	}

	cv::imshow(w_name, im);
	cv::waitKey(0);

	cv::imwrite(w_name, mask);
	
	cv::Mat cloyn{ cv::imread(w_name) };
	


	cv::imshow(w_name, cloyn);
	cv::waitKey(0);
	
	cloyn.at<cv::Vec3b>(cv::Point(0, 0)) = vec3;
	
	cv::imshow(w_name, cloyn);
	cv::waitKey(0);
	
	cv::Mat cloyn1 = cloyn;
	cv::imshow(w_name1, cloyn1);
	cv::waitKey(0);
	*/
	
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
	}
	
	//std::cout << "   0  = " <<  (int)vec34.val[0] << "  1  = " << (int)vec34.val[1] << "  2 =  " << (int)vec34.val[2] << std::endl;
	//
	//vec34.val[0] >>=  4;
	//vec34.val[1] >>=  4;
	//vec34.val[2] >>=  4;
	//
	////vec34 = vec34 >> 4;
	//
	//std::cout << "   0  = " << (int)vec34.val[0] << "  1  = " << (int)vec34.val[1] << "  2 =  " << (int)vec34.val[2] << std::endl;
	//
	cv::imshow(w_name1, cloyn1);
	cv::waitKey(0);
	//
	////cloyn.at<cv::Vec3b>(cv::Point(0, 0)).val[1] = vec3[1];
	////cloyn.at<cv::Vec3b>(cv::Point(0, 0)).val[2] = vec3[2];
	//cv::imwrite(w_name, cloyn);
	//
	////cloyn.at<cv::Vec3b>(11, 43) = vec3;
	////cloyn.at<cv::Vec3b>(12, 42) = vec3;
	////cloyn.at<cv::Vec3b>(11, 42) = vec3;
	//
	//cloyn = WHITE;
	//
	//cv::imshow(w_name, cloyn);
	//
	//cv::waitKey(0);
	//
	cv::destroyWindow(w_name);
	//
	//
	//cv::imwrite("image2.png", cloyn);
	*/
	
}
