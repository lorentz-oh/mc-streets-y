#pragma once
#include <mapnik/image.hpp>
#include <bitset>
#include <libnoise/noise.h>
#include "elevation.h"
#include "region.h"
#include "schematic.h"
#include "const.h"
#include "layer.h"

using mapnik::image_rgba8;

class RegionGenerator
{
public:
    RegionGenerator(int RegX, int RegZ, bool Overwrite);
    void Generate();
	void GenerateRange(BlockRange & Range, RegMap & RegionMap, bool Everywhere);
	void GenerateLayer(Layer & Layer, RegMap & RegionMap, bool Everywhere);
	void GenerateStructures(const Layer & Layer);
	//clears blocks within extent map
	void ClearExtent();
	//Снимает разметку с тех колонн, в которых есть блоки,
	//это нужно для параметра append из файла настроек
	void UnmarkExtent();

	ElevMap m_Elev;
	std::unordered_map<std::string, RegMap> m_Layers;
private:
	bool m_Overwrite = false;
    int RegX, RegZ;
    Region Reg;
};

void PrepElevation(image_rgba8& Elev);

extern noise::module::Perlin PerlinModule;

