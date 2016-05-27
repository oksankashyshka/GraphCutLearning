#include <cmath>

#include "Graph_Initialization.hpp"

typedef Graph::arraySizeT arraySizeT;

namespace
{
	typedef std::map < int, weightT > seedT;

	static seedT objSeed_map, bkgSeed_map;

	void setMapSeeds(const cv::Mat& image, const vec_pointT& vecSeeds,
		seedT& mapSeeds)
	{
		rgbT pixelRGB;
		int i_Color = 0;
		auto iteratMap = std::begin(mapSeeds);
		//calculate number of similar colours
		for (int i = 0; i < vecSeeds.size(); i++)
		{
			pixelRGB = image.at<rgbT>(vecSeeds[i].x, vecSeeds[i].y);

			i_Color = ((int)pixelRGB.val[2]) << 16;
			i_Color += ((int)pixelRGB.val[1]) << 8;
			i_Color += (int)pixelRGB.val[0];//unique number - its colour

			iteratMap = mapSeeds.find(i_Color);

			if (iteratMap == mapSeeds.end())
				mapSeeds[i_Color] = 1.0;
			else
				mapSeeds[i_Color] += 1.0;
		}

		//calculate - ln Pr()
		for (iteratMap = mapSeeds.begin(); iteratMap != mapSeeds.end(); iteratMap++)
		{
			mapSeeds[iteratMap->first] = -std::log((iteratMap->second) / vecSeeds.size());
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
	int i_Color = pixel.second.val[2] << 16;
	i_Color += pixel.second.val[1] << 8;
	i_Color += (int)pixel.second.val[0];//unique number
	auto iteratMap = objSeed_map.find(i_Color);
	if (iteratMap == std::end(objSeed_map))
		return 0;
	else
		return objSeed_map[i_Color] * lambda;
}

weightT sinkDefWeight(const pixelT& pixel, const weightT lambda)
{
	int i_Color = pixel.second.val[2] << 16;
	i_Color += pixel.second.val[1] << 8;
	i_Color += (int)pixel.second.val[0];//unique number
	auto iterat = bkgSeed_map.find(i_Color);
	if (iterat == std::end(bkgSeed_map))
		return 0;
	else
		return bkgSeed_map[i_Color] * lambda;
}

Graph imageToGraph(const imgT& image, const vec_pointT& objSeeds,
	const vec_pointT& bkgSeeds, vec_pointT& neighborhood,
	const weightT lambda, const weightT sigma, terminalWeightF weightSourceF,
	terminalWeightF weightSinkF, linkWeightF weightLinkF)
{
	int i = 0, j = 0, x = 0, y = 0;

	//delete symetric neighborhoods
	for (i = 0; i < neighborhood.size(); i++)
		for (j = i + 1; j < neighborhood.size(); j++)
		{
			if ((neighborhood[i].y == -neighborhood[j].y) 
				&& (neighborhood[i].x == -neighborhood[j].x))
			{
				neighborhood.erase(neighborhood.begin() + j);
				break;//because we can have only ONE symetric neighbor
			}
		}

	const arraySizeT rowsXcols = image.rows * image.cols;
	const arraySizeT nodenum = rowsXcols + 2;
	const arraySizeT linknum = rowsXcols * (2 + neighborhood.size());//m*n*k/2 + m*n + m*n

	const terminalWeightF funcs_Terminal[2] = { weightSourceF, weightSinkF };

	if (weightSourceF == sourceDefWeight)
		setMapSeeds(image, objSeeds, objSeed_map);

	if (weightSinkF == sinkDefWeight)
		setMapSeeds(image, bkgSeeds, bkgSeed_map);

	rgbT pixelRGB1, pixelRGB2;//for taking RGB values
	coordinateT x2 = 0, y2 = 0;//coordinates for temporary using

	Graph graph(nodenum, linknum);

	//initialize memory for neighbors of vertices
	for (i = 0; i < 2; i++)
	{
		graph[i].nb = new Graph::Nbhd[rowsXcols];
	}

	graph[0].tag = -1;//our sourse
	graph[0].place = 0;
	graph[1].tag = -2;//our sink
	graph[1].place = 1;

	arraySizeT numNbhd = neighborhood.size() * 2 + 2;//number of neighbors per vertice
	for (i = 2; i < nodenum; i++)
	{
		graph[i].nb = new Graph::Nbhd[numNbhd];
		graph[i].place = i;//place in Node array  //graph[i].tag   = -1;//must be i - 2
	}

	//there are THREE types of vertices : Source, Sink and real Vertice
	arraySizeT massCounter[3] = { 0, rowsXcols, rowsXcols * 2 };//our counters for place in linkweigh for all nodes

	//initialize neighbors for terminals
	for (i = 0; i < 2; i++)//FOR EACH TERMINAL
	{
		for (x = 0; x < image.rows; x++) // FOR EACH ROW
		{
			for (y = 0; y < image.cols; y++) // FOR EACH COL
			{

				pixelRGB1 = image.at<rgbT>(x, y);
				numNbhd = x * image.cols + y + 2;

				//int r = (int)pixelRGB1.val[2];
				//int g = (int)pixelRGB1.val[1];
				//int b = (int)pixelRGB1.val[0];
				//
				//cout << "x = " << x << ", y = " << y << ", r = " << r << ", g = " << g << ", b = " << b << ", numNbhd = " << numNbhd << endl;

				//set neighbors: Termainal -> vertice
				graph[i].nb[numNbhd - 2].last = false;
				graph[i].nb[numNbhd - 2].n = &graph[numNbhd];

				graph[i].nb[numNbhd - 2].link = massCounter[i];
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

	for (i = 0; i < neighborhood.size(); i++)
	{
		for (x = 0; x < image.rows; x++) // FOR EACH ROW
		{
			for (y = 0; y < image.cols; y++) // FOR EACH COL
			{
				pixelRGB1 = image.at<rgbT>(x, y);

				x2 = x + neighborhood[i].x;
				y2 = y + neighborhood[i].y;

				if ((y2 >= 0) && (x2 >= 0) && (x2 < image.rows) && (y2 < image.cols))//if point is not valid
				{
					pixelRGB2 = image.at<rgbT>(x2, y2);

					tmpPlace = x  * image.cols + y + 2;//number of our vertice(x, y) in node arr
					numNbhd = x2 * image.cols + y2 + 2;//number of our neighbor vertice(x2, y2) in node arr

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


	for (i = 2; i < nodenum; i++)
	{
		for (j = 2; j <= graph[i].tag; j++)
			tmp_weight += graph(graph[i].nb[j].link);

		if (tmp_weight > K)
		{
			K = tmp_weight;
		}
		tmp_weight = 0.0;
	}

	K += 1.0;

	//set weight of objSeed to Terminals
	for (i = 0; i < objSeeds.size(); i++)
	{
		numNbhd = objSeeds[i].x * image.cols + objSeeds[i].y;//number in linkweight
		graph(numNbhd) = K;
		graph(numNbhd + rowsXcols) = 0;
	}

	//set weight of bkgSeed to Terminals
	for (i = 0; i < bkgSeeds.size(); i++)
	{
		numNbhd = bkgSeeds[i].x * image.cols + bkgSeeds[i].y;//number in linkweight
		graph(numNbhd) = 0;
		graph(numNbhd + rowsXcols) = K;
	}

	for (i = 2; i < nodenum; i++)
		graph[i].tag = graph[i].place - 2;

	return graph;
}
