#pragma once
#include "SymActions.h"
#include <map>
#include <iomanip>
#include <thread>
#include "ParseTable.h"



struct cmp_str {
	bool operator()(const char* a, const  char* b) const {
		return std::strcmp(a, b) < 0;
	}
};

void ExecuteLexer(char* file_name, unsigned int* num_error);
bool CreateParsingTable();
bool ParseLexOutput(vector<char*> &synInput);
void AnalyzeSyntax(vector<char*>& synInput);
bool CheckScope(SymbolTable& ST, const char* lexval);
bool CheckFunctionRedefinition(SymbolTable& ST);
void PushPopStack(vector<char*>& synInput, unsigned int in_ind, vector<const char*>& Stack, vector<const char*>* productions, char* action);
void PrintStackMatch(vector<char*>& synInput, unsigned int matched, vector<const char*> Stack, char* action);
void PrintMap();

//Function Pointers for Symentic Actions.
void (*ActFuncs[])() = { SyAct1, SyAct2, SyAct3, SyAct4, SyAct5, 
						 SyAct6, SyAct7, SyAct8, SyAct9, SyAct10,
						 SyAct11, SyAct12, SyAct13, SyAct14, SyAct15,
						 SyAct16, SyAct17, SyAct18 };

map<const char*, map<const char*, vector<const char*>, cmp_str>, cmp_str> ParseTable;	//Global ParseTable MAP.
map<const char*, bool, cmp_str> MapTerminals;											//A bloomfilter for terminals
map<unsigned int, const char*> LexVal;													//A mapping of Lex values corresponding to the index of that token in vector<char*> synInput
vector<unsigned int> TokenLines;
ofstream fout("actions.tsv");															//file for writing the actions taken by the parser.


int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		cout << "Please Give code file name!!\n";
		return -1;
	}
	vector<char*> synInput;
	unsigned int num_error=0;
	//Simultaneously Running the Lexer and Creating a Parse Table.
	cout << "Lexical Analysis...\n";
	thread LexThread(ExecuteLexer,argv[1], &num_error);
	CreateParsingTable();
	LexThread.join();

	if (num_error)
	{
		cout << endl << num_error << " ERRORS in Lexical Analysis!. Fix these to continue with Syntax Analysis\n";
		return -1;
	}

	ParseLexOutput(synInput);

	cout << "Syntax Analysis...\n";
	//PrintMap();
	fout << "Stack\tInput\t\tAction" << endl;
	AnalyzeSyntax(synInput);

	translated_out.close();
	fout.close();

	return 0;
}


void ExecuteLexer(char* file_name, unsigned int *num_error)
{
	char command[100] = "Lexer.exe \"";
	strcat_s(command,100, file_name);
	strcat_s(command, 100, "\"");
	*num_error = system(command);
}

/// <summary>
/// A Function That Reads and Creates A Map of the Parsing Table from The tab delimeted ParseTable.txt.
/// </summary>
/// <returns></returns>
bool CreateParsingTable()
{
	char* line;
	char* pos = NULL;
	vector<const char*> terminals;
	
	line = (ParseData[0]);

	//Creating a List of Terminals to be used as key2 in our 2D Map.
	terminals.push_back((const char*)(strtok_s(line, "\t", &pos)));
	while (terminals[terminals.size() - 1] != NULL) {
		MapTerminals[terminals[terminals.size() - 1]] = true;
		terminals.push_back(strtok_s(NULL, "\t", &pos));
	}
	terminals.pop_back();		//Removing the NULL that is pushed into it.
	
	for(int i=1; i<29; i++)
	{
		char* p;
		char* terms;
		char* key1 = new char[50];
		int count = 0;

		line = ParseData[i];
		p = strpbrk(line, "\t");
		*p = 0;
		strcpy_s(key1, 50, line );
		line = p+1;
		
		//Creating a Map for the new Non-Terminal Found 
		ParseTable.insert(make_pair(key1, map<const char*, vector<const char*>, cmp_str>()));
		
		while (line != NULL) {
			if ((p = strpbrk(line, "\t")) != NULL)
				*p = 0;
			terms = line;
			line = p == NULL ? p : p + 1;

			// Pushing Each Non-Terminal/ Terminal separately.
			ParseTable[key1][terminals[count]].push_back(strtok_s(terms, " ", &pos));
			while (ParseTable[key1][terminals[count]][ParseTable[key1][terminals[count]].size() - 1] != NULL) {
				ParseTable[key1][terminals[count]].push_back(strtok_s(NULL, " ", &pos));
			}
			count++;
		}
	}
	return true;
}


