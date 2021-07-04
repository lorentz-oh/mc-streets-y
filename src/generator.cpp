#include "generator.h"
#include "const.h"
#include "layer.h"
#include "region_generator.h"
#include "settings.h"
#include "elevation.h"
#include "mapnik/agg/agg_pixfmt_gray.h"
#include <mapnik/box2d.hpp>
#include <mapnik/map.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/image.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/projection.hpp>
#include <string>
#include <cmath>
#include <algorithm>
#include <boost/property_tree/xml_parser.hpp>

/*
double GetScale()
{
	const static std::string ScaleAttr = "+k_0";
	std::string Proj = S.GetStr("map_projection");
	auto Start = Proj.find(ScaleAttr);
	Start = Proj.find("=", Start) + 1;
	auto End = Proj.find("+", Start)-1; //find next attribute
	if(End == std::string::npos){
		End = Proj.back();}
	
	auto ScaleStr = Proj.substr(Start, End - Start);
	return std::stod(ScaleStr);
}
*/

generator::generator(mapnik::coord2d MCC, double Scale):
    m_Map{RegSideI,RegSideI},
	m_MCC{MCC}
{
	
	//Устанавливаем границы эталонного box2d
    RefRegBox.set_minx( -MCC.x - Margin);
    RefRegBox.set_miny( MCC.y - ImgSide + Margin);
    RefRegBox.set_maxx(RefRegBox.minx() + (double)ImgSide );
    RefRegBox.set_maxy(RefRegBox.miny() + (double)ImgSide );

	std::string key_base = "datasource_";
	for(int i = 1; S.Exists( key_base + std::to_string(i) ); ++i){
		auto DatasourcePath = S.GetStr(key_base + std::to_string(i));
		mapnik::datasource_cache::instance().register_datasources(DatasourcePath);
	}

    mapnik::load_map(m_Map, S.GetStr("style"));
}

void generator::DebugGenerate()
{
	static const double MAX_IMG_SIZE = 1000.0;
		
	mapnik::Map DebugMap{1, 1};
	mapnik::load_map(DebugMap, S.GetStr("debug_style")); 	
	SetLayer(DebugMap, "Extent");
	DebugMap.zoom_all();
	auto ExtentEnvelope = DebugMap.get_current_extent();
	ToggleAllLayers(DebugMap, true);
	
		//Находим правильный размер для картинки, что-бы сохранить пропорции
		//И ни одна из сторон не была больше MAX_IMG_SIZE пикселей
	auto ImgResX = std::abs(ExtentEnvelope.minx() - ExtentEnvelope.maxx());
	auto ImgResY = std::abs(ExtentEnvelope.miny() - ExtentEnvelope.maxy());
	if(ImgResX > MAX_IMG_SIZE){
		ImgResY = ( ImgResY / ImgResX ) * MAX_IMG_SIZE;
		ImgResX = MAX_IMG_SIZE;}
		
	if(ImgResY > MAX_IMG_SIZE){
		ImgResX = ( ImgResX / ImgResY ) * MAX_IMG_SIZE;
		ImgResY = MAX_IMG_SIZE;}
		
		//Отрисовываем
	DebugMap.resize((int)ImgResX, (int)ImgResY); //необходимое разрешение

	mapnik::image_rgba8 Img{(int)ImgResX, (int)ImgResY};
    mapnik::agg_renderer<mapnik::image_rgba8> Ren(DebugMap, Img);
    Ren.apply();
    mapnik::save_to_file(Img, "extent.png");
}

void generator::Generate()
{
	ElevMap::m_Warper.Init();
	SetTaskViaExtent();
    std::cout << "Started generating " << Task.size() << " regions" << std::endl;
    
	int i = 0;
    for(auto RegPos : Task){
		++i;
        RegionGenerator RegGen{RegPos.x, RegPos.y, S.GetStr("treatment") == "overwrite"};
        RenderReg(RegPos.x, RegPos.y, RegGen);
        RegGen.Generate();
        std::cout << i << "/" << Task.size() << " -- Generated region " << RegPos.x << ", " << RegPos.y << std::endl;
    }
}

void SetLayer(mapnik::Map & Map, std::string LayerName)
{
    int LyrCount = Map.layer_count();
    for(int i = 0; i < LyrCount; i++){
        mapnik::layer& L = Map.get_layer(i);
        if(L.name() == LayerName){
            L.set_active(true);}
        else{
            L.set_active(false);}
    }
}

