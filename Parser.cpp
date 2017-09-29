#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <queue>
#include <exception>
#include "Lexer.cpp"
#include "Stacks.cpp"
#include "Bytecode.cpp"

using namespace std;

class Parser {
	long long int bpos, rpos; // buffer and "record" positions
	int** buf; // array of int arrays
	lexeme* rec; // array of lexemes
	Lexer* lex; // my lexer
	// Registers* regs; // registers
	Stacks* stk; // stacks
	Bytecode* bc; // bytecode emitter and interpreter
	Heap* heap; // heap to simulate indexing and freeing
	int length; // length of data string
	int should_interpret; // compile-only or not
	std::map<std::string, double> jump_table; // jump table --> label => bytecode position
	std::queue<int> jump_queue; // jump queue --> push integers of jump write positions
	std::map<int, std::string> jump_label; // jump labels --> integer of jump queue position => label name
	// Note: Some jumps are to labels declared later, and must be resolved later.
	std::map<std::string, double> replace_table; // replace table --> [macro/constant name] => value
	std::map<std::string, double> heap_table; // heap table --> [heap variable name] => heap slot index
	std::map<std::string, double> heap_type; // heap type --> [heap variable name] => heap slot type
	protected:
		void Tokenize(void); // "tokenize" and fill up parse arrays
		void error(std::string str);
		int interpret_line(int i);
		void parse_func(int i, std::string cmd_name);
	public:
		Parser(std::string data, int flags); // constructor
		~Parser(void); // destructor
		void Start(void); // start "actual" parsing
};

Parser::Parser(std::string data, int flags = -1){
	if(flags != -1) should_interpret = 0;
	else should_interpret = 1;
	lex = new Lexer(data);
	// regs = new Registers();
	heap = new Heap(1);
	stk = new Stacks();
	bc = new Bytecode("code.bin");
	// rec must be array of lexemes of size strlen(data)
	rec = (lexeme*) malloc(strlen(data)*sizeof(lexeme));
	for(int i = 0; i < strlen(data); i++){
		rec[i] = unknown; // thus far, at least
	}
	// buf must be array of int arrays of size strlen(data)
	buf = (int**) malloc(strlen(data)*sizeof(int*));
	bpos = rpos = 0;
	length = strlen(data);
	logger << "Parser class instantiated with values initialized.\n";
	Tokenize();
}

Parser::~Parser(void){
	logger << "Freeing Parser Values...\n";
	for(int i = 0; i < bpos; i++){
		free(buf[i]);
	}
	free(buf);
	free(rec);
	delete bc;
	delete stk;
	delete heap;
	// delete regs;
	delete lex;
	logger << "Freed Parser Values!\n";
}

void Parser::error(std::string str){
	cerr << "Parser Error: " << str << "\n";
	exit(1);
}

void Parser::Tokenize(void){ // "tokenize" and fill up parse arrays
	int in_comment = 0, in_sstring = 0; // for string, collect all chars
	int tmp_a, tmp_b; // tmp values for 'whatever' literals
	while(lex->cpos() <= length){
		lex->getsym();
		lexeme sym = lex->cur();
		if(sym == hashchar) in_comment = 1;
		if(sym == squote) in_sstring = 1;
		if(sym == na) continue; // no need to save
		if(sym == unknown) error("Lexer has reported an unknown symbol.");
		int cur_com; // current for comment
		while(in_comment){
			if(lex->cpos() > length) error("Comment overflow.");
			cur_com = lex->GetChar();
			if(cur_com == '#'){
				lex->GetChar(); // go one forward
				in_comment = 0;
				break;
			}
		}
		tmp_a = tmp_b = lex->cpos();
		if(in_sstring){
			// quickly save that single quote
			rec[rpos++] = sym;
			int* tmp = (int*) malloc(2*sizeof(int));
			tmp[0] = lex->st();
			tmp[1] = lex->end();
			buf[bpos++] = tmp;
		}
		while(in_sstring){
			// unrecognized symbols don't matter
			// --> it goes by chars
			// it's slower though...
			int ch = lex->GetChar();
			if(ch == '\''){
				in_sstring = 0;
				lex->UnGetChar(); // allow squote to live
				tmp_b = (lex->cpos()-1); // before squote
				rec[rpos++] = letters; // word technically
				int* tmp = (int*) malloc(2*sizeof(int));
				tmp[0] = tmp_a;
				tmp[1] = tmp_b;
				buf[bpos++] = tmp;
				lex->getsym();
				break;
			}
		}
		lex->shsym(sym);
		rec[rpos++] = sym;
		int* tmp = (int*) malloc(2*sizeof(int));
		tmp[0] = lex->st();
		tmp[1] = lex->end();
		//printf("|%d| to |%d|.\n", b_a, b_b);
		buf[bpos++] = tmp;
	}
	logger << "Parser has tokenized and filled up all parse arrays.\n";
}


