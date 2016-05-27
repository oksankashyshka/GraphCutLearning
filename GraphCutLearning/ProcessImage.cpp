#include <algorithm>
#include <climits>
#include <fstream>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "ProcessImage.hpp"

namespace
{
	const cv::Scalar GREEN_SC { 0, 255, 0 }, // GREEN_COLOR
		BLUE_SC { 255, 0, 0 }; // BLUE_COLOR

	cv::Vec3b GREEN_PIX{ 0, 255, 0 },
		BLUE_PIX{ 255, 0, 0 };

	cv::Scalar OBJECT_COLOR(GREEN_SC),
			   BACKGR_COLOR(BLUE_SC);

	cv::Vec3b OBJECT_COLOR_PIX(GREEN_PIX),
		BACKGR_COLOR_PIX(BLUE_PIX);

	const fs::path path_to_obj  = "D:/education/Diploma/TrainSet/ObjSeed";
	const fs::path path_to_bkg  = "D:/education/Diploma/TrainSet/BkgSeed";
	const fs::path path_to_img  = "D:/education/Diploma/TrainSet/Images/";
	const fs::path path_to_segm = "D:/education/Diploma/TrainSet/Segmented/";

	std::string wName = "Image Window";

	const int RADIUS { 0 },
		LSIZE{ -4 },
		LEN_VEC3B{ 3 };

	cv::Mat image, view;
	
	std::vector<cv::Point> obj_pix, bkg_pix;

	int mouseAct = 0;

	void redraw()
	{	// draw image if there was any event
		static cv::Mat view; // This one I may delete in future
		image.copyTo(view);
		for (auto p : obj_pix)
			cv::circle(view, {p}, RADIUS, OBJECT_COLOR, LSIZE);

		for (auto p : bkg_pix)
			cv::circle(view, {p}, RADIUS, BACKGR_COLOR, LSIZE);

		imshow(wName, view);
	}

	void mouseProc(int event, int x, int y, int flags, void* param)
	{	// processing events of mouse
		switch (event)
		{
		case cv::EVENT_LBUTTONDOWN:
			mouseAct = 1;
			if (x >= 0 && y >= 0 && x < image.cols && y < image.rows)
			{
				obj_pix.emplace_back(x, y);
				redraw();
			}
			break;
		case cv::EVENT_RBUTTONDOWN:
			mouseAct = 2;
			if (x >= 0 && y >= 0 && x < image.cols && y < image.rows)
			{
				bkg_pix.emplace_back(x, y);
				redraw();
			}
			break;

		case cv::EVENT_LBUTTONUP:
		case cv::EVENT_RBUTTONUP:
			mouseAct = 0;
			break;

		case cv::EVENT_MOUSEMOVE:
			if (mouseAct)
				if (x >= 0 && y >= 0 && x < image.cols && y < image.rows)
				{
					if (mouseAct == 1)
						obj_pix.emplace_back(x, y);
					else if (mouseAct == 2)
						bkg_pix.emplace_back(x, y);
					redraw();
				}
			break;
		default:
			break;
		}
	}

	bool imageIsntOpen(const std::string& filename)
	{	// check if image can`t be open
		image = cv::imread(filename);
		return image.empty();
	}

	bool writeVecToTxt(const std::vector<cv::Point>& vec, const fs::path& file_path,
		const std::size_t num_points_to_write = 50)
	{	// write vector to txt file if possible

		std::fstream file(file_path, std::ios::out);

		if (!file.is_open() || vec.empty()) return false;

		auto vec_size = std::min(vec.size(), num_points_to_write); // if we have more elements that we need, don`t use them

		std::copy_n(
			std::cbegin(vec),
			vec_size,
			std::ostream_iterator<cv::Point>(file, "\n"));

		file.close();
		return true;
	}

	bool writeObjVecToJpg(const std::vector<cv::Point>& vec_obj_pix, const fs::path& file_path,
		cv::Vec3b color)
	{	// write vector to jpg file if possible
		// create image with default bkg_color
		image = cv::Mat(image.rows, image.cols, CV_8UC3, BACKGR_COLOR);

		for (auto& pixel : vec_obj_pix)
		{
			image.at<cv::Vec3b>(pixel) = OBJECT_COLOR_PIX;
		}

		cv::imwrite(file_path.string(), image);

		return true;
	}

	int convertVec3bToIntWithRightShift(cv::Vec3b& vec3b_input, const int shift_right)
	{
		int color = 0;

		for (int i = 0; i < LEN_VEC3B; ++i)
		{
			vec3b_input.val[i] >>= shift_right;
			color = color + ((int)vec3b_input.val[i] << ((sizeof(vec3b_input.val[i]) * CHAR_BIT - shift_right) * i));
		}
		return color;
	}

	unsigned long writeTwoIntToLong(const int first, const int second, const int shift)
	{
		long one = (first << shift);
		return ((first << shift) + second);
	}

