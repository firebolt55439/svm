#include <iostream>
#include <string>
#include <sstream>
#include <exception>

using namespace std;

typedef enum { letters, number, comma, semicolon, lparen, rparen, op_add, op_sub, op_mul, op_div, 
equals, dquote, squote, hashchar, colon, backslash, lbrace, rbrace, lbrack, rbrack, period, amp,
space, op_l, op_leq, op_g, op_geq, na, unknown } lexeme;


class Lexer {
	std::string data; // actual data buffer
	lexeme current; // current lexeme
	int pos; // starts at 0 --> position of data buffer at
	int b_a;
	int b_b;
	protected:
		lexeme rsym(int ch); // recognize symbol
		void error(std::string str); // fatal lexing error
	public:
		Lexer(std::string raw); // Lexer constructor takes pointer to data
		~Lexer(void); // destructor
		int GetChar(void); // get a character from buffer
		void UnGetChar(void); // un-get a character from buffer
		void getsym(void); // consume and record lexeme
		void shsym(lexeme sym); // display lexeme name
		std::string strval(int a, int b); // get value from data
		lexeme cur(void); // returns current lexeme
		int st(void); // returns current b_a
		int end(void); // returns current b_b
		int cpos(void); // returns current position
};

int strlen(std::string str){ // overloaded version
	return str.length();
}

void printf(std::string str){
	logger << str;
}

Lexer::Lexer(std::string raw){
	char* tmp = (char*) malloc(raw.length()+1);
	raw.copy(tmp, raw.length(), 0); // copy entire raw data into class data
	tmp[raw.length()] = '\0';
	data = std::string(tmp); // construct std::string data with char* tmp
	pos = 0; // start pos
	b_a = b_b = 0; // on 0 right now
	logger << "Lexer class constructed and data intialized.\n";
	current = na; // not applicable yet
}

Lexer::~Lexer(void){
	logger << "Lexer class freed.\n";
}

void Lexer::error(std::string str){
	cerr << "Lexer Error: " << str << "\n";
	exit(1);
}

int Lexer::GetChar(void){
	const char* cs = data.c_str();
	return cs[pos++];
}

void Lexer::UnGetChar(void){
	pos--;
}

lexeme Lexer::rsym(int ch){
	b_a = b_b = (pos-1); // instantiate values, can be changed later
	switch(ch){
		case '+': return op_add;
		case '-': return op_sub;
		case '*': return op_mul;
		case '/': return op_div;
		case '(': return lparen;
		case ')': return rparen;
		case '=': return equals;
		case ';': return semicolon;
		case ' ': return space;
		case '"': return dquote; // double quote
		case '\'': return squote; // single quote
		case '[': return lbrack; // left bracket
		case ']': return rbrack;
		case ',': return comma;
		case '>': return op_g;
		case '<': return op_l; // still haven't added <= and >=, etc.
		case '#': return hashchar;
		case ':': return colon;
		case '&': return amp;
		case '\\': return backslash;
		case '{': return lbrace;
		case '}': return rbrace;
		default:
			if(((ch >= '0') && (ch <= '9')) || (ch == '.')){
				do {
					char ctmp = (char) ch;
					logger << "Got: |" << ctmp << "|\n";
					// consume it
					ch = GetChar();
					if(!(((((ch >= '0') && (ch <= '9')) || (ch == '.'))) && (pos < strlen(data)))) pos--; // backtrack basically
				} while(((((ch >= '0') && (ch <= '9')) || (ch == '.'))) && (pos < strlen(data)));
				b_b = (pos-1);
				return number;
			}
			if((ch >= 'a') && (ch <= 'z')){
				do {
					char ctmp = (char) ch;
					logger << "Got: |" << ctmp << "|\n";
					// consume it
					ch = GetChar();
					if(!((ch >= 'a') && (ch <= 'z') && (pos < strlen(data)))) pos--; // backtrack basically
				} while((ch >= 'a') && (ch <= 'z') && (pos < strlen(data)));
				b_b = (pos-1);
				return letters;
			} else {
				logger << "Unknown ==> |" << ch << "|\n";
				error("Unknown char.\n");
			}
	}
	return unknown;
}

void Lexer::getsym(void){
	int ch = GetChar();
	if(isupper(ch)) ch += 32; // to lower
	if(ch < 0) current = na; // neg value = n/a
	else if(iscntrl(ch)) current = na; // any control chars ('\n', '\f', '\b', etc.)
	else if(1) current = rsym(ch);
}

void Lexer::shsym(lexeme sym){ // "show" sym/token
	if(logger.lvl() <= 1) return;
	if(sym == op_add) printf("Plus\n");
	else if(sym == op_sub) printf("Minus\n");
	else if(sym == op_mul) printf("Times\n");
	else if(sym == op_div) printf("Divided By\n");
	else if(sym == number) printf("Number\n");
	else if(sym == letters) printf("Letter(s)\n");
	else if(sym == equals) printf("Equals\n");
	else if(sym == lparen) printf("Open Parenthesis\n");
	else if(sym == rparen) printf("Close Parenthesis\n");
	else if(sym == semicolon) printf("Semicolon\n");
	else if(sym == space) printf("Space\n");
	else if(sym == dquote) printf("Double Quote\n");
	else if(sym == squote) printf("Single Quote\n");
	else if(sym == lbrack) printf("Close Bracket\n");
	else if(sym == rbrack) printf("Open Bracket\n");
	else if(sym == comma) printf("Comma\n");
	else if(sym == hashchar) printf("Hash\n");
	else if(sym == unknown) printf("Unknown\n");
	else if(sym == na) printf("N/A\n");
	else if(1) printf("Unregistered symbol.\n");
}

std::string Lexer::strval(int a, int b){
	char* ret = (char*) malloc((b-a)+3);
	int i, c = 0;
	for(i = a; i <= b; i++){
		ret[c++] = data[i];
	}
	ret[c] = '\0';
	//string rt = new string; // dynamically allocated
	std::string rt = std::string(ret);
	return rt;
}

lexeme Lexer::cur(void){ // returns current lexeme
	return current;
}

int Lexer::st(void){ // returns current b_a
	return b_a;
}

int Lexer::end(void){ // returns current b_b
	return b_b;
}

int Lexer::cpos(void){ // returns current position
	return pos;
}







































