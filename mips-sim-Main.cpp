//2020315232 Shakhzod
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <cstring>
#include <string>
#include <fstream>
#include <cmath>
#include <cstdio>
using namespace std;

const int SIZE = 256;
char chr[SIZE*SIZE][SIZE] = { "" };
unsigned char* elm;
int length;
int PC;
int numofInst;
bool loop = false;
bool flag = true;
int arrayNum;
int InstLen = 0;

struct Reg{
	int value = 0;
	string id = "";
}reg[32];

struct Instr {
	string opCode = "";
	string instr_b = "";
	int crnAddr = 0;
	char instr_x[10] = { "" }; // to store one instruction in binary
	char sub_instr[30] = { "" }; // to store one substring of instructuion
	int rd = 0;
	int rt = 0;
	int rs = 0;
	int immd = 0;
}; Instr* instr = nullptr;

struct Memory {
	char hexMem[10] = { "" };
	unsigned int value = 0;
}; Memory *memory = nullptr;

string hexToBin(char* sHex);

void rmvSpc(char chr[]) {
	int cnt = 0;
	for (int i = 0; chr[i]; i++)
		if (chr[i] != ' ')
			chr[cnt++] = chr[i];
	chr[cnt] = '\0';
}

void inverse(char c[]) {
	int i;
	for (i = 0; i < strlen(c); i++) {
		if (c[i] == '1')
			c[i] = '0';
		else
			c[i] = '1';
	}
}

void hexDump(int offset, void* PC, int len) {
	int i;
	elm = (unsigned char*)PC;
	length = len;
	for (i = 0; i < len; i++) {
		if ((i % 16) == 0)
			offset += (i % 16 == 0) ? 16 : i % 16;
	}
}

