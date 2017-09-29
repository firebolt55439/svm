#include <iostream>
#include <string>
#include <sstream>
#include <exception>
#include <stack>

class Stacks {
	std::stack<double> int_arr;
	std::stack<std::string> char_arr;
	std::stack<int> func_stack; // function position stack
	std::stack<int> rfunc_stack; // function return value stack
	public:
		Stacks(void); // constructor
		~Stacks(void); // destructor
		void clean(void); // clean up stacks
		void push(double val); // for the int_arr stack
		void push(std::string val);
		void push(char* val); // both for the char_arr stack
		void fpush(int val); // for the func_stack stack
		void rpush(int val); // for the rfunc_stack stack
		int isize(void);
		int csize(void);
		int fsize(void);
		int rsize(void); // numbers of elements on stack for each stack
		double ipop(void);
		std::string cpop(void);
		int fpop(void);
		int rpop(void); // pop and return elements off stacks
};

Stacks::Stacks(void){
	logger << "Stacks class constructed successfully.\n";
}

Stacks::~Stacks(void){
	logger << "Cleaning up Stacks...\n";
	while(!int_arr.empty()) int_arr.pop();
	while(!char_arr.empty()) char_arr.pop();
	while(!func_stack.empty()) func_stack.pop();
	while(!rfunc_stack.empty()) rfunc_stack.pop();
	logger << "Cleaned up Stacks!\n";
}

void Stacks::clean(void){ // cleans "argument" stacks
	while(!int_arr.empty()) int_arr.pop();
	while(!char_arr.empty()) char_arr.pop();
}

void Stacks::push(double val){
	int_arr.push(val);
}

void Stacks::push(std::string val){
	char_arr.push(val);
}

void Stacks::push(char* val){
	std::string sval = std::string(val);
	char_arr.push(sval);
}

void Stacks::fpush(int val){
	func_stack.push(val);
}

void Stacks::rpush(int val){
	rfunc_stack.push(val);
}

int Stacks::isize(void){
	return int_arr.size();
}

int Stacks::csize(void){
	return char_arr.size();
}

int Stacks::fsize(void){
	return func_stack.size();
}

int Stacks::rsize(void){
	return rfunc_stack.size();
}

double Stacks::ipop(void){
	double val = int_arr.top();
	int_arr.pop();
	return val;
}

std::string Stacks::cpop(void){
	std::string val = char_arr.top();
	char_arr.pop();
	return val;
}

int Stacks::fpop(void){
	int val = func_stack.top();
	func_stack.pop();
	return val;
}

int Stacks::rpop(void){
	int val = rfunc_stack.top();
	rfunc_stack.pop();
	return val;
}



































