#include <iostream>
#include <iomanip> // I/O manipulators
#include <fstream> // file I/O streams
#include <string>
#include <sstream>
#include <exception> // an exceptional class... hehe
#include <cstdio>
#include "Instructions.h"
#include "Heap.cpp"
#include "Registers.cpp"
#include <unistd.h>

// macro for branch instructions
#define BRNCH(typem, condm) \
	logger << typem " r" << (int)(v1) << ", r" << (int)(v2) << ";\n"; \
	regs->cmp((int)(v1), (int)(v2), comp_e, comp_l, comp_g); \
	logger << "Compared "; \
	if(comp_e) logger << "Equal."; \
	else if(comp_l) logger << "Less."; \
	else if(comp_g) logger << "Greater."; \
	else if(true) error("Broken compare instruction."); \
	logger << "\n"; \
	if(condm){ \
		double tmp_jmp = to_double(&bc[(isp+1)]); \
		logger << "BRANCH JUMP " << std::dec << std::setprecision(16) << tmp_jmp << ";\n"; \
		isp = (tmp_jmp-1); \
		logger << "Press [Enter] To Continue.\n"; \
		if(logger.lvl() > 1) getchar(); \
	} else logger << "Branch conditions not met.\n\n";

using namespace std;

class Bytecode {
	ofstream fp;
	ifstream ifp;
	int fsize; // file length
	int* bc; // bytecode array
	int isp; // instruction pointer
	int max_isp;
	int write_isp; // byte number for writing --> used for jumps
	// start comparisons
	int comp_e; // equal
	int comp_g; // greater than
	int comp_l; // less than
	// end comparisons
	std::string fname;
	Registers* regs; // registers
	Heap* heap; // heap
	Stacks* stk; // stack
	std::map<std::string, int> func_pos; // [function name] => ISP pos
	std::stack<double> user_stack;
	protected:
		void error(std::string str);
	public:
		Bytecode(std::string name, int flags);
		~Bytecode(void);
		void init_read(void);
		unsigned char get_byte(int val, int num);
		unsigned char get_byte(double val, int num);
		void alter_byte(int& val, int num, unsigned char bt);
		void alter_byte(double& val, int num, unsigned char bt);
		int create_num(unsigned char instr, unsigned char after, unsigned char v1, unsigned char v2);
		void write_num(unsigned char instr, unsigned char after, unsigned char v1, unsigned char v2);
		int* create_double(double val);
		void write_double(double val);
		double to_double(int* dbl);
		int* create_string(std::string val);
		void write_string(std::string str);
		std::string to_string(int* str);
		int wisp(void); // get write_isp value
		int wpos(void); // get write position
		void rewind_to_pos(int isp); // take write isp and seek to that
		void decode_instr(int code);
		void Start(void);
};

Bytecode::Bytecode(std::string name, int flags = -1){
	fname = name;
	if(flags == -1){
		::unlink(fname.c_str());
		fp.open(fname.c_str(), ios::out | ios::binary);
		if(!fp){
			cerr << "Bytecode Error: Unable to open file for writing.\n";
			exit(1);
		}
	}
	write_isp = 0;
	logger << "Bytecode class intialized with binary file.\n";
	/*
	fp.close();
	*/
	/*
	int ct = 0;
	while(!ifp.eof()){ 
		ifp.read((char*)(&(arr[ct++])), sizeof(int));
	}
	for(int i = 0; i < ct; i++){
		decode_instr(arr[i]);
	}
	*/
}

Bytecode::~Bytecode(void){
	if(fp.is_open()) fp.close();
	if(ifp.is_open()) ifp.close();
	delete[] bc;
	delete stk;
	delete regs;
	delete heap;
	logger << "Bytecode class freed.\n";
}

void Bytecode::init_read(void){
	if(fp.is_open()) fp.close(); // no writing once reading --> no two files open at the same time
	ifp.open(fname.c_str(), ios::in | ios::binary);
	if(!ifp){
		cerr << "Bytecode Error: Unable to open file for reading.\n";
		exit(1);
	}
	ifp.seekg(0, ifp.end); // to end
	int len = ifp.tellg();
	fsize = len;
	ifp.seekg(0, ifp.beg); // to start
	bc = new int[(len/4)];
	isp = 0;
	int ct = 0;
	while(!ifp.eof()){ 
		ifp.read((char*)(&(bc[ct++])), sizeof(int));
	}
	max_isp = ct;
	heap = new Heap();
	regs = new Registers(heap);
	stk = new Stacks();
	logger << "\nBytecode class initialized for reading and interpreting.\n";
}

