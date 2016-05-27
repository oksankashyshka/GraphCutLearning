#include <cmath>

#include "Graph_Initialization.hpp"

typedef Graph::arraySizeT arraySizeT;

namespace
{
	typedef std::map < int, weightT > seedT;
	seedT obj_seed_map, bkg_seed_map;

	template <typename T>
	inline T delta(const T I, const T II)
	{ // return difference between I and II
		return (I - II);
	}

	inline int getColorValue(const rgbT& rgb)
	{ // calculate color value using rgb values
		long long color_val = 0;
		short shift = 0;
		int bits_in_byte = 8;

		for (std::size_t i = 0; i < rgb.rows; ++i)
		{
			shift = i * bits_in_byte; // each component of colour has 8 bit
			color_val += ((int)rgb.val[i]) << shift;
		}
		return color_val;
	}

	void setMapSeeds(const cv::Mat& image, const vec_pointT& vec_seeds,
		seedT& mapSeeds)
	{
		rgbT pixelRGB;
		int color_val = 0;
		auto iteratMap = std::begin(mapSeeds);
		//calculate number of similar colours
		for (const auto& seed : vec_seeds)
		{
			pixelRGB = image.at<rgbT>(seed.x, seed.y);
			color_val = getColorValue(pixelRGB);

			iteratMap = mapSeeds.find(color_val);

			if (iteratMap == mapSeeds.end())
				mapSeeds[color_val] = 1.0;
			else
				mapSeeds[color_val] += 1.0;
		}

		//calculate - ln Pr()
		auto vec_seeds_size = vec_seeds.size();
		for (auto& seed_num : mapSeeds)
		{
			mapSeeds[seed_num.first] = -std::log((mapSeeds[seed_num.second]) / vec_seeds_size);
		}
	}

}

weightT linkDefWeight(const pixelT& pixel_I, const pixelT& pixel_II, const weightT sqr_sigmaX2)
{
	weightT deltaX = (weightT)pixel_I.first.x - (weightT)pixel_II.first.x;
	weightT deltaY = (weightT)pixel_I.first.y - (weightT)pixel_II.first.y;
	weightT deltaB = (weightT)pixel_I.second.val[0] - (weightT)pixel_II.second.val[0];
	weightT deltaG = (weightT)pixel_I.second.val[1] - (weightT)pixel_II.second.val[1];
	weightT deltaR = (weightT)pixel_I.second.val[2] - (weightT)pixel_II.second.val[2];

	weightT distRGB = deltaR * deltaR + deltaB * deltaB + deltaG * deltaG;
	weightT distXY = deltaX * deltaX + deltaY * deltaY;
	
	return (weightT)((std::exp(-distRGB / sqr_sigmaX2)) / (std::sqrtf(distRGB + distXY)));
}

weightT sourceDefWeight(const pixelT& pixel, const weightT lambda)
{
	int color_val = getColorValue(pixel.second);

	auto iteratMap = obj_seed_map.find(color_val);
	if (iteratMap == std::end(obj_seed_map))
		return 0;
	else
		return obj_seed_map[color_val] * lambda;
}

weightT sinkDefWeight(const pixelT& pixel, const weightT lambda)
{
	int color_val = getColorValue(pixel.second);
	auto iterat = bkg_seed_map.find(color_val);
	if (iterat == std::end(bkg_seed_map))
		return 0;
	else
		return bkg_seed_map[color_val] * lambda;
}

