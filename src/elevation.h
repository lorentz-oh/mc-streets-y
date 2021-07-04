#pragma once

#include <array>
#include <mapnik/coord.hpp>
#include <gdal/gdalwarper.h>
#include <gdal/gdal_priv.h>
#include "layer.h"
#include <const.h>

using mapnik::coord2d;

class ElevationWarper;
class ElevationDataset;

//класс, представляющий карту высот для региона
class ElevMap
{
public:
	ElevMap(int RegX, int RegZ);

	ElevMap(){}

	void Render(int RegX, int RegZ);
	
	inline int& operator()(int X, int Y)
	{ return m_ElevData[ (X + Margin) + ((Y + Margin) * XRes)]; }
  	
	static const int XRes = ImgSide;
	static const int YRes = ImgSide;
	
	static ElevationWarper m_Warper;
private:
	std::array<int, ImgSide * ImgSide> m_ElevData; //обработанные данные о высоте
};

class ElevationWarper
{
public:
	GDALWarpOperation& GetWarpOper()
		{return m_WarpOper;}
	
	float *MakeBuffer(int XSize, int YSize);
	void Init();
	//обёртка над методом WarpRegionToBuffer()
	void WarpToMap(ElevMap& Map, int XOff, int YOff, float *Buf);
private:
	GDALWarpOptions* m_WarpOpts = nullptr;
	GDALWarpOperation m_WarpOper;
	static ElevationDataset m_Dataset;
	/* Этот метод проверяет, на какие глобальный тайлы накладывается тайл ElevMap,  
	 * и помещает координаты глобальных тайлов в вектор Tiles*/
	void m_GetOverlappingTiles(GDALDataset* ElevMap, std::vector<coord2d>& Tiles);
};

class ElevationDataset
{
public:
	/* Этот метод возвращает определённый тайл, или вызывает m_ReadTile с m_DownloadTile при необходимости */
	GDALDataset* GetTile(int Lon, int Lat);
	/*Этот метод загружает тайл с диска*/
	bool m_ReadTile(GDALDataset*, int Lon, int Lat);
	/*Этот метод скачивает тайл */
	bool m_DownloadTile(GDALDataset*, int Lon, int Lat);
	
	/* Этот тайл возвращается методом GetTile, в случае если запрашиваемый тайл
	 * не был найден. Данные в нем забиты нулями (nodata)*/
	GDALDataset* m_DummyTile = nullptr;
	
	/* Этот массив хранит указатели на тайлы с данным о высоте
	 * над уровнем моря.
	 * Массив имеет размерность 360*180 из-за того, что тайлы
	 * покрывают площадь Земли размером 1*1 градус,
	 * пока не реализовано - используется перебор всех тайлов,
	 * переданных в настройках
	 */
	//std::array<GDALDataset*, 360 * 180> m_Storage;
};