void Bytecode::error(std::string str){
	cerr << "Bytecode Error: " << str << "\n";
	exit(1);
}

unsigned char Bytecode::get_byte(int val, int num){
	// num = 0 -> 3
	if((num < 0) || (num >= sizeof(int))) return 0;
	unsigned char* b = (unsigned char*) &val;
	int offset = (int)(sizeof(val)-num-1); // since it's little-endian
	return *(b+offset);
}

unsigned char Bytecode::get_byte(double val, int num){ // overloaded method
	// num = 0 -> 7
	if((num < 0) || (num >= sizeof(double))) return 0;
	unsigned char* b = (unsigned char*) &val;
	int offset = (int)(sizeof(double)-num-1); // since it's little-endian
	return *(b+offset);
}

void Bytecode::alter_byte(int& val, int num, unsigned char bt){
	if((num < 0) || (num >= sizeof(int))) return;
	unsigned char* b = (unsigned char*) &val;
	int offset = (int)(sizeof(val)-num-1); // since it's little-endian
	*(b+offset) = bt;
}

void Bytecode::alter_byte(double& val, int num, unsigned char bt){
	if((num < 0) || (num >= sizeof(double))) return;
	unsigned char* b = (unsigned char*) &val;
	int offset = (int)(sizeof(val)-num-1); // since it's little-endian
	*(b+offset) = bt;
}

int Bytecode::create_num(unsigned char instr, unsigned char after, unsigned char v1, unsigned char v2){
	// instr = instruction
	// after = how many of these 4-byte sets after this are used (i.e. length of a string right after this)
	// v1, v2 can be registers
	// or:
	// v1 can be a single byte of a double or an ascii val
	// and v2 can be a "type" byte --> is it a double, ascii, etc.
	// v1 and v2's operation, content, and purpose is determined by instruction
	// i.e. ldr r1, r2 has v1 and v2 of r1 and r2 respectively.
	// but lda r1, 100 has v1 of register 1, but "after" specifies sizeof(double) in bytes
	int ret = 0;
	alter_byte(ret, 0, instr);
	alter_byte(ret, 1, after);
	alter_byte(ret, 2, v1);
	alter_byte(ret, 3, v2);
	// logger << std::hex << ret << ", " << std::dec << ret << "\n";
	return ret;
}

void Bytecode::write_num(unsigned char instr, unsigned char after, unsigned char v1, unsigned char v2){
	int code = create_num(instr, after, v1, v2);
	fp.write((char*)&code, sizeof(int));
	write_isp++;
}

int* Bytecode::create_double(double val){
	const size_t dbl = sizeof(double);
	int* ret = new int[dbl];
	// each int is a byte specifying a double val
	for(int i = 0; i < sizeof(double); i++){
		unsigned char v1 = get_byte(val, i);
		unsigned char after = (unsigned char)(sizeof(double)-i-1);
		ret[i] = create_num(VAL, after, v1, DOUBLE);
	}
	return ret;
}

void Bytecode::write_double(double val){
	int* dbl = create_double(val);
	for(int i = 0; i < sizeof(double); i++){
		write_num(get_byte(dbl[i], 0), get_byte(dbl[i], 1), get_byte(dbl[i], 2), get_byte(dbl[i], 3));
	}
}

double Bytecode::to_double(int* dbl){
	double ret;
	//unsigned char* r = (unsigned char*) &ret;
	for(int i = 0; i < sizeof(double); i++){
		//unsigned char v1 = get_byte(dbl[i], 2); // 3rd byte
		alter_byte(ret, i, get_byte(dbl[i], 2));
	}
	return ret;
}

int* Bytecode::create_string(std::string val){
	int* ret = new int[val.length()];
	for(int i = 0; i < val.length(); i++){
		unsigned char v1 = val.at(i);
		unsigned char after = (unsigned char)(val.length()-i-1);
		ret[i] = create_num(VAL, after, v1, ASCII);
	}
	return ret;
}

void Bytecode::write_string(std::string str){
	int* rst = create_string(str);
	for(int i = 0; i < str.length(); i++){
		write_num(get_byte(rst[i], 0), get_byte(rst[i], 1), get_byte(rst[i], 2), get_byte(rst[i], 3));
	}
	delete[] rst;
}

