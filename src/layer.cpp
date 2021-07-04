#include "layer.h"
#include <boost/property_tree/xml_parser.hpp>
#include <fstream>
#include <string>
#include <iostream>
#include <filesystem>

using boost::property_tree::ptree;

bool RegMap::isEmpty()
{
	for(int X = 0; X < RegSide; ++X)
		for(int Y = 0; Y < RegSide; ++Y){
			if(IsDyed(X,Y)){
				return false;}
		}
	return true;
}

void Scheme::ParseScheme(std::string SchemeFile)
{
	ptree Scheme;
	read_xml(SchemeFile, Scheme);
	
	m_Surface.ParseLayer(Scheme.get_child("Surface"));

	for(auto & Layer : Scheme.get_child("Layers"))
	{
		if(Layer.first == "<xmlattr>"){
			continue;}
		m_Layers.emplace_back();
		m_Layers.back().ParseLayer(Layer.second);
	}
}

void Layer::ParseLayer(ptree & LayerPTree)
{
	m_Ranges.clear();
	for(auto & BlockRange : LayerPTree)
	{
		if(BlockRange.first == "<xmlattr>"){
			continue;}
		m_Ranges.emplace_back();
		m_Ranges.back().ParseRange(BlockRange.second);
	}

	m_Name = LayerPTree.get_child("<xmlattr>.name").get_value<std::string>();

	m_Structs.clear();
	std::string Path;
	try{Path = LayerPTree.get_child("<xmlattr>.structures_path").get_value<std::string>();}
	catch(...){return;}
	
	//reading structures
	std::ifstream Ifs;
	for (const auto & Entry : std::filesystem::directory_iterator(Path)){
		if(!std::filesystem::is_regular_file(Entry)){
			continue;}
		std::cout << "Structure:" << Entry.path() << std::endl;
		Ifs.open(Entry.path());
		m_Structs.emplace_back();
		m_Structs.back().Read(Ifs);
		Ifs.close();
	}
}

void BlockRange::ParseRange(boost::property_tree::ptree & pTree)
{	
	Id = pTree.get<int>("<xmlattr>.id");
	Add = pTree.get<int>("<xmlattr>.add");
	Data = pTree.get<int>("<xmlattr>.data");
	From = pTree.get<int>("<xmlattr>.from");
	To = pTree.get<int>("<xmlattr>.to");
	std::string StrMode = pTree.get<std::string>("<xmlattr>.mode");
	if(StrMode == "absolute"){
		Mode = OFF_ABSOLUTE;}
	else if(StrMode == "elevation"){
		Mode = OFF_RELATIVE;}
	else{
		Mode = OFF_RELATIVE;}
		
	std::string StrSmooth = pTree.get<std::string>("<xmlattr>.smooth");
	if(StrSmooth == "data"){
		Smooth = SMOOTH_DATA;}
	else{
		Smooth = SMOOTH_NONE;}
}

Scheme g_Scheme;
