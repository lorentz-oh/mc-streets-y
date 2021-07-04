#include <filesystem>
#include <iostream>
#include <mapnik/image.hpp>
#include <mapnik/image_util.hpp>
#include "region_generator.h"
#include "settings.h"
#include "const.h"
#include "ids.h"
#include "layer.h"
#include "menger.h"

noise::module::Perlin PerlinModule;

RegionGenerator::RegionGenerator(int RegX, int RegZ, bool Overwrite):
    RegX{RegX},
    RegZ{RegZ},
    m_Elev{},
	Reg{RegX, RegZ, S.GetStr("region_dir")}
{
	m_Overwrite = Overwrite && Reg.Exists();
}

void RegionGenerator::GenerateRange(BlockRange & Range, RegMap & RegMap, bool Everywhere)
{
	auto & Extent = m_Layers["Extent"];
	int H;
	char Data;
	int MinY, MaxY;
	
	for(int X = 0; X < RegSide; ++X){
		for(int Z = 0; Z < RegSide; ++Z){
			if(!Extent.IsDyed(X,Z)){
				continue;}
			if( !(Everywhere || RegMap.IsDyed(X,Z)) ){
				continue;}
			H = Range.To + (Range.Mode == OFF_RELATIVE? m_Elev(X,Z) : 0);
			Data = Range.Smooth == SMOOTH_DATA? H%16 : Range.Data;
			MinY = (H - std::abs(Range.To - Range.From)) / 16;
			MaxY = H / 16;

			if(MinY < 0 ){
				MinY = 0;}
			if(MinY > BUILD_LIMIT){
				MinY = BUILD_LIMIT;}

			if(MaxY < 0){
				MaxY = 0;}
			if(MaxY > BUILD_LIMIT){
				MaxY = BUILD_LIMIT;}

			for(int Y = MinY; Y < MaxY; ++Y){	
				Reg.SetBlock(X, Y, Z, Range.Id, Range.Add, Range.Data);
			}
			Reg.SetBlock(X, MaxY, Z, Range.Id, Range.Add, Data);
		}
	}
}

void RegionGenerator::GenerateLayer(Layer & Lyr, RegMap & RegMap, bool Everywhere)
{
	for(auto Range : Lyr.m_Ranges){
		GenerateRange(Range, RegMap, Everywhere);
	}
}

void RegionGenerator::GenerateStructures(const Layer & Lyr)
{
	auto & Extent = m_Layers["Extent"];
	RegMap & Map = m_Layers[Lyr.m_Name]; 
	int Count = Lyr.m_Structs.size();
	int Y = 0;
	if(Count == 0){
		return;}
	for(int X = -Margin; X < RegSide+Margin; ++X){
		for(int Z = -Margin; Z < RegSide+Margin; ++Z){

			if(PerlinModule.GetValue(X,Z,10.30) < 0.88 || !Map.IsDyed(X,Z) || !Extent.IsDyed(X,Z) ){
				continue;}

			auto Struct = Lyr.m_Structs[ std::abs(X ^ Z) % Count ];
			
			Y = m_Elev(X,Z) / 16;
			Struct.Put(Reg, X, Y, Z);
		}
	}
}

void RegionGenerator::Generate()
{
	if(m_Layers["Extent"].isEmpty()){
		std::cout << "Region was skipped ";
		return;
	}

	if(m_Layers["Ufa"].isEmpty()){
		MakeSponge(Reg);
		Reg.SaveRegion();
		return;}
	//gdal рендерит карту отзеркаленной по оси Z,
	//поэтому нужно перевернуть координату
	m_Elev.Render(RegX, -(RegZ+1)); 
		//Обрабатываем параметры
	if(S.GetStr("treatment") == "append"){
		UnmarkExtent();}
		
	GenerateLayer(g_Scheme.m_Surface, m_Layers.at("Extent"), true);
	for(auto & Lyr : g_Scheme.m_Layers){
		GenerateStructures(Lyr);
		GenerateLayer(Lyr, m_Layers.at(Lyr.m_Name), false);
	}
	Reg.SaveRegion();
}

void RegionGenerator::UnmarkExtent()
{
	auto & Extent = m_Layers["Extent"];
	for(int X = 0; X < RegSide; ++X){
		for(int Z = 0; Z < RegSide; ++Z){
			//Проверяем наличие блоков в колонне
			for(int Y = 0; Y < BUILD_LIMIT; ++Y){
				
				if(Reg.GetBlock(X, Y, Z) != 0){ //0 - id Воздуха
					Extent.Unmark(X,Z);
					break; 
				}
			}
		}
	}
}

void RegionGenerator::ClearExtent()
{
	auto & Extent = m_Layers["Extent"];
	for(int X = 0; X < RegSide; ++X){
		for(int Z = 0; Z < RegSide; ++Z){
			if(Extent.IsDyed(X,Z)){
				continue;}
			for(int Y = 0; Y < BUILD_LIMIT; ){
				//air
				Reg.SetBlock(X, Y, Z, 0, 0, 0);
			}
		}
	}
}

void BlurImg(image_rgba8& Img)
{
	int Width = Img.width();
	int Height = Img.height();
	image_rgba8 Buf{Width, Height};

	for(int X = BlurR; X < Width-BlurR; ++X){
		for(int Y = BlurR; Y < Height-BlurR; ++Y){
			unsigned int Sum = 0;
			for(int i = -BlurR; i < BlurR+1; ++i){
				Sum += Img(X + i,Y);
			}
			Sum /= (BlurR*2+1);
			Buf(X,Y) = Sum;
		}
	}
	for(int X = BlurR; X < Width-BlurR; ++X){
		for(int Y = BlurR; Y < Height-BlurR; ++Y){
			unsigned int Sum = 0;
			for(int i = -BlurR; i < BlurR+1; ++i){
				Sum += Buf(X,Y + i);
			}
			Sum /= (BlurR*2+1);
			Img(X,Y) = Sum;
		}
	}

}

void PrepElevationMult(image_rgba8& Elev)
{
	int Width = Elev.width();
	int Height = Elev.height();
	for(int X = 0; X < Width; ++X){
		for(int Y = 0; Y < Height; ++Y){
			auto Col = Elev(X,Y);
			u_char * p_Px = reinterpret_cast<u_char*>(&Col);
			Elev(X,Y) = p_Px[0] + p_Px[1] * 0xff;
		}
	}
}

//not used 
void PrepElevation(image_rgba8& Elev)
{
	int Width = Elev.width();
	int Height = Elev.height();
	for(int X = 0; X < Width; ++X){
		for(int Y = 0; Y < Height; ++Y){
			auto& Col = Elev(X,Y);
			Col = *(reinterpret_cast<u_char*>(&Col));
			Col *= 16;
		}
	}

	//this loop smoothes the terrain
	for(int X = 0; X < Width; ++X){
		int YPl = 0; //Y coordiantes of the placer
		int YOb = 0; //and observer
		auto Col = Elev(X,0);
		while(YPl < Height){//processing one row in this loop
			while(Elev(X,YOb) == Col || YOb+3 > Height){//three is a cool number
				++YOb;
				if((YOb - YPl) > 16){
					++YPl;}
			}
			int Dist = YOb - YPl;
			for(int i = 0; i < Dist; ++i){
				Elev(X,YPl) = Elev(X,YPl) + i*16/Dist;
			}
			YPl = YOb;
		}
	}
}
