#include "settings.h"
#include <string>
#include <fstream>
#include <iostream>

Settings S;

Settings::Settings()
{

}

bool Settings::readConfigFile(const std::string& filename)
{
    std::ifstream ifs(filename);
    if (!ifs.good())
        return false;

    return parseConfigLines(ifs, "");
}

bool Settings::parseConfigLines(std::istream &is, const std::string &end)
{
    std::string line, name, value;

        while (is.good()) {
            std::getline(is, line);
            parseOption(line, end);
        }

        return end.empty();
}

void Settings::parseOption(const std::string &line, const std::string &end)
{
    if (line.empty())
        return;
    if (line[0] == '#')
        return;
    if (line == end)
        return;

    size_t pos = line.find('=');
    if (pos == std::string::npos)
        return;

    std::string name  = line.substr(0, pos);
    std::string value = line.substr(pos + 1);
    m_settings[name] = value;
}

double Settings::GetDouble(std::string Key)
{
    double Double = 0.0;
    try {
        Double = std::stod(m_settings[Key]);
    } catch (std::invalid_argument& e) {
        std::cerr << "Error in key " << Key << " ";
        throw e;
    }
    return Double;
}

float Settings::GetFloat(std::string Key)
{
    float TheFloat = 0.0;
    try {
        TheFloat = std::stof(m_settings[Key]);
    } catch (std::invalid_argument& e) {
        std::cerr << "Error in key " << Key << " ";
        throw e;
    }
    return TheFloat;
}

bool Settings::GetBool(std::string Key)
{
    std::string StrBool = m_settings[Key];
    if(StrBool == "true"){
        return true;}
    if(StrBool == "false"){
        return false;}

    std::cerr << "Error in key " << Key << " ";
    throw std::invalid_argument("");
}

int Settings::GetInt(std::string Key)
{
    int Integer = 0;
    try {
        Integer = std::stoi(m_settings[Key]);
    }
    catch(std::invalid_argument& e){
        std::cerr << "Error in key " << Key << " ";
        throw e;
    }
    return Integer;
}

std::string Settings::GetStr(std::string Key)
{
    if(m_settings.find(Key) == m_settings.end()){
        return "";
    }
    return m_settings[Key];
}

bool Settings::Exists(std::string Key)
{	
	return !( m_settings.find(Key) == m_settings.end() );
}
