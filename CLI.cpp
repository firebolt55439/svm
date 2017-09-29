#include <iostream>
#include <string>
#include <sstream>
#include <exception>
#include "CLIFile.cpp"

#define VERSION "0.1.4a (debug)"

class CLI {
	int arc;
	char** arv;
	CLIFile* clf;
	protected:
		void usage(void);
		void info(void);
		void version(void);
		int exists(std::string search);
	public:
		CLI(int argc, char** argv, int* flags);
		~CLI(void);
		std::string Contents(void);
};

CLI::CLI(int argc, char** argv, int* flags = NULL){
	if(argc == 1) usage();
	arc = argc;
	arv = argv;
	if((exists("--help")) || (exists("-h"))) usage();
	if((exists("--info")) || (exists("-i"))) info();
	if((exists("--version")) || (exists("-v"))) version();
	if((exists("--debug")) || (exists("-d"))) logger.set(2); // display all
	if(flags != NULL){
		if((exists("--compile")) || (exists("-c"))) *flags = 1;
		if((exists("--bin")) || (exists("-b"))) *flags = 2;
		if((exists("--run")) || (exists("-r"))) *flags = 0; // means compile + interpret
	}
	std::string fname = std::string(argv[(argc-1)]);
	clf = new CLIFile(fname);
	logger << "CLI class successfully instantiated with arguments.\n";
}

CLI::~CLI(void){
	delete clf;
	logger << "CLI class freed.\n";
}

void CLI::usage(void){
	cout << "The Sumer Virtual Machine (SVM).\n";
	cout << "Written By Sumer Kohli in C++.\n";
	cout << "Usage: " << getenv("_") << " [arguments] [file].\n";
	cout << "[--help/-h] [--info/-i] [--version/-v] [--compile/-c] [--run/-r] [--bin/-b] [--debug/-d]\n";
	exit(0);
}

void CLI::info(void){
	cout << "The Sumer Virtual Machine (SVM).\n";
	cout << "Written By Sumer Kohli in C++.\n";
	cout << "Number of virtual registers: " << MAX_REGS << ".\n"; // since Parser is included before this
	cout << "Date of compilation: " << __DATE__ << ".\n";
	exit(0);
}

void CLI::version(void){
	cout << "The Sumer Virtual Machine (SVM) Version " << VERSION << ".\n";
	cout << "Written by Smer Kohli in C++.\n";
	exit(0);
}

int CLI::exists(std::string search){
	for(int i = 1; i < arc; i++){
		std::string tmp = std::string(arv[i]);
		if(tmp.length() != search.length()) continue;
		if(search.compare(0, tmp.length(), tmp) == 0) return 1;
	}
	return 0;
}

std::string CLI::Contents(void){
	return clf->Contents();
}


