/// <summary>
/// Reading the tokens from output file of Lexical Analyzer.
/// </summary>
/// <param name="synInput"></param>
/// <returns></returns>
bool ParseLexOutput(vector<char*> &synInput)
{
	ifstream fin("words.txt");
	while (!fin.eof())
	{
		char* token;
		char* pos = NULL;
		char* line = new char[1000]{ 0 };
		
		unsigned int line_num;
		fin >> line_num;
		TokenLines.push_back(line_num);
		fin.ignore();
		fin.getline(line, 1000, '\n');
		if (!strcmp(line, ""))
		{
			synInput.push_back((char*)"$");
			break;
		}
		if (line[0] != '<')
			continue;

		line++;
		token = strtok_s(line, ",", &pos);

		if (!strcmp(token, "Comment"))
			continue;

		if (!strcmp(token, "Ident") || !strcmp(token, "IntConst") || !strcmp(token, "BoolConst")
			|| !strcmp(token, "DoubleConst") || !strcmp(token, "StringConst"))
		{
			synInput.push_back(token);
			pos[strlen(pos) - 1] = 0;
			LexVal.insert(pair<unsigned int, const char*>(synInput.size() - 1, (const char*)pos));
		}
		else
		{
			pos[strlen(pos)-1] = 0;
			if (!strcmp(pos, "<="))
			{
				synInput.push_back((char*)"<");
				pos++;
			}
			synInput.push_back(pos);
		}
	}
	return true;
}