std::string Bytecode::to_string(int* str){
	// get length as "after" byte of str[0] + 1
	int length = ((int) get_byte(str[0], 1)) + 1; // "after" byte for 1st one plus 1
	std::string ret;
	ret.reserve((length+1));
	for(int i = 0; i < length; i++){
		ret.push_back(get_byte(str[i], 2));
	}
	char* cr = strdup(ret.c_str());
	int done = 0;
	if(strlen(cr) < 2) done = 1;
	else if(strlen(cr) == 2){
		if((cr[0] == '\\') && (cr[1] == 'n')){
			cr[0] = '\n';
			cr[1] = 0;
		}
		done = 1;
	}
	while(!done){
		for(int i = 1; i < strlen(cr); i++){
			if((cr[(i-1)] == '\\') && (cr[i] == 'n')){
				cr[(i-1)] = '\n';
				// shift everything over left including '\0'
				for(int j = (i+1); j <= strlen(cr); j++){
					cr[(j-1)] = cr[j];
				}
				break;
			} else if(i == (strlen(cr)-1)){ // reached end without '\n'
				done = 1;
				break;
			}
		}
	}
	ret = std::string(cr);
	free(cr);
	return ret;
}

int Bytecode::wisp(void){
	return write_isp;
}

void Bytecode::rewind_to_pos(int pos){
	// int offset = (isp*4);
	fp.seekp(pos);
}

int Bytecode::wpos(void){
	return fp.tellp();
}

