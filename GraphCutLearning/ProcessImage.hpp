//	
#ifndef _PROCESS_IMAGE_HPP_
#define _PROCESS_IMAGE_HPP_

#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>

#include <array>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::tr2::sys;

// pair.first is obj_seed and pair.second is bkg_seeds
typedef std::pair<std::vector<cv::Point>, std::vector<cv::Point> > pair_seeds_T;
/*
typedef bool pix_class_T;
typedef cv::Vec3b pix_color_T;
typedef cv::Point pix_coord_T;
typedef std::tuple< pix_color_T, pix_coord_T, pix_class_T > pixel_model_T;
typedef std::vector<pixel_model_T> image_model_T;
*/

//typedef std::array<std::vector<double>, 4 > neigh_table_T;

// first map responds for Pr of bkg pixels, and second one - of obj pixels
typedef std::array< std::map < int, double >, 2 > pix_color_prob_T;

// 00 map -> (first neigh bkg, second - bkg); 01 map -> (first neigh bkg, second - obj)
// 10 map -> (first neigh obj, second - bkg); 11 map -> (first neigh obj, second - obj)
typedef std::array< std::map < long, double >, 4 > neigh_pix_color_prob_T;

// set seeds by mouse and write them to pair_seeds
void processImage(const std::string& name_file, pair_seeds_T& pair_seeds);

void markImages(const std::size_t start_range, const std::size_t end_of_range);

// write seeds to txt file if possible, if not - return false
bool writeToTxt(const pair_seeds_T& pair_seeds, const fs::path& filename,
	const std::size_t num_points_to_write = 100);

// write seeds to jpg file if possible, if not - return false
bool writeToJpg(const const pair_seeds_T& pair_seeds, const fs::path& filename);

//bool getInfoFromImage(const std::string& name_file, image_model_T model_image);

void setVerticalNeighPixProb(neigh_pix_color_prob_T& neigh_pix_color_prob,
	const cv::Mat& mask);
void setPixProb(pix_color_prob_T& pix_color_prob, const cv::Mat& mask);

void getPrOfLabels(const std::size_t start_range, const std::size_t end_of_range,
	const int shift_per_channel);


#endif //_PROCESS_IMAGE_HPP_