void rType() {
	bool unsign = false;
	bool shift = false;
	bool nop = false;
	bool unknown = false;
	loop = false;

	if ((instr[PC].opCode == "") && (PC < InstLen)) {
		strncpy(instr[PC].sub_instr, "", 30);

		strncpy(instr[PC].sub_instr, instr[PC].instr_b.c_str() + 26, 6);
		if (!(strcmp(instr[PC].sub_instr, "100000")))
			instr[PC].opCode = "add";
		else if (!(strcmp(instr[PC].sub_instr, "100001"))) {
			instr[PC].opCode = "addu";
			unsign = true;
		}
		else if (!(strcmp(instr[PC].sub_instr, "100010")))
			instr[PC].opCode = "sub";
		else if (!(strcmp(instr[PC].sub_instr, "100011"))) {
			instr[PC].opCode = "subu";
			unsign = true;
		}
		else if (!(strcmp(instr[PC].sub_instr, "100100")))
			instr[PC].opCode = "and";
		else if (!(strcmp(instr[PC].sub_instr, "100101")))
			instr[PC].opCode = "or";
		else if (!(strcmp(instr[PC].sub_instr, "101010")))
			instr[PC].opCode = "slt";
		else if (!(strcmp(instr[PC].sub_instr, "101011"))) {
			instr[PC].opCode = "sltu";
			unsign = true;
		}
		else if (!(strcmp(instr[PC].sub_instr, "001000")))
			instr[PC].opCode = "jr";
		else if (!(strcmp(instr[PC].sub_instr, "000000"))) {
			instr[PC].opCode = "sll";
			shift = true;
		}
		else if (!(strcmp(instr[PC].sub_instr, "000010"))) {
			instr[PC].opCode = "srl";
			shift = true;
		}
		else if (!(strcmp(instr[PC].sub_instr, "000000"))) {
			instr[PC].opCode = "nop";
			nop = true;
		}
		else {
			cout << "unknown instruction\n";
			unknown = true;
		}
		if ((!nop) && (!unknown)) {
			strncpy(instr[PC].sub_instr, "", 30);
			strncpy(instr[PC].sub_instr, instr[PC].instr_b.c_str() + 16, 5);
			instr[PC].rd = stoi(instr[PC].sub_instr, 0, 2);

			//counting rs register and also if there is a shift instruction making it store shiftAmmount
			strncpy(instr[PC].sub_instr, "", 30);
			strncpy(instr[PC].sub_instr, instr[PC].instr_b.c_str() + 6, 5);
			if (!shift) {
				if (unsign) {
					instr[PC].rs = stoi(instr[PC].sub_instr, 0, 2);
				}
				else {
					if (instr[PC].sub_instr[0] == '1') {
						inverse(instr[PC].sub_instr);
						instr[PC].rs = (stoi(instr[PC].sub_instr, 0, 2) + 1) * -1;
					}
					else if (instr[PC].sub_instr[0] == '0')
						instr[PC].rs = stoi(instr[PC].sub_instr, 0, 2);
				}
			}
			else {
				strncpy(instr[PC].sub_instr, "", 30);
				strncpy(instr[PC].sub_instr, instr[PC].instr_b.c_str() + 21, 5);
				instr[PC].rs = stoi(instr[PC].sub_instr, 0, 2);
			}

			//counting rt register and determining if it is unsigned or signed 
			strncpy(instr[PC].sub_instr, "", 30);
			strncpy(instr[PC].sub_instr, instr[PC].instr_b.c_str() + 11, 5);
			if (unsign) {
				instr[PC].rt = stoi(instr[PC].sub_instr, 0, 2);
			}
			else {
				if (instr[PC].sub_instr[0] == '1') {
					inverse(instr[PC].sub_instr);
					instr[PC].rt = (stoi(instr[PC].sub_instr, 0, 2) + 1) * -1;
				}
				else if (instr[PC].sub_instr[0] == '0')
					instr[PC].rt = stoi(instr[PC].sub_instr, 0, 2);
			}
		}
	}
	//in case if there are no registers created, it helps to create ones 
	if (reg[instr[PC].rd].id != "$" + to_string(instr[PC].rd)) {
		reg[instr[PC].rd].id = "$" + to_string(instr[PC].rd);
		reg[instr[PC].rd].value = 0;
	}
	if (!shift) {
		if (reg[instr[PC].rs].id != "$" + to_string(instr[PC].rs)) {
			reg[instr[PC].rs].id = "$" + to_string(instr[PC].rs);
			reg[instr[PC].rs].value = 0;
		}
	}
	if (reg[instr[PC].rt].id != "$" + to_string(instr[PC].rt)) {
		reg[instr[PC].rt].id = "$" + to_string(instr[PC].rt);
		reg[instr[PC].rt].value = 0;
	}

	if ((instr[PC].opCode == "add") || (instr[PC].opCode == "addu"))
		reg[instr[PC].rd].value = reg[instr[PC].rs].value + reg[instr[PC].rt].value;
	else if ((instr[PC].opCode == "sub") || (instr[PC].opCode == "subu"))
		reg[instr[PC].rd].value = reg[instr[PC].rs].value - reg[instr[PC].rt].value;
	else if (instr[PC].opCode == "and")
		reg[instr[PC].rd].value = reg[instr[PC].rs].value & reg[instr[PC].rt].value;
	else if (instr[PC].opCode == "or")
		reg[instr[PC].rd].value = reg[instr[PC].rs].value | reg[instr[PC].rt].value;
	else if (instr[PC].opCode == "sll")
		reg[instr[PC].rd].value = reg[instr[PC].rt].value << instr[PC].rs;
	else if (instr[PC].opCode == "srl")
		reg[instr[PC].rd].value = (int)((unsigned int)reg[instr[PC].rt].value >> instr[PC].rs);
	else if ((instr[PC].opCode == "slt") || (instr[PC].opCode == "sltu"))
		reg[instr[PC].rd].value = (reg[instr[PC].rs].value < reg[instr[PC].rt].value) ? 1 : 0;
	//else if (instr[PC].opCode == "jr") {
	//	PC = ;
	//}

	if (!loop)
		PC++;
}