	template< typename T >
	void  writeColorToMap(const T color, std::map < T, double >& map_to_write)
	{
		auto it_map_to_write = map_to_write.find(color);

		if (it_map_to_write == std::end(map_to_write))
			map_to_write[color] = 1.0;
		else
			map_to_write[color] += 1.0;
	}

	void rotateImage(cv::Mat& mask, int angle = 90, const int scale = 1)
	{
		int width = image.size().width,
			height = image.size().height;
		cv::Mat rot = cv::getRotationMatrix2D(cv::Point2f(0, 0), angle, scale);// / scale; //scale later
		double sinv = rot.at<double>(0, 1),
			cosv = rot.at<double>(0, 0);
		rot.at<double>(1, 2) = width*sinv;  //adjust row offset

		cv::Size dstSize(width*cosv + height*sinv, width*sinv + height*cosv);
	
		cv::Mat dst;

		cv::warpAffine(image, dst, rot, dstSize);
		cv::resize(dst, dst, cv::Size(), scale, scale);  //scale now
		image = dst;

		// now we rotate mask
		width = mask.size().width,
			height = mask.size().height;
		rot = cv::getRotationMatrix2D(cv::Point2f(0, 0), angle, scale);// / scale; //scale later
		sinv = rot.at<double>(0, 1),
			cosv = rot.at<double>(0, 0);
		rot.at<double>(1, 2) = width*sinv;  //adjust row offset

		dstSize = cv::Size(width*cosv + height*sinv, width*sinv + height*cosv);

		cv::warpAffine(mask, dst, rot, dstSize);
		cv::resize(dst, dst, cv::Size(), scale, scale);  //scale now
		mask = dst;
	}

}

void processImage(const std::string& name_file, pair_seeds_T& pair_seeds)
{	// set seeds and write them to files
	if (imageIsntOpen(name_file)) return;

	cv::namedWindow(wName, cv::WINDOW_NORMAL);
	cv::setMouseCallback(wName, mouseProc, 0);

	redraw();
	cv::waitKey(0);

	cv::destroyWindow(wName);

	pair_seeds = std::make_pair(std::move(obj_pix), std::move(bkg_pix));
}

bool writeToTxt(const const pair_seeds_T& pair_seeds, const fs::path& filename,
	const std::size_t num_points_to_write)
{	// record seeds to files
	if (!writeVecToTxt(pair_seeds.first, path_to_obj / filename)) return false;
	if (!writeVecToTxt(pair_seeds.second, path_to_bkg / filename)) return false;
	return true;
}
	
bool writeToJpg(const const pair_seeds_T& pair_seeds, const fs::path& filename)
{

	writeObjVecToJpg(pair_seeds.first, path_to_segm / filename, BACKGR_COLOR_PIX);
	//if (!writeVecToJpg(pair_seeds.first, path_to_obj / filename)) return false;
	//if (!writeVecToJpg(pair_seeds.second, path_to_bkg / filename)) return false;
	return true;
}

void markImages(const std::size_t start_range, const std::size_t end_of_range)
{	// set seeds for images and record them to files
	pair_seeds_T k;
	for (size_t i = start_range; i < end_of_range; ++i)
	{
		k.first.clear();
		k.second.clear();	
		
		processImage(path_to_img.string() + std::to_string(i) + ".jpg", k);
		writeToJpg(k, std::to_string(i) + ".png");
		//writeToTxt(k, std::to_string(i) + ".txt");
	}
}

void setPixProb(pix_color_prob_T& pix_color_prob, const cv::Mat& mask, 
	const int shift_right)
{
	cv::Vec3b pixelRGB;
	uchar pixel_gray = 0;
	const int len_pixelRGB = 3;
	int color = 0;

	// point.x responds for number of number of column, 
	// and point.y responds for number of row.
	// For example numeration of 4 x 3 Mat
	// (0, 0) (1, 0) (2, 0) (3, 0)
	// (0, 1) (1, 1) (2, 1)	(3, 1)
	// (0, 2) (1, 2) (2, 2)	(3, 2)
	for (cv::Point p(0, 0); p.x < image.cols; ++p.x)
	{	// image and mask have the same size
		for (p.y = 0; p.y < image.rows; ++p.y)
		{
			// 1 Look at the color of pixel
			///pixelRGB = image.at<cv::Vec3b>(p);
			pixel_gray = image.at<uchar>(p);
			color = pixel_gray >> shift_right;
			///color = convertVec3bToIntWithRightShift(pixelRGB, shift_right);
			
			// 2 Look at the class of pixel
			pixelRGB = mask.at<cv::Vec3b>(p);
			// 3 Record pixel to right map
			if (pixelRGB == BACKGR_COLOR_PIX) // record color to bkg_pix_map
				writeColorToMap(color, pix_color_prob[0]);
			else // record color to obg_pix_map
				writeColorToMap(color, pix_color_prob[1]);
		}
	}
}

