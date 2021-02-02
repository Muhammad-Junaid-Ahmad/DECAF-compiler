#pragma once
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;


// Custom Data Structure for Symbol Table
struct SymbolEntry {
	char id_type[10] = "";
	char id_name[40] = "";
};
struct FunctionEntry {
	char ret_type[10] = "";
	char func_name[40] = "";
	vector<char*> args_type;
};
struct Scope {
	unsigned int scope_level;
	vector<SymbolEntry*> symbols;
};
struct SymbolTable {
	vector<Scope> scope_entries;
	vector<FunctionEntry> functions;
};


///////////////////////////////////////////
//		Symentic Actions Functions		///
///////////////////////////////////////////
ofstream translated_out("translated_code.txt");
vector<char*> SymActStack;
char* last_match;
static unsigned int label_num = 0;
vector<unsigned int> nested_label_stack;
vector<char*> nested_Func_call_stack;
vector<int> nested_Func_call_args;
static unsigned int temp_num = 0;
vector<FunctionEntry>* functions;
char* CreateNewTmp()
{
	char* tmp = new char[20]{ "cmp_gn_t" };
	char* n = new char[5]{};
	_itoa_s(temp_num, n, 5, 10);
	strcat_s(tmp, 20, n);
	delete[] n;
	temp_num++;
	return tmp;
}

void SyAct1()		//Action 1 is for pushing last matched entry onto stack
{
	SymActStack.push_back(last_match);
}
void SyAct2()		//Action 2 is for Multiplication of two top stack entries
{
	char* b = SymActStack.back();
	char* a = SymActStack[SymActStack.size() - 2];
	char* t = CreateNewTmp();
	translated_out << t << " = " << a << " * " << b << endl;

	SymActStack.pop_back();
	SymActStack.pop_back();
	SymActStack.push_back(t);
	//delete t;
}
void SyAct3()		//Action 3 is for Addition of two top stack entries
{
	char* b = SymActStack.back();
	char* a = SymActStack[SymActStack.size() - 2];
	char* t = CreateNewTmp();
	translated_out << t << " = " << a << " + " << b << endl;

	SymActStack.pop_back();
	SymActStack.pop_back();
	SymActStack.push_back(t);
}
void SyAct4()		//Action 4 is for < comparison of two top stack entries
{
	char* b = SymActStack.back();
	char* a = SymActStack[SymActStack.size() - 2];
	char* t = CreateNewTmp();
	translated_out << t << " = " << a << " < " << b << endl;

	SymActStack.pop_back();
	SymActStack.pop_back();
	SymActStack.push_back(t);
}
void SyAct5()		//Action 5 is for <= comparison of two top stack entries
{
	char* b = SymActStack.back();
	char* a = SymActStack[SymActStack.size() - 2];
	char* t = CreateNewTmp();
	translated_out << t << " = " << a << " <= " << b << endl;

	SymActStack.pop_back();
	SymActStack.pop_back();
	SymActStack.push_back(t);
}
void SyAct6()		//Action 6 is for Assignment of last stack entriy to second last stack entry
{
	char* b = SymActStack.back();
	char* a = SymActStack[SymActStack.size() - 2];
	translated_out << a << " = " << b << endl;

	SymActStack.pop_back();
	SymActStack.pop_back();
}


////////////////////////////////////////////////
// Actions 7-10 are for if Else 
// if (Expr) A7 Stmt A8 ElseStmt A10
// A9 else Stmt
///////////////////////////////////////////////
void SyAct7()		//Action 7 is checking value of expression in if condition and directing towards right label.
{
	char* a = SymActStack.back();
	nested_label_stack.push_back(label_num);
	label_num++;
	translated_out << "ifTrue " << a << " goto if_label" << nested_label_stack[nested_label_stack.size() - 1] << endl;
	translated_out << "goto else_label" << nested_label_stack[nested_label_stack.size() - 1] << endl;
	translated_out << "if_label" << nested_label_stack[nested_label_stack.size() - 1] << ":" << endl;

	SymActStack.pop_back();
}
void SyAct8()		//Action 8 is for adding goto instruction that points to end of if Stmt
{
	translated_out << "goto endIf_label" << nested_label_stack[nested_label_stack.size() - 1] << ":" << endl;
	translated_out << "else_label" << nested_label_stack[nested_label_stack.size() - 1] << ":" << endl;
}
void SyAct9()		//Action 9 for adding appropriate label for ending if
{
	translated_out << "endIf_label" << nested_label_stack[nested_label_stack.size() - 1] << ":" << endl;
	nested_label_stack.pop_back();
}




////////////////////////////////////////////////
// Actions 11-13 are for While
// A11 while (Expr) A12 Stmt A13
///////////////////////////////////////////////
void SyAct10()		//Action 10 is for adding goto instruction that points to end of while Stmt
{
	nested_label_stack.push_back(label_num);
	label_num++;
	translated_out << "while_label" << nested_label_stack[nested_label_stack.size() - 1] << ":" << endl;
}
void SyAct11()		//Action 11 is checking value of expression in while's condition and directing towards right label.
{
	char* a = SymActStack.back();
	translated_out << "ifFalse " << a << " goto endWhile_label" << nested_label_stack[nested_label_stack.size() - 1] << endl;
	SymActStack.pop_back();
}

void SyAct12()		//Action 12 for adding appropriate label for ending while
{
	translated_out << "goto while_label" << nested_label_stack[nested_label_stack.size() - 1] << ":" << endl;
	translated_out << "endWhile_label" << nested_label_stack[nested_label_stack.size() - 1] << ":" << endl;
	nested_label_stack.pop_back();
}

void SyAct13()		//Action 13 for Return expression from a Function
{
	translated_out << "return " << SymActStack.back() << endl;
	SymActStack.pop_back();
}

void SyAct14()		//Action 14 for Function Definition
{
	translated_out << (*functions)[functions->size() - 1].func_name << "_BeginLabel:" << endl;
	for (unsigned int i = 0; i < (*functions)[functions->size() - 1].args_type.size(); i++)
		translated_out << "Arg " << (*functions)[functions->size() - 1].args_type[i] << endl;

}

void SyAct15()		//Action 15 for Function ending
{
	translated_out << (*functions)[functions->size() - 1].func_name << "_EndLabel:\n" << endl;
}

void SyAct16()		//Action 16 for Function call for holding its identifier
{
	nested_Func_call_args.push_back(0);
	nested_Func_call_stack.push_back(SymActStack.back());
	SymActStack.pop_back();
}
void SyAct17()		//Action 17 for Function call
{
	char* t = CreateNewTmp();
	translated_out << t << " = Call " << nested_Func_call_stack.back() <<", " << nested_Func_call_args.back() << endl;
	SymActStack.push_back(t);
	nested_Func_call_args.pop_back();
	nested_Func_call_stack.pop_back();
}
void SyAct18()		//Action 18 for Function call Putting arguments
{
	nested_Func_call_args.back()++;
	translated_out <<"PutArg " << SymActStack.back() << endl;
	SymActStack.pop_back();
}