void Parser::parse_func(int i, std::string cmd_name){
	if(cmd_name == "lda"){
		logger << "Received LDA.\n";
		if(stk->isize() == 2){
			// lda r1, 100;
			double val = stk->ipop();
			int reg = (int) stk->ipop();
			bc->write_num(LDA, 8, (unsigned char) reg, DOUBLE);
			bc->write_double(val);
			//regs->update(reg, val);
			//regs->dump(1, 5);
		} else if((stk->isize() == 1) && (stk->csize() == 1)){
			// lda s1, 'Hello, World!';
			std::string val = stk->cpop();
			int reg = (int) stk->ipop();
			bc->write_num(LDA, (unsigned char) val.length(), (unsigned char) reg, ASCII);
			bc->write_string(val);
			//regs->update(reg, val);
			//regs->dump(-1, 5);
		} else if(true){
			error("Invalid number/type of arguments to LDA.");
		}
	} else if(cmd_name == "sto"){
		logger << "Received STO.\n";
		if(stk->isize() == 2){
			// sto 100, r1;
			int reg = (int) stk->ipop();
			double val = stk->ipop();
			bc->write_num(LDA, 8, (unsigned char) reg, DOUBLE);
			bc->write_double(val);
			//regs->update(reg, val);
			//regs->dump(1, 5);
		} else if((stk->isize() == 1) && (stk->csize() == 1)){
			// sto 'Hello, World!', s1;
			std::string val = stk->cpop();
			int reg = (int) stk->ipop();
			bc->write_num(LDA, (unsigned char) val.length(), (unsigned char) reg, ASCII);
			bc->write_string(val);
			//regs->update(reg, val);
			//regs->dump(-1, 5);
		} else if(true){
			error("Invalid number/type of arguments to STO.");
		}
	} else if(cmd_name == "ldr"){
		logger << "Received LDR.\n";
		// ldr r1, r2 means r1 = r2
		// ldr is a double register-only operation
		if(stk->isize() != 2) error("Invalid number/type of arguments to LDR.");
		int r2 = (int) stk->ipop();
		int r1 = (int) stk->ipop(); // since it's a LIFO structure
		bc->write_num(LDR, 0, (unsigned char) r1, (unsigned char) r2);
	 } else if(cmd_name == "add"){
	 	int size = stk->isize();
	 	if((size != 2) && (size != 3)) error("Invalid number/type of arguments to ADD.");
	 	if(size == 2){
			int r2 = (int) stk->ipop();
			int r1 = (int) stk->ipop();
			bc->write_num(ADD, 0, (unsigned char) r1, (unsigned char) r2);
		} else if(size == 3){
			int r3 = (int) stk->ipop();
			int r2 = (int) stk->ipop();
			int r1 = (int) stk->ipop();
			bc->write_num(LDR, 0, (unsigned char) r3, (unsigned char) r1); // load r1 into r3
			bc->write_num(ADD, 0, (unsigned char) r2, (unsigned char) r3); // add r2 to r3
		}
		// add r1, r2 --> r2 += r1
	} else if(cmd_name == "sub"){
		int size = stk->isize();
		if((size != 2) && (size != 3)) error("Invalid number/type of arguments to SUB.");
		if(size == 2){
			int r2 = (int) stk->ipop();
			int r1 = (int) stk->ipop();
			bc->write_num(SUB, 0, (unsigned char) r1, (unsigned char) r2);
		} else if(size == 3){
			int r3 = (int) stk->ipop();
			int r2 = (int) stk->ipop();
			int r1 = (int) stk->ipop();
			bc->write_num(LDR, 0, (unsigned char) r3, (unsigned char) r1); // load r1 into r3
			bc->write_num(SUB, 0, (unsigned char) r2, (unsigned char) r3); // sub r2 from r3
		}
	} else if(cmd_name == "mul"){
		int size = stk->isize();
		if((size != 2) && (size != 3)) error("Invalid number/type of arguments to MUL.");
		if(size == 2){
			int r2 = (int) stk->ipop();
			int r1 = (int) stk->ipop();
			bc->write_num(MUL, 0, (unsigned char) r1, (unsigned char) r2);
		} else if(size == 3){
			int r3 = (int) stk->ipop();
			int r2 = (int) stk->ipop();
			int r1 = (int) stk->ipop();
			bc->write_num(LDR, 0, (unsigned char) r3, (unsigned char) r1); // load r1 into r3
			bc->write_num(MUL, 0, (unsigned char) r2, (unsigned char) r3); // mul r2 with r3
		}
	} else if(cmd_name == "div"){
		int size = stk->isize();
		if((size != 2) && (size != 3)) error("Invalid number/type of arguments to DIV.");
		if(size == 2){
			int r2 = (int) stk->ipop();
			int r1 = (int) stk->ipop();
			bc->write_num(DIV, 0, (unsigned char) r1, (unsigned char) r2);
		} else if(size == 3){
			int r3 = (int) stk->ipop();
			int r2 = (int) stk->ipop();
			int r1 = (int) stk->ipop();
			bc->write_num(LDR, 0, (unsigned char) r3, (unsigned char) r1); // load r1 into r3
			bc->write_num(DIV, 0, (unsigned char) r2, (unsigned char) r3); // div r2 by r3
		}
	} else if(cmd_name == "print"){ // print double register value
		if(stk->isize() != 1) error("Invalid number/type of arguments to PRINT.");
		int reg = (int) stk->ipop();
		bc->write_num(PRT, 0, (unsigned char) reg, 0);
	} else if(cmd_name == "sprint"){ // print string register value
		if(stk->isize() != 1) error("Invalid number/type of arguments to SPRINT.");
		int reg = (int) stk->ipop();
		bc->write_num(SPT, 0, (unsigned char) reg, 0);
	} else if(cmd_name == "dprint"){ // print string from arguments
		if(stk->csize() != 1) error("Invalid number/type of arguments to DPRINT.");
		std::string val = stk->cpop();
		bc->write_num(DPT, (unsigned char) val.length(), 0, ASCII);
		bc->write_string(val);
	} else if((cmd_name == "jmp") || (cmd_name == "je") || (cmd_name == "jg") || (cmd_name == "jl") || (cmd_name == "jne") || (cmd_name == "jnl") || (cmd_name == "jng")){ // jumps
		// Have to add support for "forward jumps"
		// --> label is later in code
		// --> may need to add "labeling" as a preprocessing stage?
		// --> or resolve "deltas" and binary address post-bytecode generation.
		if(stk->csize() != 1) error("Invalid number/type of arguments to JUMP.");
		std::string val = stk->cpop();
		std::map<std::string, double>::const_iterator it = jump_table.find(val);
		bool exists = (it != jump_table.end());
		double lbl_pos;
		if(!exists){ 
			// error("Jump label does not exist.");
			lbl_pos = -1000;
		} else {
			lbl_pos = jump_table[val]; // since it does exist
		}
		if(cmd_name == "jmp") bc->write_num(JMP, (unsigned char) sizeof(double), 0, DOUBLE);
		else if(cmd_name == "je") bc->write_num(JPE, (unsigned char) sizeof(double), 0, DOUBLE);
		else if(cmd_name == "jg") bc->write_num(JPG, (unsigned char) sizeof(double), 0, DOUBLE);
		else if(cmd_name == "jl") bc->write_num(JPL, (unsigned char) sizeof(double), 0, DOUBLE);
		else if(cmd_name == "jne") bc->write_num(JNE, (unsigned char) sizeof(double), 0, DOUBLE);
		else if(cmd_name == "jnl") bc->write_num(JNL, (unsigned char) sizeof(double), 0, DOUBLE);
		else if(cmd_name == "jng") bc->write_num(JNG, (unsigned char) sizeof(double), 0, DOUBLE);
		if(!exists){
			int pos = bc->wpos();
			jump_queue.push(pos); // push position of start of the <double> jump offset
			jump_label[pos] = val;
			logger << "Pushed possible forward jump to post-processing queue.\n";
		}
		bc->write_double(lbl_pos);
	} else if(cmd_name == "label"){
		if(stk->csize() != 1) error("Invalid number/type of arguments to LABEL.");
		std::string val = stk->cpop();
		jump_table[val] = (double) bc->wisp();
	} else if(cmd_name == "cmp"){
		if(stk->isize() != 2) error("Invalid number/type of arguments to CMP.");
		int r2 = (int) stk->ipop();
		int r1 = (int) stk->ipop();
		bc->write_num(CMP, 0, (unsigned char) r1, (unsigned char) r2);
	} else if((cmd_name == "bl") || (cmd_name == "bg") || (cmd_name == "be") || (cmd_name == "bnl") || (cmd_name == "bng") || (cmd_name == "bne")){ // branches/conditional jumps
		if((stk->isize() != 2) || (stk->csize() != 1)) error("Invalid number/type of arguments to BRANCH.");
		int r2 = (int) stk->ipop();
		int r1 = (int) stk->ipop();
		unsigned char v1 = (unsigned char) r1;
		unsigned char v2 = (unsigned char) r2;
		std::string val = stk->cpop();
		std::map<std::string, double>::const_iterator it = jump_table.find(val);
		bool exists = (it != jump_table.end());
		double lbl_pos;
		if(!exists){ 
			lbl_pos = -1000;
		} else {
			lbl_pos = jump_table[val]; // if it does exist
		}
		if(cmd_name == "bl") bc->write_num(BPL, (unsigned char) sizeof(double), v1, v2);
		else if(cmd_name == "bg") bc->write_num(BPG, (unsigned char) sizeof(double), v1, v2);
		else if(cmd_name == "be") bc->write_num(BPE, (unsigned char) sizeof(double), v1, v2);
		else if(cmd_name == "bnl") bc->write_num(BNL, (unsigned char) sizeof(double), v1, v2);
		else if(cmd_name == "bne") bc->write_num(BNE, (unsigned char) sizeof(double), v1, v2);
		else if(cmd_name == "bng") bc->write_num(BNG, (unsigned char) sizeof(double), v1, v2);
		if(!exists){
			int pos = bc->wpos();
			jump_queue.push(pos); // push position of start of the <double> jump offset
			jump_label[pos] = val;
			logger << "Pushed possible forward branch to post-processing queue.\n";
		}
		bc->write_double(lbl_pos);
	} else if((cmd_name == "define") || (cmd_name == "def")){
		if((stk->isize() != 1) || (stk->csize() != 1)) error("Invalid number/type of arguments to BRANCH.");
		std::string val = stk->cpop();
		double fval = stk->ipop();
		replace_table[val] = fval;
		logger << "Defined constant '" << val << "'.\n";
	} else if(cmd_name == "gets"){
		if(stk->isize() != 1) error("Invalid number/type of arguments to GETS.");
		unsigned char reg = (unsigned char) stk->ipop();
		bc->write_num(GTS, 0, reg, 0);
	} else if(cmd_name == "atof"){
		if(stk->isize() != 2) error("Invalid number/type of arguments to ATOF.");
		unsigned char nreg = (unsigned char) stk->ipop();
		unsigned char sreg = (unsigned char) stk->ipop();
		bc->write_num(ATF, 0, sreg, nreg);
	} else if(cmd_name == "sheapalloc"){ // string heap alloc
		if(stk->csize() != 1) error("Invalid number/type of arguments to SHEAPALLOC.");
		std::string val = stk->cpop();
		heap_table[val] = heap->sreg();
		heap_type[val] = 2;
		bc->write_num(HAL, 0, 2, 0); // already have heap index simulated
	} else if(cmd_name == "heapalloc"){ // double heap alloc
		if(stk->csize() != 1) error("Invalid number/type of arguments to HEAPALLOC.");
		std::string val = stk->cpop();
		heap_table[val] = heap->nreg();
		heap_type[val] = 1;
		bc->write_num(HAL, 0, 1, 0); // type (1 = double, 2 = string)
	} else if(cmd_name == "hld"){
		/*
		#define HLD 0x37 // load heap value into register (heap slot index)
#define HPS 0x38 // restore heap value from register (given heap slot index)
		*/
		if(stk->isize() != 2) error("Invalid number/type of arguments to HLD.");
		// hld hX, rY;
		int reg = stk->ipop();
		int hp = -(stk->ipop());
		bc->write_num(HLD, 0, (unsigned char) hp, (unsigned char) reg);
	} else if(cmd_name == "hps"){
		if(stk->isize() != 2) error("Invalid number/type of arguments to HLD.");
		// hps rX, hY;
		int hp = -(stk->ipop());
		int reg = stk->ipop();
		bc->write_num(HPS, 0, (unsigned char) reg, (unsigned char) hp);
	} else if(cmd_name == "hsto"){
		if(stk->isize() != 2) error("Invalid number/type of arguments to HSTO.");
		// hsto 10, h2;
		int hp = -(stk->ipop());
		double reg = stk->ipop();
		bc->write_num(HST, 8, (unsigned char) hp, DOUBLE);
		bc->write_double(reg);
	} else if((cmd_name == "function") || (cmd_name == "func")){
		if(stk->csize() != 1) error("Invalid number/type of arguments to FUNC.");
		std::string val = stk->cpop();
		bc->write_num(FNC, (unsigned char) val.length(), 0, ASCII);
		bc->write_string(val);
	} else if(cmd_name == "end"){
		bc->write_num(END, 0, 0, 0);
	} else if((cmd_name == "ret") || (cmd_name == "return")){
		// for now, no return value
		bc->write_num(RET, 0, 0, 0);
	} else if(cmd_name == "call"){
		if(stk->csize() != 1) error("Invalid number/type of arguments to CALL.");
		std::string val = stk->cpop();
		bc->write_num(CAL, (unsigned char) val.length(), 0, ASCII);
		bc->write_string(val);
	} else if(cmd_name == "push"){
		if(stk->isize() != 1) error("Invalid number/type of arguments to PUSH.");
		double val = stk->ipop();
		bc->write_num(PSH, 8, 0, DOUBLE);
		bc->write_double(val);
	} else if(cmd_name == "pop"){
		if(stk->isize() != 1) error("Invalid number/type of arguments to POP.");
		int val = (int) stk->ipop();
		bc->write_num(POP, 8, (unsigned char) val, 0);
	}
}