void Bytecode::decode_instr(int code){
	unsigned char instr = get_byte(code, 0); // instr byte
	unsigned char after = get_byte(code, 1);
	unsigned char v1 = get_byte(code, 2);
	unsigned char v2 = get_byte(code, 3);
	static void* jump_table[] = {
		&&do_halt, &&do_lda, &&do_sto, &&do_ldr, &&do_val,
		&&do_add, &&do_sub, &&do_mul, &&do_div, &&do_prt,
		&&do_spt, &&do_dpt, &&do_jmp, &&do_cmp, &&do_jpe,
		&&do_jpg, &&do_jpl, &&do_jne, &&do_jng, &&do_jnl,
		&&do_bpe, &&do_bpg, &&do_bpl, &&do_bne, &&do_bng,
		&&do_bnl, &&do_na, &&do_na, &&do_na, &&do_na,
		&&do_na, &&do_na, &&do_na, &&do_na, &&do_na,
		&&do_na, &&do_na, &&do_na, &&do_na, &&do_na,
		&&do_na, &&do_na, &&do_na, &&do_na, &&do_na, 
		&&do_na, &&do_na, &&do_na, &&do_na, &&do_na,
		&&do_gts, &&do_atf, &&do_na, &&do_hal, &&do_na,
		&&do_hld, &&do_hps, &&do_hst, &&do_fnc, &&do_end,
		&&do_ret, &&do_cal, &&do_psh, &&do_pop
	};
	// unsigned char next = get_byte(code, 0);
	goto *jump_table[instr];
	do_halt:
		logger << "HLT;\n";
		// logger << "Halting VM...\n";
		logger << "End of interpretation detected.\n\n";
		// exit(0); // later, call some global destructor method
		isp = max_isp; // make sure that interpretation will end
		return;
	do_lda:
		logger << "LDA r" << (int)(v1);
		if(v2 == DOUBLE){
			double val = to_double(&bc[(isp+1)]);
			isp += (int) after; // skip the double val
			logger << ", " << std::dec << std::setprecision(16) << val << ";\n";
			regs->update((int)(v1), val);
			regs->dump(1, 5);
		} else if(v2 == ASCII){
			std::string val = to_string(&bc[(isp+1)]);
			isp += (int) after; // skip string
			logger  << ", '" << val << "';\n";
			regs->update((int)(v1), val);
			regs->dump(-1, 5);
		}
		logger << "\n";
		return;
	do_sto:
		// technically the parser should have taken care of this...
		logger << "STO (val), r" << (int)(v1) << ";\n";
		return;
	do_ldr:
		logger << "LDR r" << (int)(v1) << ", r" << (int)(v2) << ";\n";
		regs->copy((int)(v1), (int)(v2), 1);
		regs->dump(1, 5);
		logger << "\n";
		//printf("|%d| |%d|.\n", v1, v2);
		return;
	do_val:
		// ignore val bytes
		return;
	do_add:
		// add r1, r2
		logger << "ADD r" << (int)(v1) << ", r" << (int)(v2) << ";\n";
		regs->add((int)(v1), (int)(v2));
		regs->dump(1, 5);
		logger << "\n";
		return;
	do_sub:
		// add r1, r2
		logger << "SUB r" << (int)(v1) << ", r" << (int)(v2) << ";\n";
		regs->sub((int)(v1), (int)(v2));
		regs->dump(1, 5);
		logger << "\n";
		return;
	do_mul:
		// add r1, r2
		logger << "MUL r" << (int)(v1) << ", r" << (int)(v2) << ";\n";
		regs->mul((int)(v1), (int)(v2));
		regs->dump(1, 5);
		logger << "\n";
		return;
	do_div:
		// add r1, r2
		logger << "DIV r" << (int)(v1) << ", r" << (int)(v2) << ";\n";
		regs->div((int)(v1), (int)(v2));
		regs->dump(1, 5);
		logger << "\n";
		return;
	do_prt:
		// print r1
		logger << "PRINT r" << (int)(v1) << ";\n";
		regs->print((int)(v1));
		logger << "\n";
		return;
	do_spt:
		// print r1
		logger << "SPRINT r" << (int)(v1) << ";\n";
		regs->print((int)(v1), 2);
		logger << "\n";
		return;
	do_dpt:
		// dprint 'test';
		if(v2 == ASCII){
			std::string tmp_dpt = to_string(&bc[(isp+1)]);
			isp += (int) after; // skip string
			logger << "DPRINT '" << tmp_dpt << "';\n";
			logger << "Printing: |";
			cout << tmp_dpt;
			logger << "|.\n\n";
		} else cerr << "Invalid DPRINT.\n";
		return;
	do_jmp:
		// jump (isp) from bytecode
		// (isp) given by double value
		if(v2 == DOUBLE){
			double tmp_jmp = to_double(&bc[(isp+1)]);
			logger << "JUMP " << std::dec << std::setprecision(16) << tmp_jmp << ";\n";
			isp = (tmp_jmp-1); // minus one since incremented in loop
			logger << "Press [Enter] To Continue.\n";
			if(logger.lvl() > 1) getchar();
		} else cerr << "Invalid JUMP.\n";
		return;
	do_cmp:
		// compare two registers and set flags accordingly
		logger << "CMP r" << (int)(v1) << ", r" << (int)(v2) << ";\n";
		regs->cmp((int)(v1), (int)(v2), comp_e, comp_l, comp_g);
		logger << "Compared ";
		if(comp_e) logger << "Equal.";
		else if(comp_l) logger << "Less.";
		else if(comp_g) logger << "Greater.";
		else if(true) error("Broken compare instruction.");
		logger << "\n\n";
		return;
	do_jpe:
		// jump if equal
		if(!comp_e) return;
		goto do_jmp;
	do_jpg:
		if(!comp_g) return;
		goto do_jmp;
	do_jpl:
		if(!comp_l) return;
		goto do_jmp;
	do_jne:
		if(comp_e) return;
		goto do_jmp;
	do_jng:
		if(comp_g) return;
		goto do_jmp;
	do_jnl:
		if(comp_l) return;
		goto do_jmp;
	/*
	#define BPE 0x14 // branch if equal
#define BPG 0x15 // if greater
#define BPL 0x16 // less
#define BNE 0x17 // neq
#define BNG 0x18 // ng
#define BNL 0x19 // nl
	*/
	do_bpe:
		BRNCH("BE", comp_e) // an awesome macro I made!
		return;
	do_bpg:
		BRNCH("BG", comp_g)
		return;
	do_bpl:
		BRNCH("BL", comp_l)
		return;
	do_bne:
		BRNCH("BNE", !comp_e)
		return;
	do_bng:
		BRNCH("BNG", !comp_g)
		return;
	do_bnl:
		BRNCH("BNL", !comp_l)
		return;
	do_na:
		logger << "Operation not implemented yet.\n\n";
		return;
	do_gts:
		logger << "GTS s" << (int)(v1) << ";\n";
		if(v1 > 0){
			std::string in;
			getline(cin, in);
			regs->update((int)(v1), in);
		} else error("Invalid GTS register.");
		return;
	do_atf:
		logger << "ATOF s" << (int)(v1) << ", r" << (int)(v2) << ";\n";
		regs->update((int)(v2), atof((regs->cval((int)(v1))).c_str()));
		return;
	do_hal:
		logger << "HAL;\n\n";
		if(v1 == 2){
			heap->sreg(); // heap indices already predicted by simulated heap in parser
		} else if(v1 == 1){
			heap->nreg();
		} else if(true) error("Unknown heap allocation type.");
		return;
	do_hld: // only doubles for now
		logger << "HLD h" << (int)(v1) << ", r" << (int)(v2) << ";\n";
		regs->update((int)(v2), (*heap)(v1));
		regs->dump(1, 5);
		logger << "\n";
		return;
	do_hps: // only doubles for now
		logger << "HPS r" << (int)(v1) << ", h" << (int)(v2) << ";\n";
		(*heap)(v2) = regs->nval((int)(v1));
		regs->dump(1, 5);
		logger << "\n";
		return;
	do_hst: // only doubles for now
		logger << "HPS r" << (int)(v1) << ";\n";
		if(v2 == DOUBLE){
			double val = to_double(&bc[(isp+1)]);
			(*heap)(v1) = val;
		} else error("Invalid arguments to HPS.");
		regs->dump(1, 5);
		logger << "\n";
		return;
	/*
	#define FNC 0x3A // function 'test' for example
#define END 0x3B // end of function
#define RET 0x3C // a "premature" end --> return (a value)
	*/
	do_fnc:
		logger << "FUNC ";
		if(v2 == ASCII){
			std::string val = to_string(&bc[(isp+1)]);
			logger << "'"+val+"';\n";
			// start skipping until end instruction
			logger << "Recording position...\n";
			func_pos[val] = isp; // save instruction pointer to code after function
			logger << "Starting skip...\n";
			while(get_byte(bc[isp++], 0) != END){
				if(isp == max_isp) error("Function overflow - no end.");
			}
			logger << "Skipped!\n\n";
			// since postfix increment, isp is already one past END instruction
			isp--; // because it is incremented in loop later
		} else error("Invalid arguments to FUNC.");
		return;
	do_end:
		logger << "END;\n\n";
		if(!v2){
			int pos = stk->fpop();
			int ret = stk->rpop();
			isp = pos; // since incremented later in loop
		} else error("Malformed END instruction.");
		return;
	do_ret:
		logger << "RET;\n\n";
		if(!v2){ // no return value for now
			int pos = stk->fpop();
			int ret = stk->rpop();
			isp = pos; // since incremented later in loop
		} else error("Malformed RET instruction.");
		return;
	do_cal: // a function call
		logger << "CALL ";
		if(v2 == ASCII){
			std::string val = to_string(&bc[(isp+1)]);
			logger << "'"+val+"';\n";
			std::map<std::string, int>::const_iterator it = func_pos.find(val);
			bool exists = (it != func_pos.end());
			if(!exists) error("Function '"+val+"' does not exist.");
			// start skipping until end instruction
			logger << "Saving position...\n";
			stk->fpush(isp); // save instruction pointer to function call
			stk->rpush(-1); // temporary --> should have return register number later
			logger << "Calling function '"+val+"'...\n\n";
			isp = func_pos[val];
		} else error("Invalid arguments to CALL.");
		return;
	do_psh:
		logger << "PUSH ";
		if(v2 == DOUBLE){
			double val = to_double(&bc[(isp+1)]);
			logger << std::dec << std::setprecision(16) << val << ";\n\n";
			user_stack.push(val);
		} else error("Invalid arguments to PUSH.");
		return;
	do_pop:
		logger << "POP r" << (int)(v1) << ";\n";
		if(!v2){
			regs->update((int)(v1), user_stack.top());
			regs->dump(1, 5);
			user_stack.pop();
			logger << "\n";
		} else error("Invalid arguments to POP.");
}

void Bytecode::Start(void){
	logger << "Bytecode class started interpretation.\n\n";
	while(isp < max_isp){
		decode_instr(bc[isp]);
		isp++;
	}
	logger << "Bytecode class ended interpretation.\n";
}

/*
int main(void){
	Bytecode* bt = new Bytecode("code.bin");
	bt->write_num(LDR, 0, 1, 2);
	bt->write_num(LDA, 8, 1, 0);
	bt->write_num(STO, 8, 12, 7);
	bt->write_num(HLT, 0, 0, 0);
	bt->init_read();
	bt->Start();
	delete bt;
	return 0;
}
*/































