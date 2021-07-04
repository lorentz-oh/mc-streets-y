#pragma once
#include <string>
#include <unordered_map>

class Settings
{
public:
    Settings();
    double GetDouble(std::string Key);
    float GetFloat(std::string Key);
    bool GetBool(std::string Key);
    int GetInt(std::string Key);
    std::string GetStr(std::string Key);
    bool readConfigFile(const std::string& filename);
	
	//Проверяет суещствует ли ключ 
	bool Exists(std::string Key);
private:
    void parseOption(const std::string &line, const std::string &end);
    bool parseConfigLines(std::istream &is, const std::string &end = "");
    std::unordered_map<std::string, std::string> m_settings;
    std::unordered_map<std::string, std::string> m_defaults;
};

extern Settings S;
