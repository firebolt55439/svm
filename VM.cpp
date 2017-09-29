#include <iostream>
#include <ostream>
#include <string>
#include <sstream>
#include <exception>
#include "Logger.cpp"
#include "Parser.cpp"
#include "CLI.cpp"

int main(int argc, char** argv){
	logger.set(1);
	int compile_only = 0;
	CLI* cli = new CLI(argc, argv, &compile_only);
	cout << "Hello, World!\n";
	/*
	string tmp = "lda r2, 100.3;\n"
	"lda r3, 100.4;";
	Lexer* lx = new Lexer(tmp);
	for(int i = 0; i < tmp.length(); i++){ // since absolute max is one symbol per char in string
		lx->getsym();
		lexeme cur = lx->cur();
		int b_a = lx->st();
		int b_b = lx->end();
		int pos = lx->cpos();
		if(pos > tmp.length()) break;
		// cout << b_a << ", " << b_b << ", " << pos << ".\n";
		lx->shsym(cur);
	} */
	if(!compile_only){ // compile into bytecode + interpret --> default
		std::string tmp = cli->Contents();
		logger << "Parsing: |" << tmp << "|\n";
		Parser* pr = new Parser(tmp);
		pr->Start();
		delete pr; 
		delete cli; // call destructors and clean up
	} else if(compile_only == 1){ // compile-only
		logger.set(2); // display all
		std::string tmp = cli->Contents();
		logger << "Parsing: |" << tmp << "|\n";
		Parser* pr = new Parser(tmp, 1);
		pr->Start();
		delete pr; 
		delete cli; // call destructors and clean up
	} else if(compile_only == 2){ // interpret from bytecode file
		Bytecode* bc = new Bytecode("code.bin", 1);
		bc->init_read();
		bc->Start();
		delete bc;
		delete cli; // call destructors and clean up
	}
	/*
	Bytecode* t = new Bytecode("code.bin", 1);
	t->init_read();
	t->Start();
	*/
	return 0;
}