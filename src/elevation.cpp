#include "elevation.h"
#include "settings.h"
#include <mapnik/load_map.hpp>
#include <mapnik/map.hpp>

ElevationWarper ElevMap::m_Warper{};
ElevationDataset ElevationWarper::m_Dataset{};

std::string GetProjection()
{
	mapnik::Map TempMap;
	mapnik::load_map(TempMap, S.GetStr("style"));
	return TempMap.srs();
}
	
ElevMap::ElevMap(int RegX, int RegZ)
{
	Render(RegX, RegZ);
}

void ElevMap::Render(int RegX, int RegZ)
{
	int MCCX = S.GetInt("mccx");
	int MCCZ = S.GetInt("mccz");
	
	float* Buffer = reinterpret_cast<float*>(m_Warper.GetWarpOper().CreateDestinationBuffer(XRes, YRes));
	
	m_Warper.WarpToMap(*this, 
			RegX * RegSide - Margin - MCCX, 
			RegZ * RegSide - Margin + MCCZ, //ось перевёрнута 
			Buffer);
	
	float from = S.GetFloat("elev_from");
	float to = S.GetFloat("elev_to");
	float elev_scale =(float)BUILD_LIMIT / std::abs(to - from);
	float tmp;
	for(int X = 0; X < XRes; ++X){
		for(int Y = 0; Y < YRes; ++Y){
			tmp = Buffer[X + Y * XRes];
			tmp -= from;
			tmp *= elev_scale;
			//gdal рендерит карту отзеркаленной по оси Z,
			//поэтому заполняем буфер наоборот
			//см. region_generator.cpp
			m_ElevData[X + (YRes - 1 - Y) * XRes] = tmp * 16;
		}
	}

	m_Warper.GetWarpOper().DestroyDestinationBuffer(reinterpret_cast<void*>(Buffer));
}

void ElevationWarper::Init()
{

	GDALAllRegister();
	GDALDatasetH hSrcDS = GDALOpen( S.GetStr("dataset").c_str(), GA_ReadOnly );

	OGRSpatialReference oSRS;
	char *pszDstWKT = nullptr;
	const char *pszSrcWKT = GDALGetProjectionRef(hSrcDS);
	std::string Proj = GetProjection();
	oSRS.importFromProj4(Proj.c_str());
	oSRS.exportToWkt(&pszDstWKT);

	//здесь устанавливаются различные опции для gdalwarper
	m_WarpOpts = GDALCreateWarpOptions();
	m_WarpOpts->eWorkingDataType = GDT_Float32; //используем float для карты
	m_WarpOpts->hSrcDS = hSrcDS; 
	m_WarpOpts->hDstDS = NULL;
	m_WarpOpts->eResampleAlg = GRA_Bilinear;
	m_WarpOpts->nBandCount = 1;
	m_WarpOpts->panSrcBands =
		(int *) CPLMalloc(sizeof(int) * m_WarpOpts->nBandCount );
	m_WarpOpts->panSrcBands[0] = 1;
	m_WarpOpts->panDstBands =
		(int *) CPLMalloc(sizeof(int) * m_WarpOpts->nBandCount );
	m_WarpOpts->panDstBands[0] = 1;
	m_WarpOpts->pfnProgress = GDALTermProgress;

	m_WarpOpts->pTransformerArg =
	GDALCreateGenImgProjTransformer( hSrcDS,
                                        pszSrcWKT,
										NULL,
										pszDstWKT,
                                        FALSE, 0.0, 1 );
	m_WarpOpts->pfnTransformer = GDALGenImgProjTransform;
	m_WarpOper.Initialize( m_WarpOpts );
}

void ElevationWarper::WarpToMap(ElevMap& Map, int XOff, int YOff, float *Buffer)
{
	//TODO более умная логика для выбора тайлов
	m_WarpOper.WarpRegionToBuffer(XOff, YOff, Map.XRes, Map.YRes, Buffer, GDT_Float32);
}

/*
void ElevationWarper::GetRegElev(coord2d<int> RegCoord, ElevMap & RegElevMap, GDALTransformation Transformation)
{
	std::vector<coord2d> Tiles;
	m_GetOverlappingTiles(RegElevMap, Tiles);
	
	for(auto TileCoord : Tiles){
		auto Tile = ElevDataset.GetTile(TileCoord);
		GDALWarpOperation WarpOp{Transformation}; //Это не рабочий код, просто прототип
		WarpOp.Transform(Tile, RegMap);
	}
}

//Тип для хранения уникальных координат
typedef CoordsSet = std::unordered_map<int, std::set<int>>;

inline void UpdateCoords(CoordsSet, int X, int Y)
{
	PixelToTileCoords(X,Y);
	CoordsSet.try_emplace(X, {});
	CoordsSet[X].emplace(Y);
}

void ElevationWarper::m_GetOverlappingTiles(GDALDataset* ElevMap, std::vector<coord2d>& Tiles)
{
	CoordsSet TileCoords; 
		//Создаём трансформацию из системы координат базы тайлов в систему координат карты высот
	OGRCoordinateTransformation * CoordTrans = OGRCreateCoordinateTransformation(ElevDataset.GetProjectionRef, ElevMap.GetProjectionRef);
	
	int XSize = ElevMap.GetRasterXSize;	
	int YSize = ElevMap.GetRasterYSize;
	
	for(int X = 0; X < XSize; ++X){
		UpdateCoords(TileCoords, X, 0);
		UpdateCoords(TileCoords, X, YSize);}
	
	for(int Y = 0; Y < XSize; ++Y){
		UpdateCoords(TileCoords, 0, Y);
		UpdateCoords(TileCoords, XSize, Y);}
}
*/
