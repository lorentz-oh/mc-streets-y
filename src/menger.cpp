#include "menger.h"
#include <cmath>

using std::pow;

inline bool isInSponge(int X, int Y, int Z)
{
	const int tOrder = 6;
	for(int i = 1; i < tOrder; ++i){
		int N = pow(3,i);
		int Dims = 0;
		Dims += (X % N / (N/3)) == 1;
		Dims += (Y % N / (N/3)) == 1;
		Dims += (Z % N / (N/3)) == 1;
		if(Dims > 1){
			return false;}
	}
	return true;
}

void MakeSponge(Region & Reg)
{
	for(int X = 0; X < 243; ++X){
		for(int Y = 0; Y < 243; ++Y){			
			for(int Z = 0; Z < 243; ++Z){
				if(isInSponge(X,Y,Z)){
					Reg.SetBlock(X,Y,Z,41,0,0);}
				else{
					Reg.SetBlock(X,Y,Z,0 ,0,0);}
			}
		}
	}
}
