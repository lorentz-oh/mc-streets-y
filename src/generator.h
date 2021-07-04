#pragma once
#include <mapnik/box2d.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/map.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/image.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/datasource_cache.hpp>
#include <const.h>
#include "region_generator.h"

template<class T>
T CeilChunk(T Num, T Chunk)
{
	T ChunkCount = std::ceil(Num / Chunk);
	return ChunkCount * Chunk;
}

template<class T>
T FloorChunk(T Num, T Chunk)
{
	T ChunkCount = std::floor(Num / Chunk);
	return ChunkCount * Chunk;
}

// Эта функция выравнивает box2d по границам регионов
void AlignToRegionBorders(mapnik::box2d<double> & Box);

void SortTwo(int& One, int& Two);

int DivFloor(int N, int Divisor);

//Этот активирует слой с названием LayerName, деактивируя остальные.
//После этого остальные слои не будут отрисовываться.
void SetLayer(mapnik::Map & Map, std::string LayerName);

//Этот активирует или деактивирует слой с названием LayerName, в этот раз НЕ деактивируя остальные.
void ToggleLayer(mapnik::Map & Map, std::string LayerName, bool State);

void ToggleAllLayers(mapnik::Map & Map, bool State);

//эта функция возвращает слой
//Map - карта, слой из которой нас интересует
//LayerName - название слоя
mapnik::layer& GetLayer(mapnik::Map & Map, const std::string LayerName);


class generator
{
public:
    //Minecraft center, scale
    generator(mapnik::coord2d MCC, double Scale);
	
	//Этот метод генерирует регионы (файлы с расширением .mca в сохранении кубача)
	//Подробности в классе RegionGenerator
    void Generate();
    
	void DebugGenerate();
private:
	//Этот метод устанавливает задание для генератора - координаты регионов,
	//попадающих в слой Extent засовываются в вектор Task класса Generator 
	void SetTaskViaExtent();
    
	//not YR, but ZR because in minecraft Y is height
	//Этот метод отрисовывает слой
    void RenderReg(int XR, int ZR, RegionGenerator& Reg);

	//Этот метод перемещает карту m_Map к определенному региону
    void PanToReg(int XR,int ZR);
	
	/*Члены класса*/
   	//Box, describing region 0,0
    mapnik::box2d<double> RefRegBox;
   
	mapnik::coord2d m_MCC;

	//Сама карта собственной персоной
	mapnik::Map m_Map{RegSideI, RegSideI};

	double m_Scale;	
	//A vector, listing regions to generate
    std::vector<mapnik::coord2i> Task;
};