void iType() {
	bool unsign = false;
	loop = false;

	if ((instr[PC].opCode == "") && (PC < InstLen)) {
		strncpy(instr[PC].sub_instr, "", 30);
		strncpy(instr[PC].sub_instr, instr[PC].instr_b.c_str(), 6);

		if (!(strcmp(instr[PC].sub_instr, "001000")))
			instr[PC].opCode = "addi";
		else if (!(strcmp(instr[PC].sub_instr, "001001"))) {
			instr[PC].opCode = "addiu";
			unsign = true;
		}
		else if (!(strcmp(instr[PC].sub_instr, "001100"))) {
			instr[PC].opCode = "andi";
			unsign = true;
		}
		else if (!(strcmp(instr[PC].sub_instr, "001101"))){
			instr[PC].opCode = "ori";
		unsign = true;
		}
		else if (!(strcmp(instr[PC].sub_instr, "000100")))
			instr[PC].opCode = "beq";
		else if (!(strcmp(instr[PC].sub_instr, "000101")))
			instr[PC].opCode = "bne";
		else if (!(strcmp(instr[PC].sub_instr, "001010")))
			instr[PC].opCode = "slti";
		else if (!(strcmp(instr[PC].sub_instr, "001011"))) {
			instr[PC].opCode = "sltiu";
			unsign = true;
		}
		else if (!(strcmp(instr[PC].sub_instr, "100011")))
			instr[PC].opCode = "lw";
		else if (!(strcmp(instr[PC].sub_instr, "101011")))
			instr[PC].opCode = "sw";
		else if (!(strcmp(instr[PC].sub_instr, "001111")))
			instr[PC].opCode = "lui";

		else
			cout << "unknown instruction\n";

		// counting and storint rd, rs registers and immediate value 
		strncpy(instr[PC].sub_instr, "", 30);
		strncpy(instr[PC].sub_instr, instr[PC].instr_b.c_str() + 11, 5);
		instr[PC].rd = stoi(instr[PC].sub_instr, 0, 2);

		strncpy(instr[PC].sub_instr, "", 30);
		strncpy(instr[PC].sub_instr, instr[PC].instr_b.c_str() + 6, 5);
		instr[PC].rs = stoi(instr[PC].sub_instr, 0, 2);

		strncpy(instr[PC].sub_instr, "", 30);
		strncpy(instr[PC].sub_instr, instr[PC].instr_b.c_str() + 16, 16);

		if (unsign) {
			instr[PC].immd = stoi(instr[PC].sub_instr, 0, 2);
		}
		else {
			if (instr[PC].sub_instr[0] == '1') {
				inverse(instr[PC].sub_instr);
				instr[PC].immd = (stoi(instr[PC].sub_instr, 0, 2) + 1) * -1;
			}
			else if (instr[PC].sub_instr[0] == '0')
				instr[PC].immd = stoi(instr[PC].sub_instr, 0, 2);
		}
	}
	//in case if there are no registers created, it helps to create ones 
	if (reg[instr[PC].rd].id != "$" + to_string(instr[PC].rd)) {
		reg[instr[PC].rd].id = "$" + to_string(instr[PC].rd);
		reg[instr[PC].rd].value = 0;
	}
	if (reg[instr[PC].rs].id != "$" + to_string(instr[PC].rs)) {
		reg[instr[PC].rs].id = "$" + to_string(instr[PC].rs);
		reg[instr[PC].rs].value = 0;
	}

	if ((instr[PC].opCode == "addi") || (instr[PC].opCode == "addiu"))
		reg[instr[PC].rd].value = reg[instr[PC].rs].value + instr[PC].immd;
	else if (instr[PC].opCode == "andi")
		reg[instr[PC].rd].value = reg[instr[PC].rs].value & instr[PC].immd;
	else if (instr[PC].opCode == "ori")
		reg[instr[PC].rd].value = reg[instr[PC].rs].value | instr[PC].immd;
	else if ((instr[PC].opCode == "slti") || (instr[PC].opCode == "sltiu"))
		reg[instr[PC].rd].value = (reg[instr[PC].rs].value < instr[PC].immd) ? 1 : 0;
	else if (instr[PC].opCode == "lui") 
		reg[instr[PC].rd].value = instr[PC].immd * pow(2, 16);
	else if (instr[PC].opCode == "lw") {
		if (flag) {
			arrayNum = instr[PC].rs;
			flag = false;
			reg[instr[PC].rd].value = memory[instr[PC].immd / 4].value;
		}
		else {
			reg[instr[PC].rd].value = memory[(instr[PC].rs + instr[PC].immd) / 4].value;
		}
	}
	else if (instr[PC].opCode == "sw") {
		memory[instr[PC].immd / 4].value = reg[instr[PC].rd].value;
	}
	else if (instr[PC].opCode == "bne") {
		if (reg[instr[PC].rs].value != reg[instr[PC].rd].value) {
			loop = true;
			int immd = instr[PC].immd;
			PC = instr[PC + 1 + immd].crnAddr;
			return;
		}
	}
	else if (instr[PC].opCode == "beq") {
		if (reg[instr[PC].rs].value == reg[instr[PC].rd].value) {
			loop = true;
			int immd = instr[PC].immd;
			PC = instr[PC + 1 + immd].crnAddr;			
			return;
		}
	}
	else if (instr[PC].opCode == "jr") {
		PC = instr[PC].rs;
		loop = true;
	}

	if (!loop)
		PC++;
}

void jType() {

	if ((instr[PC].opCode == "") && (PC < InstLen)) {
		strncpy(instr[PC].sub_instr, "", 30);
		strncpy(instr[PC].sub_instr, instr[PC].instr_b.c_str(), 6);

		if (!(strcmp(instr[PC].sub_instr, "000010")))
			instr[PC].opCode = "j";
		else if (!(strcmp(instr[PC].sub_instr, "000011")))
			instr[PC].opCode = "jal";
		else
			cout << "unknown instruction\n";

		strncpy(instr[PC].sub_instr, "", 30);
		strncpy(instr[PC].sub_instr, instr[PC].instr_b.c_str() + 6, 26);

		if (instr[PC].sub_instr[0] == '1') {
			inverse(instr[PC].sub_instr);
			instr[PC].immd = (stoi(instr[PC].sub_instr, 0, 2) + 1) * -1;
		}
		else if (instr[PC].sub_instr[0] == '0')
			instr[PC].immd = stoi(instr[PC].sub_instr, 0, 2);
	}

	if (instr[PC].opCode == "j") {
		PC = instr[PC].immd;
		loop = true;
	}
	else if (instr[PC].opCode == "jal") {
		PC = instr[PC].immd;
		loop = true;
	}
	
	if (!loop)
		PC++;
}

