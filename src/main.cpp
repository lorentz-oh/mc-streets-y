#include <string>
#include <iostream>
#include <mapnik/coord.hpp>
#include <mapnik/config_error.hpp>
#include "generator.h"
#include "settings.h"
#include "layer.h"
#include "io/stream_reader.h"
#include "io/izlibstream.h"

int main(int argc, char** argv)
{
    if(argc !=2){
        std::cout << "Usage: " << argv[0] << " [config file]" << std::endl;
        return 0;
    }
    if(!S.readConfigFile(argv[1])){
        std::cout << "Couldn't read config file\n";
        return -1;}

    try {
		mapnik::coord2d MCC{S.GetDouble("mccx"), S.GetDouble("mccz")};
        generator Gen{MCC, 0.0};

        std::string Mode = S.GetStr("mode");
        if(Mode == "preview"){
            Gen.DebugGenerate();}
        else if(Mode == "generate"){
            g_Scheme.ParseScheme(S.GetStr("scheme"));
			Gen.Generate();}
		else {
			std::cout << "Invalid mode setting" << std::endl;
		}
    } catch (std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return -1;
    } catch (nbt::io::input_error&){
        std::cerr << "Input error\n";
        return -1;
    } catch (zlib::zlib_error&){
        std::cerr << "Zlib error\n";
        return -1;
    } 
}