/// <summary>
/// Analyzing the syntax of the code file based on the Parsing Table and Applying Error Handling Heuristics also.
/// (update 1) Also Added Scope Checking in it.
/// </summary>
/// <param name="synInput"></param>
void AnalyzeSyntax(vector<char*>& synInput)
{
	vector<const char*> Stack;
	unsigned int in_ind = 0;
	Stack.push_back("$");
	Stack.push_back("Program");

	//Symbol Table Related Variables
	SymbolTable ST;
	functions = &ST.functions;
	ST.scope_entries.push_back({ 0 });
	SymbolEntry* SE = NULL;
	bool skip_brace = false;
	bool function_def = false;
	/////////////////////////////////
	
	//While the input is not ended and the stack is not empty
	while (strcmp(synInput[in_ind], "$") && strcmp(Stack[Stack.size()-1], "$") || Stack[Stack.size() - 1][0] == '^')
	{

		//Checking If Top of the Stack is Some symentic action Symbol
		if (Stack[Stack.size() - 1][0] == '^')
		{
			int n = atoi((Stack[Stack.size() - 1] + 2));
			ActFuncs[n - 1]();
			Stack.pop_back();
			continue;
		}

		char* action = new char[100]{ 0 };
		vector<const char*>* productions;

		//If An unknown symbol is seen in the code
		if (!MapTerminals.count(synInput[in_ind]))
		{
			cout << "Syntax ERROR (line " << TokenLines[in_ind] << " ): " << "Unrecognized Input Symbol '" << synInput[in_ind] << "'. Skipping Input!\n";
			in_ind++;
			strcat_s(action, 100,"Skip Input");
		}

		//Checking if top of Stack is a Terminal
		if (MapTerminals.count(Stack[Stack.size() - 1]))
		{
			//Terminals Matched
			if (!strcmp(Stack[Stack.size() - 1], synInput[in_ind]))
			{
				//Handling Symbol Table Insertions
				if (!strcmp(synInput[in_ind], "Ident"))
				{
					if (SE != NULL)
					{
						bool isRedefine = false;
						strcpy_s(SE->id_name, 40, LexVal[in_ind]);
						for (unsigned int i = 0; i < ST.scope_entries[ST.scope_entries.size() - 1].symbols.size(); i++)
						{
							if (!strcmp(SE->id_name, ST.scope_entries[ST.scope_entries.size() - 1].symbols[i]->id_name))
							{
								cout << "Syntax ERROR (line " << TokenLines[in_ind] << "): " << "Redefinition of '" << LexVal[in_ind] << "'\n";
							}
						}
						if (!isRedefine && !strcmp(synInput[in_ind + 1], ";") || !strcmp(synInput[in_ind + 1], ",") || !strcmp(synInput[in_ind + 1], ")"))
						{
							if (function_def)
							{	//Pushing arguments types that the function takes.
								ST.functions[ST.functions.size() - 1].args_type.push_back(SE->id_type);
								//vector<char[10]>* STF = &(ST.functions[ST.functions.size() - 1].args_type);
								//strcpy_s((*STF)[STF->size()-1], 10, SE->id_type);
								if(!strcmp(synInput[in_ind + 1], ")"))
									function_def = false;	//meaning that function definition ends here.
							}
							ST.scope_entries[ST.scope_entries.size() - 1].symbols.push_back(SE);
							SE = NULL;
						}
						//This means new function is going to start
						else if (!isRedefine && !strcmp(synInput[in_ind + 1], "("))
						{
							//Because new scope is being created here instead of at '{'. 
							//So, this bool will skip the next '{' instead of creating another scope there.
							skip_brace = true;
							ST.scope_entries.push_back({ 0 });
							ST.scope_entries[ST.scope_entries.size() - 1].scope_level = ST.scope_entries.size() - 1;
							//Pushing new Function's definition.
							ST.functions.push_back({ 0 });
							strcpy_s(ST.functions[ST.functions.size() - 1].ret_type, 10, SE->id_type);
							strcpy_s(ST.functions[ST.functions.size() - 1].func_name, 40, SE->id_name);
							function_def = true;

							delete SE;
							SE = NULL;
						}
					}
					//Else Checking if the identifier at hand is used inside the scope or not.
					else if (!CheckScope(ST, LexVal[in_ind]))
					{
						cout << "Syntax ERROR (line " << TokenLines[in_ind] << "): " << "Identifier '" << LexVal[in_ind] << "' not defined in this Scope!\n";
					}
				}
				if (!strcmp(synInput[in_ind], "{"))
				{
					if (!skip_brace)
					{
						ST.scope_entries.push_back({ 0 });
						ST.scope_entries[ST.scope_entries.size() - 1].scope_level = ST.scope_entries.size() - 1;
						delete SE;
						SE = NULL;
					}
					skip_brace = false;
					function_def = false;
					if (CheckFunctionRedefinition(ST))
					{
						cout << "ERROR: (line " << TokenLines[in_ind - 1] << "): " << "Redifinition of Function '"
							<< ST.functions[ST.functions.size() - 1].ret_type << " "
							<< ST.functions[ST.functions.size() - 1].func_name << " (";
						for (unsigned int i = 0; i < ST.functions[ST.functions.size() - 1].args_type.size(); i++)
							cout << ST.functions[ST.functions.size() - 1].args_type[i]
							<<( (i == (ST.functions[ST.functions.size() - 1].args_type.size() - 1) ) ? "" : ", ");
						cout << ")'\n";
					}
				}
				if (!strcmp(synInput[in_ind], "}"))
				{
					ST.scope_entries.pop_back();
				}
			
				
				strcat_s(action, 100, "Match ");
				strcat_s(action, 100, synInput[in_ind]);
				Stack.pop_back();
				last_match = (char*)LexVal[in_ind];
				in_ind++;
			}
			//Error Handling Heuristics
			//Skipping the input uptil semicolon
			else if(!strcmp(Stack[Stack.size() - 1], ";"))
			{
				cout << "Syntax ERROR (line " << TokenLines[in_ind] << "): " << "Expected '" << Stack[Stack.size() - 1] << "', got '" << synInput[in_ind] << "'. Skipping Input!" << endl;
				while(strcmp(synInput[in_ind], ";") && strcmp(synInput[in_ind], "$"))
					in_ind++;
				strcat_s(action, 100, "Skip Input");
			}
			//Poping Top of Stack for unmatched terminals
			else
			{
				Stack.pop_back();
				cout << "Syntax ERROR (line " << TokenLines[in_ind] << "): " << "Expected '" << Stack[Stack.size() - 1] << "', got '" << synInput[in_ind] << "'. Poping Stack!" << endl;
				strcat_s(action, 100, "Pop Stack");
			}
		}
		//Else Checking Parse Table for appropriate productions
		else
		{
			productions = &ParseTable[Stack[Stack.size() - 1]][synInput[in_ind]];

			//Checking for Errors
			//if blank in parse table then skipping input
			if (productions->size() <= 1)
			{
				cout << "Syntax ERROR (line " << TokenLines[in_ind] << "): " << "No production Rule M[" << Stack[Stack.size() - 1] << "][" << synInput[in_ind] << "]. Skiping Input!\n";
				in_ind++;
				strcat_s(action, 100, "Skip Input");
			}
			//if Sync found Poping stack
			else if (!strcmp((*productions)[0], "Sync"))
			{
				Stack.pop_back();
				cout << "Syntax ERROR (line " << TokenLines[in_ind] << "): " << "something expected before '" << synInput[in_ind] << "'. Poping Stack\n";
				strcat_s(action, 100, "Pop Stack");
			}
			//If Some Production is found then pushing them on Stack.
			else
			{
				//Meaning Something new is going to be declared. So, noting down the probable identifier's Type
				if (!strcmp(Stack[Stack.size() - 1], "Type"))
				{
					SE = new SymbolEntry();
					strcpy_s(SE->id_type, 10, (*productions)[0]);
				}
				
				PushPopStack(synInput, in_ind, Stack, productions, action);
			}
		}
		PrintStackMatch(synInput, in_ind, Stack, action);
	}

	//Checking if Stack is not empty when input has ended.
	if (strcmp(Stack[Stack.size() - 1], "$") && !strcmp(synInput[in_ind], "$"))
	{
		//If this is the case then checking if there is still any transistions we can use.
		while (strcmp(Stack[Stack.size() - 1], "$") && !MapTerminals.count(Stack[Stack.size() - 1]))
		{
			char* action = new char[100]{ 0 };
			vector<const char*>* productions;
			productions = &ParseTable[Stack[Stack.size() - 1]][synInput[in_ind]];
			if (productions->size() <= 1)
				break;
			PushPopStack(synInput, in_ind, Stack, productions, action);
			PrintStackMatch(synInput, in_ind, Stack, action);
		}
		//If no more Productions that can go to epsilons then displaying expected grammar at the end.
		if (Stack.size()>1)
		{
			cout << "Syntax ERROR: '";
			while (Stack.size() > 1)
			{
				cout << Stack[Stack.size() - 1] << " ";
				Stack.pop_back();
			}
			cout << "' missing at the end of the code." << endl;
		}
 
	}
	else if (!strcmp(Stack[Stack.size() - 1], "$") && strcmp(synInput[in_ind], "$"))
	{
		cout << "Syntax ERROR: Unexpected Tokens at the end\n";
		while (strcmp(synInput[in_ind], "$"))
			cout << synInput[in_ind] << " ";
	}
}