void setVerticalNeighPixProb(neigh_pix_color_prob_T& neigh_pix_color_prob, const cv::Mat& mask, 
	const int shift_right)
{
	cv::Vec3b /*pixelRGB_up, pixelRGB_down, */pixel_class_up, pixel_class_down;
	uchar pixel_gray_up, pixel_gray_down;
	const int len_pixelRGB = 3;
	int color_up = 0, color_down = 0;
	bool first_bit(false), second_bit(false);
	long two_colors = 0L;
	int number_of_array = 0;

	int im_rows_upper_neigh = image.rows - 1; // because last row doesnt have lower neigh

	// we run for each column and take lower neighbor
	for (cv::Point p(0, 0); p.x < image.cols; ++p.x)
	{	// image and mask have the same size
		p.y = 0;
		pixel_gray_down = image.at<uchar>(p);
		///pixelRGB_down = image.at<cv::Vec3b>(p);
		pixel_class_down = mask.at<cv::Vec3b>(p);
		color_down = pixel_gray_down >> shift_right;
		///color_down = convertVec3bToIntWithRightShift(pixelRGB_down, shift_right);
		if (pixel_class_down == OBJECT_COLOR_PIX)
		{
			second_bit = true;
		}

		for (; p.y < im_rows_upper_neigh; ++p.y)
		{ // run down of each row in column

			pixel_gray_up = pixel_gray_down;
			///pixelRGB_up = pixelRGB_down;
			pixel_class_up = pixel_class_down;
			color_up = color_down;
			first_bit = second_bit;
		
			// set lover neighbor
			pixel_gray_down = image.at<uchar>({ p.x, (p.y + 1) });
			///pixelRGB_down = image.at<cv::Vec3b>({p.x, (p.y + 1)});
			pixel_class_down = mask.at<cv::Vec3b>({ p.x, (p.y + 1) });
			color_down = pixel_gray_down >> shift_right;
			//color_down = convertVec3bToIntWithRightShift(pixelRGB_down, shift_right);
			if (pixel_class_down == OBJECT_COLOR_PIX)
				second_bit = true;

			if ((first_bit == second_bit) && (color_up > color_down))
			{	// if we have the same colors, we sort them
				two_colors = writeTwoIntToLong(color_down, color_up,
					sizeof(pixel_gray_down/*pixelRGB_down*/) * ( CHAR_BIT - shift_right));
			}
			else
			{
				two_colors = writeTwoIntToLong(color_up, color_down,
					sizeof(pixel_gray_down/*pixelRGB_down*/) * (CHAR_BIT - shift_right));
			}

			number_of_array = ((int)first_bit << 1) + (int)second_bit;
			
			// 3 Record pixel to right map
			writeColorToMap(two_colors, neigh_pix_color_prob[number_of_array]);
		}
	}
}

void getPrOfLabels(const std::size_t start_range, const std::size_t end_of_range,
	const int shift_per_channel)
{	
	static pix_color_prob_T pix_color_prob;
	static neigh_pix_color_prob_T vertical_neighbor_pr, horizontal_neighbor_pr;

	for (size_t i = start_range; i < end_of_range; ++i)
	{
		static cv::Mat mask;

		image = cv::imread(path_to_img.string() + std::to_string(i) + ".jpg", 0);

//		cv::namedWindow("windoY", cv::WINDOW_NORMAL);

	//	cv::imshow("windoY", image);
		//cv::waitKey(0);

		mask = cv::imread(path_to_segm.string() + std::to_string(i) + ".png");

		setPixProb(pix_color_prob, mask, shift_per_channel);
		setVerticalNeighPixProb(vertical_neighbor_pr, mask, shift_per_channel);
		
		//rotate image to 90grad and use setVerticalNeighPixProb with horizontal map
		rotateImage(mask);
		setVerticalNeighPixProb(horizontal_neighbor_pr, mask, shift_per_channel);
	}
}


/*
bool getInfoFromImage(const std::string& name_file, image_model_T model_image)
{ // give basic info of smth

	/**
	* 1. Open image
	* 2. Open mask image
	* 3. Run through images and set pixel values
	*	3a. Set from loop coordinate of pixel
	*	3b. Set from original image RGB values
	*	3c. Set from mask class of pixel(true responds to object pixel and false - bkg)
	*
	*
	//if (imageIsntOpen(path_to_img.string() + name_file + ".jpg")
	//	|| imageIsntOpen(path_to_segm.string() + name_file + ".png"))
	//	return false;
	static cv::Mat mask;

	image = cv::imread(path_to_img.string() + name_file + ".jpg");
	mask  = cv::imread(path_to_segm.string() + name_file + ".png");

	if (image.empty() || mask.empty()) return false;

	// image and mask have the same size


	}

}*/
