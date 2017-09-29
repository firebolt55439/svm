#include <iostream>
#include <fstream> // file streams
#include <string>
#include <sstream>
#include <exception>

class CLIFile {
	std::string fname;
	protected:
		void error(std::string str);
	public:
		CLIFile(std::string file);
		~CLIFile(void);
		std::string Contents(void);
};

CLIFile::CLIFile(std::string file){
	fname = file;
	logger << "CLIFile class successfully instantiated with values.\n";
}

CLIFile::~CLIFile(void){
	logger << "CLIFile class freed.\n";
}

void CLIFile::error(std::string str){
	cerr << "CLIFile Error: " << str << "\n";
	exit(1);
}

std::string CLIFile::Contents(void){
	std::string val;
	std::ifstream fp(fname.c_str(), std::ios::in); // read-only
	if(fp){
		fp.seekg(0, fp.end); // go to end
		int size = fp.tellg();
		fp.seekg(0, fp.beg); // back to start
		val.reserve((size+1));
		while(!fp.eof()) val.push_back(fp.get());
	} else error("Unable to open file for reading.");
	logger << "Successfully read file |" << fname << "| of size " << val.size() << ".\n";
	return val;
}



























