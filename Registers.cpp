#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <exception>
#include <cmath>
#include <complex>

#define MAX_REGS 48
#define PER_SREG 256 // bytes per string register

using namespace std;

class Registers {
	double regs[MAX_REGS]; // double registers for floating-point ops
	char* sregs[MAX_REGS]; // "true" C-string registers for string ops
	Heap* heap; // heap for heap operations
	protected:
		void error(std::string str);
		double op_res(double* a, double* b, std::string type);
		void op(int a, int b, std::string type);
	public:
		Registers(Heap* hp); // constructor
		~Registers(void); // destructor
		void update(int reg, double val);
		void update(int reg, std::string val);
		void update(int reg, char* val); // overloaded method
		void copy(int a, int b, int type);
		void dump(int a, int b);
		void add(int a, int b); // regs[b] += regs[a]
		void sub(int a, int b); // regs[b] -= regs[a]
		void mul(int a, int b); // regs[b] *= regs[a]
		void div(int a, int b); // regs[b] /= regs[a]
		void print(int a, int flags);
		void cmp(int a, int b, int& e, int& l, int& g);
		std::string cval(int a);
		double nval(int a);
};

Registers::Registers(Heap* hp){
	for(int i = 0; i < MAX_REGS; i++){
		regs[i] = nan(NULL);
		sregs[i] = (char*) malloc(PER_SREG);
		sregs[i][0] = '\0';
	}
	heap = hp;
	logger << "\nRegisters initialized with " << MAX_REGS << " registers.\n";
	logger << "Registers Memory Usage:\n";
	logger << MAX_REGS << " double registers at " << sizeof(double) << " bytes per register.\n";
	logger << "Memory Used: " << (MAX_REGS*sizeof(double)) << " bytes.\n";
	logger << MAX_REGS << " string registers at " << PER_SREG << " bytes per register.\n";
	logger << "Memory Used: " << (MAX_REGS*PER_SREG) << " bytes.\n";
}

Registers::~Registers(void){
	logger << "Freeing Registers...\n";
	for(int i = 0; i < MAX_REGS; i++){
		free(sregs[i]);
	}
	logger << "Registers freed!\n";
}

void Registers::error(std::string str){
	cerr << "Registers Error: " << str << "\n";
	exit(1);
}

void Registers::update(int reg, double val){
	if((reg < 0) || (reg >= MAX_REGS)) error("Invalid register number altered.");
	regs[reg] = val;
}

void Registers::update(int reg, std::string val){
	if((reg < 0) || (reg >= MAX_REGS)) error("Invalid register number altered.");
	size_t copied = val.copy(sregs[reg], val.length(), 0);
	sregs[reg][copied] = '\0';
}

void Registers::update(int reg, char* val){
	if((reg < 0) || (reg >= MAX_REGS)) error("Invalid register number altered.");
	for(int at = 0; at < strlen(val); at++){
		sregs[reg][at] = val[at];
	}
	sregs[reg][strlen(val)] = '\0';
}

void Registers::copy(int a, int b, int type = 1){
	// means copy value of b onto a, or that regs[a] = regs[b];
	if((b < 0) || (b >= MAX_REGS)) error("Invalid register number altered.");
	if(type == 1){
		update(a, regs[b]);
	} else if(type == 2){
		update(a, sregs[b]);
	} else if(true) error("Invalid type for register copy.");
}

double Registers::op_res(double* a, double* b, std::string type){
	if(type == "add"){
		return ((*b)+(*a));
	} else if(type == "sub"){
		return ((*b)-(*a));
	} else if(type == "mul"){
		return ((*b)*(*a));
	} else if(type == "div"){
		return ((*b)/(*a));
	} else if(true) error("Unknown operation '"+type+"'.");
	return 0;
}

void Registers::op(int a, int b, std::string type){
	double *ar, *br; // two double pointers
	double tmp;
	if((a < 0) && (b > 0)){
		if((b < 0) || (b >= MAX_REGS)) error("Invalid register number altered.");
		tmp = heap->nval(-a);
		ar = &tmp;
		br = &(regs[b]);
		update(b, op_res(ar, br, type));
	} else if((a > 0) && (b < 0)){
		if((a < 0) || (a >= MAX_REGS)) error("Invalid register number altered.");
		tmp = heap->nval(-b);
		ar = &(regs[a]);
		br = &tmp;
		heap->nset(*br, op_res(ar, br, type)); // update heap value
	} else if((a > 0) && (b > 0)){
		if((a < 0) || (a >= MAX_REGS)) error("Invalid register number altered.");
		else if((b < 0) || (b >= MAX_REGS)) error("Invalid register number altered.");
		ar = &(regs[a]);
		br = &(regs[b]);
		update(b, op_res(ar, br, type));
	}
}

void Registers::add(int a, int b){
	op(a, b, "add");
}

void Registers::sub(int a, int b){
	op(a, b, "sub");
}

void Registers::mul(int a, int b){
	op(a, b, "mul");
}

void Registers::div(int a, int b){
	op(a, b, "div");
}

void Registers::print(int a, int flags = -1){
	if((a < 0) || (a >= MAX_REGS)) error("Invalid register number for printing.");
	logger << "Printing: |";
	if(flags == -1) cout << std::dec << std::setprecision(16) << regs[a];
	else cout << sregs[a];
	logger << "|.\n";
}

void Registers::cmp(int a, int b, int& e, int& l, int& g){
	if((b < 0) || (b >= MAX_REGS)) error("Invalid register number for comparison.");
	else if((a < 0) || (a >= MAX_REGS)) error("Invalid register number for comparison.");
	if(regs[a] == regs[b]){ 
		e = 1;
		l = g = 0;
	} else if(regs[a] < regs[b]){
		l = 1;
		e = g = 0;
	} else if(regs[a] > regs[b]){
		g = 1;
		e = l = 0;
	}
}

std::string Registers::cval(int a){
	if((a < 0) || (a >= MAX_REGS)) error("Invalid register number requested.");
	return std::string(sregs[a]);
}

double Registers::nval(int a){
	if((a < 0) || (a >= MAX_REGS)) error("Invalid register number requested.");
	return regs[a];
}

void Registers::dump(int a, int b){
	// if a is negative, then string registers
	if((b < a) || (a == 0)){
		logger << "Invalid dump arguments (from " << a << " to " << b << ").\n";
		return;
	} else if(a < 0){
		a *= (-1);
		for(int i = a; i <= b; i++){
			logger << "String register #" << i << " has value |" << sregs[i] << "|.\n";
		}
	} else if(a > 0){
		for(int i = a; i <= b; i++){
			logger << "Double register #" << i << " has value |" << std::dec << std::setprecision(16) << regs[i] << "|.\n";
		}
	}
}










































		