void ToggleAllLayers(mapnik::Map & Map, bool State)
{
    int LyrCount = Map.layer_count();
    for(int i = 0; i < LyrCount; i++){
        mapnik::layer& L = Map.get_layer(i);
        L.set_active(State);
    }
}

void ToggleLayer(mapnik::Map & Map, std::string LayerName, bool State)
{
    int LyrCount = Map.layer_count();
    for(int i = 0; i < LyrCount; i++){
        mapnik::layer& L = Map.get_layer(i);
        if(L.name() == LayerName){
            L.set_active(State);}
    }
}

mapnik::layer& GetLayer(mapnik::Map & Map, const std::string LayerName)
{
	int LyrCount = Map.layer_count();
    for(int i = 0; i < LyrCount; i++){
		mapnik::layer& L = Map.get_layer(i);
        if(L.name() == LayerName){
			return L;
		}
    }
	throw std::invalid_argument{std::string("Error: Couldn't find layer \"") + LayerName + '\"'};
}

void generator::RenderReg(int XR, int ZR, RegionGenerator &RegGen)
{
    PanToReg(XR, ZR);

	int LyrCount = m_Map.layer_count();
	for(int i = 0; i < LyrCount; i++){
		auto Layer = m_Map.get_layer(i);
		std::string LyrName = Layer.name();
		SetLayer(this->m_Map, LyrName);
		RegMap RegionMap{Margin,RegSideI};
		mapnik::agg_renderer<mapnik::image_rgba8> Renderer{m_Map, RegionMap.m_Img};
		Renderer.apply();
		RegGen.m_Layers.insert({LyrName, std::move(RegionMap)});
	}
}

void generator::PanToReg(int XR, int ZR)
{
    m_Map.resize(ImgSide, ImgSide);
    int PanX = ImgSide / 2;
    int PanY = ImgSide / 2;
    PanX += XR * RegSideI;
    PanY += ZR * RegSideI;
    m_Map.zoom_to_box(RefRegBox);
    m_Map.pan(PanX, PanY);
}


void generator::SetTaskViaExtent()
{
	SetLayer(m_Map, "Extent");
	m_Map.zoom_all();
	auto ExtEnv = m_Map.get_current_extent();

	int MinRegX = std::floor( (  ExtEnv.minx() + m_MCC.x ) / RegSideD );	
	int MinRegY = std::floor( ( -ExtEnv.miny() + m_MCC.y ) / RegSideD );	
	int MaxRegX = std::floor( (  ExtEnv.maxx() + m_MCC.x ) / RegSideD );	
	int MaxRegY = std::floor( ( -ExtEnv.maxy() + m_MCC.y ) / RegSideD );
	SortTwo(MaxRegX, MinRegX);
	SortTwo(MaxRegY, MinRegY);
	std::cout << "Region box: Min = " << MinRegX << " ; " << MinRegY << std::endl;
	std::cout << "Max = " << MaxRegX << " ; " << MaxRegY << std::endl;
	std::cout << "Task size = " << std::abs( (MinRegY - MaxRegY) * (MinRegX - MaxRegX) ) << std::endl;
	std::cout << "Proceed?" << std::endl;
	
	std::cin.get();
	//заполянем вектор Task с заданием для генератора, итерируя через каждый пиксель ExtentMap
	for(int X = MinRegX; X <= MaxRegX; ++X)
		for(int Y = MinRegY; Y <= MaxRegY; ++Y){
			Task.emplace_back(X,Y);}
}

//quick maffs
int DivFloor(int N, int Divisor)
{
    if(N < 0){
        N /= Divisor;
        N --;}
    else {
        N /= Divisor;}
    return N;
}

void AlignToRegionBorders(mapnik::box2d<double> & Box)
{
	Box.set_minx( FloorChunk(Box.minx(), RegSideD));
	Box.set_miny( FloorChunk(Box.miny(), RegSideD));
	Box.set_maxx(  CeilChunk(Box.maxx(), RegSideD));
	Box.set_maxy(  CeilChunk(Box.maxy(), RegSideD));
	
}

void SortTwo(int& One, int& Two)
{
    if(One >= Two){
        return;
    }
    else {
        std::swap(One, Two);
    }
}
