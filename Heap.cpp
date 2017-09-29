#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector> // vectors!
#include <exception>

#define HEAP_SLOTS 64
#define PER_SSLOT 256

class Heap {
	std::string sslots[HEAP_SLOTS];
	double slots[HEAP_SLOTS];
	int sav[HEAP_SLOTS]; // available for string
	int av[HEAP_SLOTS]; // available for double
	protected:
		void error(std::string str);
		void init(int flags = -1);
	public:
		Heap(void);
		Heap(int flags);
		~Heap(void);
		int sreg(void);
		int nreg(void);
		void sfree(int n);
		void nfree(int n);
		std::string sval(int n);
		double nval(int n);
		void nset(int n, double v);
		void sset(int n, std::string v);
		double& operator() (int index); // operator overloading!
		double& operator() (unsigned char index);
		std::string& operator() (int type, int index); // double overload!
};

Heap::Heap(void){
	init();
}

Heap::Heap(int flags){
	init(1);
}

Heap::~Heap(void){
	logger << "Destruction of heap successful.\n";
}

void Heap::init(int flags){
	for(int i = 1; i < HEAP_SLOTS; i++){
		if(flags == -1) sslots[i].resize(PER_SSLOT); // unless later more is requested
		sav[i] = 1;
		av[i] = 1;
		slots[i] = 0;
	}
	logger << "\nHeap initialized with " << HEAP_SLOTS << " slots.\n";
	logger << "Heap Memory Usage:\n";
	logger << HEAP_SLOTS << " double slots at " << sizeof(double) << " bytes per slot.\n";
	logger << "Memory Used: " << (HEAP_SLOTS*sizeof(double)) << " bytes.\n";
	if(flags == -1){
		logger << HEAP_SLOTS << " string slots at " << PER_SSLOT << " bytes per slot.\n";
		logger << "Memory Used: " << (HEAP_SLOTS*PER_SSLOT) << " bytes.\n";
	}
	logger << "Heap successfully instantiated.\n";
}

void Heap::error(std::string str){
	cerr << "\nHeap Fatal Error: " << str << "\n";
	exit(1);
}

int Heap::sreg(void){
	for(int i = 1; i < HEAP_SLOTS; i++){
		if(sav[i]){
			sav[i] = 0;
			return i;
		}
	}
	error("Out of heap string slots.");
	return -1;
}
// won't give out slot #0 to avoid conflicts with registers

int Heap::nreg(void){
	for(int i = 1; i < HEAP_SLOTS; i++){
		if(av[i]){
			av[i] = 0;
			return i;
		}
	}
	error("Out of heap double slots.");
	return -1;
}

void Heap::sfree(int n){
	if((n < 0) || (n >= HEAP_SLOTS)) error("Invalid string slot freed.");
	sav[n] = 1;
}

void Heap::nfree(int n){
	if((n < 0) || (n >= HEAP_SLOTS)) error("Invalid double slot freed.");
	av[n] = 1;
}

std::string Heap::sval(int n){
	if((n < 0) || (n >= HEAP_SLOTS)) error("Invalid string slot requested.");
	return sslots[n];
}

double Heap::nval(int n){
	if((n < 0) || (n >= HEAP_SLOTS)) error("Invalid double slot requested.");
	return slots[n];
}

void Heap::nset(int n, double v){
	if((n < 0) || (n >= HEAP_SLOTS)) error("Invalid heap slot set.");
	slots[n] = v;
}

void Heap::sset(int n, std::string v){
	if((n < 0) || (n >= HEAP_SLOTS)) error("Invalid heap slot set.");
	sslots[n] = v;
}

double& Heap::operator() (int index){
	if((index < 0) || (index >= HEAP_SLOTS)) error("Invalid heap slot requested by operator.");
	return slots[index];
}

double& Heap::operator() (unsigned char index){
	if((index < 0) || (index >= HEAP_SLOTS)) error("Invalid heap slot requested by operator.");
	return slots[(int)(index)];
}

std::string& Heap::operator() (int type, int index){
	if((index < 0) || (index >= HEAP_SLOTS)) error("Invalid heap slot requested by operator.");
	return sslots[index];
}





