int Parser::interpret_line(int i){ // takes instruction pointer location, returns new one
	int in_cmd = 0; // should remove later
	std::string cmd_name;
	cmd_name.reserve(128);
	logger << "\n";
	while(1){
		if(i >= bpos) break;
		int* tmp = buf[i];
		int st = tmp[0], end = tmp[1];
		lexeme cur = rec[i];
		lexeme next = ((i+1) >= bpos) ? na : rec[(i+1)];
		lexeme prev = ((i-1) >= bpos) ? na : rec[(i-1)];
		if((cur == letters) && ((next == space) || (next == semicolon)) && (!in_cmd)){ // sto 1, r2;
			std::string stmp = lex->strval(st, end);
			cmd_name = stmp;
			logger << "Parsing Command |" << cmd_name << "|.\n";
			in_cmd = 1;
		} else if((cur == letters) && (next == number) && (in_cmd)){ // r1 or r2 or r0, etc.
			double fval = (double) atof((lex->strval(buf[(i+1)][0], buf[(i+1)][1])).c_str());
			if(!fval) error("Register #0 is reserved for virtual machine use.");
			if(fval >= MAX_REGS) error("Register Overflow.");
			stk->push(fval);
			logger << "Pushed register number |" << fval << "|.\n";
		} else if((cur == number) && ((next == comma) || (next == semicolon)) && (in_cmd) && (prev != letters)){
			double fval = (double) atof((lex->strval(st, end)).c_str());
			stk->push(fval);
			logger << "Pushed argument |" << std::dec << std::setprecision(16) << fval << "|.\n";
		} else if(((prev == squote) || (prev == dquote)) && (cur == letters) && ((next == squote) || (next == dquote)) && (in_cmd)){
			// char* cw = char_arr[char_arr_c];
			std::string cr = lex->strval(st, end);
			stk->push(cr);
			logger << "ASCII string |" << cr << "| pushed.\n";
		} else if((cur == letters) && ((next == comma) || (next == semicolon)) && (prev == space)){
			// eax, ebx, ecx, NUM, MAX_X, etc.
			// logger << "Virtual register |" << lex->strval(st, end) << "|.\n";
			std::string val = lex->strval(st, end);
			std::map<std::string, double>::const_iterator it = replace_table.find(val);
			std::map<std::string, double>::const_iterator it2 = heap_table.find(val);
			bool exists = (it != replace_table.end());
			bool exists2 = (it2 != heap_table.end());
			if((!exists) && (!exists2)) error("Invalid constant/heap variable name '"+val+"'.");
			if((exists) && (exists2)) error("Constant name '"+val+"' conflicts with heap.");
			if(exists){
				double fval = replace_table[val];
				stk->push(fval);
				logger << "Pushed constant argument |" << std::dec << std::setprecision(16) << fval << "|.\n";
			} else if(exists2){
				double fval = heap_table[val];
				fval *= -1; // negative indicates heap value
				// heap register will never be 0
				stk->push(fval);
				logger << "Pushed heap index value |" << std::dec << std::setprecision(16) << fval << "|.\n";
			}
		} else if(cur == semicolon){
			parse_func(i, cmd_name);
			logger << "\n";
			stk->clean();
			i++;
			in_cmd = 0;
			break;
		}
		i++;
	}
	return i;
}

