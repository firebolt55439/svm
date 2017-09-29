#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <ostream>
#include <sstream>

using namespace std;

class Logger {
    int log_level;
	public:
		void set(int lvl){
			log_level = lvl;
		}
		
		int lvl(void){
			return log_level;
		}
		
		template<typename T> Logger& operator << (const T& object){
			if(log_level > 1) cout << object;
			return *this;
		}
};

Logger logger; // global var for other included files

/*
int main(void){
	Logger logger;
	logger.set(2);
	logger << "This will be logged to std::cout" << "\n";
	logger.set(3);
	logger << "This also go out." << "\n";
	logger.set(1);
	logger << "This will not.\n";
}
*/