bool CheckScope(SymbolTable& ST, const char* lexval)
{
	for (int i = ST.scope_entries.size() - 1; i >= 0; i--)
		for (unsigned int j = 0; j < ST.scope_entries[i].symbols.size(); j++)
			if (!strcmp(lexval, ST.scope_entries[i].symbols[j]->id_name ))
				return true;
	
	for (unsigned int i = 0; i < ST.functions.size() ; i++)
		if (!strcmp(ST.functions[i].func_name, lexval))
			return true;

	return false;
}

bool CheckFunctionRedefinition(SymbolTable& ST)
{
	unsigned int fi = ST.functions.size() - 1;
	bool func_red = false;
	for (unsigned int i = 0; i < ST.functions.size() - 1 && !func_red; i++)
	{
		if (!strcmp(ST.functions[i].func_name, ST.functions[fi].func_name) &&
			ST.functions[i].args_type.size() == ST.functions[fi].args_type.size())
		{
			func_red = true;
			for (unsigned int i = 0; i < ST.functions[fi].args_type.size() && !func_red; i++)
			{
				if (strcmp(ST.functions[i].args_type[i], ST.functions[fi].args_type[i]))
					func_red = false;
			}
		}
	}
	return func_red;
}

void PushPopStack(vector<char*>& synInput, unsigned int in_ind, vector<const char*>& Stack, vector<const char*>* productions, char* action)
{
	strcat_s(action, 100, Stack[Stack.size() - 1]);
	strcat_s(action, 100, " --> ");
	Stack.pop_back();
	for (int j = productions->size() - 2; j >= 0; j--)
	{
		if (strcmp((*productions)[j], "?"))	//if the production is not epsilon, then pushing it 
			Stack.push_back((*productions)[j]);
		strcat_s(action, 100, (*productions)[productions->size() - 2 - j]);
		strcat_s(action, 100, " ");
	}
}

void PrintStackMatch(vector<char*>& synInput, unsigned int matched, vector<const char*> Stack, char* action)
{
	for (int i = Stack.size() - 1; i >= 0; i--)
		fout << Stack[i] << " ";
	fout << "\t";
	for (unsigned int i = matched; i < synInput.size(); i++)
		fout << synInput[i] << " ";
	fout << "\t\t" << action << endl;
}


/// <summary>
/// Utility Function to see Whether the Parsing Table is Read Correctly or Not.
/// </summary>
void PrintMap()
{
	for (auto ii = ParseTable.begin(); ii != ParseTable.end(); ++ii) {
		cout << setw(9) << (*ii).first << ": \n";
		for (auto jj = (*ii).second.begin(); jj != (*ii).second.end(); ++jj)
		{
			cout << "\t" << setw(10) << (*jj).first << ": \t";
			vector <const char*> inVect = (*jj).second;
			for (unsigned j = 0; j < inVect.size() - 1; j++)
				cout << inVect[j] << " ";
			cout << endl;
		}
		cout << endl;
	}
}