void Parser::Start(void){
	int i = 0; // instruction pointer at this scope
	while(i < bpos){
		i = interpret_line(i);
	}
	/*
	int tmp = bc->wpos();
	int tst = 100;
	bc->rewind_to_pos(tmp-tst);
	for(int i = 0; i < tst; i++) bc->write_num(LDR, 8, 9, 2);
	bc->rewind_to_pos(tmp);*/
	int end = bc->wpos(); // end position of file write pointer
	logger << "Resolving post-processing deltas...\n";
	int rsv = 0;
	while(!jump_queue.empty()){
		rsv++;
		int pos = jump_queue.front();
		jump_queue.pop();
		std::string lbl = jump_label[pos];
		std::map<std::string, double>::const_iterator it = jump_table.find(lbl);
		bool exists = (it != jump_table.end());
		if(!exists) error("Jump label '"+lbl+"' does not exist.");
		double lbl_pos = jump_table[lbl];
		bc->rewind_to_pos(pos);
		bc->write_double(lbl_pos);
		bc->rewind_to_pos(end);
	}
	if(rsv == 1) logger << "Resolved 1 delta.\n";
	else if(rsv > 1) logger << "Resolved " << rsv << " deltas.\n";
	else if(true) logger << "No deltas detected to resolve.\n";
	logger << "\nEnd of parsing.\nCode compiled into binary bytecode.\n\n";
	if(should_interpret){
		bc->init_read();
		bc->Start();
	}
}






























