Graph imageToGraph(const imgT& image, const vec_pointT& objSeeds,
	const vec_pointT& bkgSeeds, vec_pointT& neighborhood,
	const weightT lambda, const weightT sigma, terminalWeightF weightSourceF,
	terminalWeightF weightSinkF, linkWeightF weightLinkF)
{
	//delete symetric neighborhoods
	for (int i = 0; i < neighborhood.size(); ++i)
		for (int j = i + 1; j < neighborhood.size(); ++j)
		{
			if ((neighborhood[i].y == -neighborhood[j].y) 
				&& (neighborhood[i].x == -neighborhood[j].x))
			{
				neighborhood.erase(neighborhood.begin() + j);
				break;//because we can have only ONE symetric neighbor
			}
		}

	const arraySizeT rowsXcols = image.rows * image.cols;
	const arraySizeT num_terminals = 2;
	const arraySizeT nodenum = rowsXcols + num_terminals;
	const arraySizeT linknum = rowsXcols * (num_terminals + neighborhood.size());//m*n*k/2 + m*n + m*n

	const terminalWeightF funcs_Terminal[num_terminals] = { weightSourceF, weightSinkF };

	if (weightSourceF == sourceDefWeight)
		setMapSeeds(image, objSeeds, obj_seed_map);

	if (weightSinkF == sinkDefWeight)
		setMapSeeds(image, bkgSeeds, bkg_seed_map);

	rgbT pixelRGB1, pixelRGB2;//for taking RGB values
	coordinateT x2 = 0, y2 = 0;//coordinates for temporary using

	Graph graph(nodenum, linknum);

	//initialize memory for neighbors of vertices
	for (int i = 0; i < num_terminals; ++i)
	{
		graph[i].nb = new Graph::Nbhd[rowsXcols];
	}

	graph[0].tag = -1;//our sourse
	graph[0].place = 0;
	graph[1].tag = -2;//our sink
	graph[1].place = 1;

	arraySizeT numNbhd = neighborhood.size() * 2 + 2;//number of neighbors per vertice
	for (int i = num_terminals; i < nodenum; ++i)
	{
		graph[i].nb = new Graph::Nbhd[numNbhd];
		graph[i].place = i;//place in Node array  //graph[i].tag   = -1;//must be i - 2
	}

	//there are THREE types of vertices : Source, Sink and real Vertice
	arraySizeT massCounter[3] = { 0, rowsXcols, rowsXcols * 2 };//our counters for place in linkweigh for all nodes

	//initialize neighbors for terminals
	for (int i = 0; i < num_terminals; ++i)//FOR EACH TERMINAL
	{
		for (int x = 0; x < image.rows; ++x) // FOR EACH ROW
		{
			for (int y = 0; y < image.cols; ++y) // FOR EACH COL
			{

				pixelRGB1 = image.at<rgbT>(x, y);
				numNbhd = x * image.cols + y + num_terminals;

				//set neighbors: Termainal -> vertice
				graph[i].nb[numNbhd - num_terminals].last = false;
				graph[i].nb[numNbhd - num_terminals].n = &graph[numNbhd];

				graph[i].nb[numNbhd - num_terminals].link = massCounter[i];
				graph(massCounter[i]) = funcs_Terminal[i]({ myPoint(x, y), pixelRGB1}, lambda);
				//set neighbors: Vertice   -> terminal

				graph[numNbhd].nb[i].last = (bool)i;
				graph[numNbhd].nb[i].n = &graph[i];
				graph[numNbhd].nb[i].link = massCounter[i];
				massCounter[i]++;
				graph[numNbhd].tag = i;
			}
		}
	}


	graph[0].nb[rowsXcols - 1].last = true;
	graph[1].nb[rowsXcols - 1].last = true;

	//set neighbors for real nodes
	arraySizeT tmpPlace = 0;

	weightT sqr_sigmaX2 = sigma * sigma * 2;

	for (int i = 0; i < neighborhood.size(); ++i)
	{
		for (int x = 0; x < image.rows; ++x) // FOR EACH ROW
		{
			for (int y = 0; y < image.cols; ++y) // FOR EACH COL
			{
				pixelRGB1 = image.at<rgbT>(x, y);

				x2 = x + neighborhood[i].x;
				y2 = y + neighborhood[i].y;

				if ((y2 >= 0) && (x2 >= 0) && (x2 < image.rows) && (y2 < image.cols))//if point is not valid
				{
					pixelRGB2 = image.at<rgbT>(x2, y2);

					tmpPlace = x  * image.cols + y + num_terminals;//number of our vertice(x, y) in node arr
					numNbhd = x2 * image.cols + y2 + num_terminals;//number of our neighbor vertice(x2, y2) in node arr

					graph[tmpPlace].nb[graph[tmpPlace].tag].last = false;//now our neighbor is not last
					graph[tmpPlace].tag++;//number of Last neighbor

					graph[tmpPlace].nb[graph[tmpPlace].tag].last = true;//it is our last neighbor
					graph[tmpPlace].nb[graph[tmpPlace].tag].n = &graph[numNbhd];
					graph[tmpPlace].nb[graph[tmpPlace].tag].link = massCounter[2];

					graph(massCounter[2]) = weightLinkF({ myPoint(y, x), pixelRGB1 }, 
						{ myPoint(y2, x2), pixelRGB2 }, sqr_sigmaX2);

					//our vertice(x, y) is also neighbor to its neighbor vertice(x2, y2)
					graph[numNbhd].nb[graph[numNbhd].tag].last = false;
					graph[numNbhd].tag++;//number of Last neighbor

					graph[numNbhd].nb[graph[numNbhd].tag].last = true;
					graph[numNbhd].nb[graph[numNbhd].tag].n = &graph[tmpPlace];//our vertice is
					graph[numNbhd].nb[graph[numNbhd].tag].link = massCounter[2];
					massCounter[2]++;
				}
			}
		}
	}

	//Here we count K
	weightT K = 0.0;
	weightT tmp_weight = 0.0;


	for (int i = num_terminals; i < nodenum; ++i)
	{
		for (int j = num_terminals; j <= graph[i].tag; ++j)
			tmp_weight += graph(graph[i].nb[j].link);

		if (tmp_weight > K)
		{
			K = tmp_weight;
		}
		tmp_weight = 0.0;
	}

	K += 1.0;

	//set weight of objSeed to Terminals
	for (int i = 0; i < objSeeds.size(); ++i)
	{
		numNbhd = objSeeds[i].x * image.cols + objSeeds[i].y;// +num_terminals;//number in linkweight
		graph(numNbhd) = K;
		graph(numNbhd + rowsXcols) = 0;
	}

	//set weight of bkgSeed to Terminals
	for (int i = 0; i < bkgSeeds.size(); ++i)
	{
		numNbhd = bkgSeeds[i].x * image.cols + bkgSeeds[i].y;// +num_terminals;//number in linkweight
		graph(numNbhd) = 0;
		graph(numNbhd + rowsXcols) = K;
	}

	for (int i = num_terminals; i < nodenum; ++i)
		graph[i].tag = graph[i].place - num_terminals;

	return graph;
}