int main(int argc, char* argv[]) {
	//cin >> numofInst;
	numofInst = atoi(argv[1]);
	FILE* myfile = fopen(argv[2], "rb");

	if (myfile == 0) {
		cerr << "Failed to open file for reading\n";
		exit(1);
	}

	int i, j;
	int strLen;// lenth of all binary file(how many instructions are in file in total)

	unsigned char buffer[SIZE * SIZE];
	int n, offset = 0;
	while ((n = fread(buffer, 1, SIZE * SIZE, myfile)) > 0) {
		hexDump(offset, buffer, n);
		if (n < SIZE * SIZE)
			break;
		offset += n;
	}

	InstLen = length / 4;
	if (numofInst > length / 4)
		strLen = numofInst;
	else
		strLen = length / 4;
	instr = new Instr[SIZE * SIZE];
	for (i = 0; i < length; i++)
		sprintf(chr[i], "%02x", elm[i]);

	// allocating address, hex and binary values for all possible instructions inside the whole file
	for (i = 0, j = 0; i < strLen; i++, j += 4) {
		instr[i].crnAddr = i;
		sprintf(instr[i].instr_x, "%s%s%s%s", chr[j], chr[j + 1], chr[j + 2], chr[j + 3]);
		rmvSpc(instr[i].instr_x);
		instr[i].instr_b = hexToBin(instr[i].instr_x);
	}

	//reading data memory file if it exists
	if (argc == 4) {
		FILE* datafile = fopen(argv[3], "rb");
		offset = 0;
		while ((n = fread(buffer, 1, SIZE * SIZE, datafile)) > 0) {
			hexDump(offset, buffer, n);
			if (n < SIZE * SIZE)
				break;
			offset += n;
		}
		for (i = 0; i < length; i++)
			sprintf(chr[i], "%02x", elm[i]);
		memory = new Memory[SIZE * SIZE];
		for (i = 0, j = 0; i < (length / 4); i++, j += 4) {
			sprintf(memory[i].hexMem, "%s%s%s%s", chr[j], chr[j + 1], chr[j + 2], chr[j + 3]);
			memory[i].value = stoll(memory[i].hexMem, 0, 16);
		}

		fclose(datafile);
	}

	// running whole set of MIPS instructions one by one 
	PC = 0;
	loop = false;
	for (i = 0; i < numofInst; i++) {
		if ((PC >= InstLen)) {
			PC++;
			cout << "unknown instruction" << endl;
			continue;
		}

		if (!strncmp(instr[PC].instr_b.c_str(), "000000", 6))
			rType();
		else if (!strncmp(instr[PC].instr_b.c_str(), "0000", 4))
			jType();
		else
			iType();

		//cout << instr[i].opCode << " " << instr[i].rd << " " << instr[i].rs << " " << instr[i].immd << endl;
	}

	for (i = 0; i < 32; i++) {
		if (reg[i].id == "$" + to_string(i)) {
			cout << reg[i].id << ": 0x";
			printf("%08x\n", reg[i].value);
		}
		else
			cout << "$"+ to_string(i) << ": 0x00000000" << endl;
	}
	printf("PC: 0x%08x\n", PC * 4);
	delete []instr;
	fclose(myfile);
	if (argc == 4) {
		delete []memory;
	}
	return 0;
}

string hexToBin(char* sHex) {
	string sReturn = "";
	for (int i = 0; i < sizeof(sHex); ++i)
	{
		switch (sHex[i])
		{
		case '0': sReturn.append("0000"); break;
		case '1': sReturn.append("0001"); break;
		case '2': sReturn.append("0010"); break;
		case '3': sReturn.append("0011"); break;
		case '4': sReturn.append("0100"); break;
		case '5': sReturn.append("0101"); break;
		case '6': sReturn.append("0110"); break;
		case '7': sReturn.append("0111"); break;
		case '8': sReturn.append("1000"); break;
		case '9': sReturn.append("1001"); break;
		case 'a': sReturn.append("1010"); break;
		case 'b': sReturn.append("1011"); break;
		case 'c': sReturn.append("1100"); break;
		case 'd': sReturn.append("1101"); break;
		case 'e': sReturn.append("1110"); break;
		case 'f': sReturn.append("1111"); break;
		}
	}
	return sReturn;
}
