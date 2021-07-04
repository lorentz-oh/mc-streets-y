#pragma once

#include "mapnik/image.hpp"
#include "block.h"
#include "schematic.h"
#include "const.h"
#include <memory>
#include <vector>
#include <limits.h>
#include <boost/property_tree/ptree.hpp>

//Колонна имеет разметку, если интенсивность красного цвета в пикселе больше значения COLOR_THRESHOLD, которое равно 128
//Как говорил один мудрый человек - выбирай среднее
inline bool isMarked(unsigned int Pixel)
{
	return *(reinterpret_cast<u_char*>(&Pixel) + 1) > COLOR_THRESHOLD;
}

//A wrapper around mapnik's image
class RegMap
{
public:
	int m_Margin = 0;
	mapnik::image_rgba8 m_Img;
	
	RegMap():
		m_Margin{Margin},
		m_Img{Margin * 2 + RegSide, Margin * 2 + RegSide}
	{}

	RegMap(int imgMargin, int Side):
		m_Margin{imgMargin},
		m_Img{imgMargin * 2 + Side, imgMargin * 2 + Side}
	{}

	RegMap(RegMap && That):
		m_Margin{That.m_Margin},
		m_Img{std::move(That.m_Img)}	
	{}

	RegMap(const RegMap& That)
	{
		m_Margin = That.m_Margin;
		m_Img = That.m_Img;
	}

	bool isEmpty();

	inline unsigned int& operator() (std::size_t i, std::size_t j){
		return m_Img(i + m_Margin, j + m_Margin);  }
    
	inline unsigned int const& operator() (std::size_t i, std::size_t j) const{
		return m_Img(i + m_Margin, j + m_Margin);  }

	inline bool IsDyed(std::size_t i, std::size_t j){
		auto Pixel = m_Img(i + m_Margin, j + m_Margin);
		return isMarked(Pixel);  }
		
	inline void Mark(std::size_t i, std::size_t j){
		m_Img(i + m_Margin, j + m_Margin) = UINT_MAX;//пиксель мапника это просто unsigned int
	}
	
	inline void Unmark(std::size_t i, std::size_t j){
		m_Img(i + m_Margin, j + m_Margin) = 0;
	}
};

enum Range
{
	OFF_ABSOLUTE,
	OFF_RELATIVE,
	SMOOTH_DATA,
	SMOOTH_NONE
};

/*This class reprepresents a collumn of blocks*/
class BlockRange
{
public:
	char Id = 0;
	char Add = 0;
	char Data = 0;
	int From = 0;
	int To = 0;
	int Mode = OFF_RELATIVE;
	int Smooth = SMOOTH_NONE;

	void ParseRange(boost::property_tree::ptree & pTree);
};

class Layer
{
public:
	std::string m_Name;

	std::vector<BlockRange> m_Ranges;
	std::vector<Schematic> m_Structs;
	void ParseLayer(boost::property_tree::ptree & pTree);
};

class Scheme
{
public:
	Layer m_Surface;
	std::vector<Layer> m_Layers;
	void ParseScheme(std::string SchemeFile);
};

extern Scheme g_Scheme;
