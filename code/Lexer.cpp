#include <iostream>
#include <fstream>
#include <vector>
#include <map>

using namespace std;


bool readFile(char filename[], vector<char>& buffer);
void Lexer(vector<char>& buffer);
int stateTransition(char c, int state);
void createToken(int state, char lexeme[], int line_num);
void createMapper();

int isKeywordOrReserved(char lexeme[]);
bool isLetter(char c);
bool isPunct(char c);
bool isDigit(char c);
bool isHexAlpha(char c);


// A mapping dictionary that maps final states to its corresposnding token attribute
map<int, const char*> token_attr;
ofstream word_out("words.txt", ios::out);
ofstream table_out("table.txt", ios::out);
unsigned int table_entries = 0;
unsigned int num_errors = 0;

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		cout << "Code File name must be given!!\n";
		return 1;
	}
	vector<char> buffer;

	createMapper();
	if (readFile(argv[1], buffer))
	{
		/*
		cout << buffer.size() << endl;
		for(int i=0; i<buffer.size(); i++)
		{
			cout << buffer[i] << " ";
		}
		*/

		Lexer(buffer);

		word_out.flush();
		word_out.close();
		table_out.flush();
		table_out.close();
	}

	return num_errors;
}


/// <summary>
/// Read the code file given to it
/// </summary>
/// <param name="filename"> name of the code file </param>
/// <param name="buffer"> buffer variable in which file is to be read </param>
/// <returns> true if file read is successfull else false </returns>
bool readFile(char filename[], vector<char>& buffer)
{

	ifstream fin(filename);

	if (fin.fail())
	{
		cout << "File " << filename << " Could not be read!" << endl;
		return false;
	}

	cout << "Reading File ...\n";

	while (!fin.eof())
	{
		buffer.push_back(fin.get());
	}

	return true;
}

/// <summary>
/// Tokenize the stream of characters present in the buffer
/// </summary>
/// <param name="buffer"></param>
void Lexer(vector<char>& buffer)
{
	int line_num = 1;  //Keeping track of lines.

	for (unsigned int i = 0; i < buffer.size()-1;)
	{
		int current_state = 0; //Placing the state machine at start node.
		char lexeme[1000] = { 0 };
		int lexeme_length = 0;

		for (; i < buffer.size(); i++)
		{
			int next_state = stateTransition(buffer[i], current_state);

			// Meaning End of a Lexeme or an error in the code file
			if (next_state == -1)
			{
				if (current_state == 0 && next_state == -1)
				{
					if (buffer[i] != ' ' && buffer[i] != '\r' && buffer[i] != '\n' && buffer[i] != '\0' && buffer[i] != '\t')
					{
						cout << "ERROR: line " << line_num << "\t" << buffer[i] << " Unrecognized Token (Found No Rule for this)\n";
						num_errors++;
					}
					if (buffer[i] == '\n')
						line_num++;
					i++;
				}
				else if (token_attr.count(current_state)==1)
				{
					if (current_state == 17 && lexeme_length > 31)
					{
						cout << "ERROR: line " << line_num << "\t" << lexeme << " Identifier exceeded 31 characters\n";
						num_errors++;
					}
					else
						createToken(current_state, lexeme, line_num);
					lexeme_length = 0;
				}
				break;
			}			
			else
			{
				if (buffer[i] == '\n')
					line_num++;
				current_state = next_state;
				lexeme[lexeme_length] = buffer[i]!= '\n' ? buffer[i] : current_state == 11? '\0' : '\n';
				lexeme_length++;
			}
		}

		if (lexeme_length != 0)
		{
			cout << "ERROR: line " << line_num << "\t" << lexeme << " Unrecognized Token (Token not complete)\n";
			num_errors++;
		}
	}
}


/// <summary>
/// This is a state Machine with all the transitions.
/// It takes a character and the state and outputs the next state.
/// </summary>
/// <param name="c"></param>
/// <param name="state"></param>
/// <returns> next_state </returns>
int stateTransition(char c, int state)
{
	int next_state = -1;
	switch (state)
	{
	case 0: //All Transisitions from the start state
		
		if (c == '"')
			next_state = 1;

		else if (c == '&')
			next_state = 3;

		else if (c == '|')
			next_state = 5;

		else if (isPunct(c))
			next_state = 7;

		else if (c == '+' || c == '-' || c == '*' || c == '%')
			next_state = 8;

		else if (c == '/')
			next_state = 9;

		else if (c == '=')
			next_state = 14;

		else if (c == '<' || c == '>' || c == '!')
			next_state = 15;

		else if (isLetter(c))
			next_state = 17;

		//else if (c == '+' || c == '-')
		//	next_state = 18;

		else if (c == '0')
			next_state = 24;

		else if (isDigit(c))
			next_state = 19;

		else
			next_state = -1;

		break;
	
	case 1: //String start
		if (c == '"')
			next_state = 2;
		else if (c != '\n')
			next_state = 1;
		else
			next_state = -1;
		break;
	
	case 2:
	case 4:
	case 6:
	case 7:
	case 8:
	case 11:
	case 16:
		//Anything at these states will go to dead state
		next_state = -1;
		break;
	
	case 3:
		if (c == '&')
			next_state = 4;
		else
			next_state = -1;
		break;
	
	case 5:
		if (c == '|')
			next_state = 4;
		else
			next_state = -1;
		break;
	
	case 9: //May be division or start of comment
		if (c == '/')
			next_state = 10;
		else if (c == '*')
			next_state = 12;
		else
			next_state = -1;
		break;

	case 10:
		if (c == '\n')
			next_state = 11;
		else
			next_state = 10;
		break;

	case 12:
		if (c == '*')
			next_state = 13;
		else
			next_state = 12;
		break;

	case 13:
		if (c == '/')
			next_state = 11;
		else
			next_state = 12;
		break;

	case 14:
		if (c == '=')
			next_state = 16;
		else
			next_state = -1;
		break;

	case 15:
		if (c == '=')
			next_state = 16;
		else
			next_state = -1;
		break;

	case 17:
		if (isLetter(c) || isDigit(c) || c == '_')
			next_state = 17;
		else
			next_state = -1;
		break;

	case 18:
		if (isDigit(c))
			next_state = 19;
		else
			next_state = -1;
		break;
	
	case 19:
		if (isDigit(c))
			next_state = 19;
		else if (c == '.')
			next_state = 20;
		else
			next_state = -1;
		break;
	
	case 20:
		if (isDigit(c))
			next_state = 20;
		else if (c == 'E' || c == 'e')
			next_state = 21;
		else
			next_state = -1;
		break;
	
	case 21:
		if (isDigit(c))
			next_state = 23;
		else if (c == '+' || c == '-')
			next_state = 22;
		else
			next_state = -1;
		break;
	
	case 22:
		if (isDigit(c))
			next_state = 23;
		else
			next_state = -1;
		break;
	
	case 23:
		if (isDigit(c))
			next_state = 23;
		else
			next_state = -1;
		break;
	
	case 24:
		if (isDigit(c))
			next_state = 19;
		else if (c == '.')
			next_state = 20;
		else if (c == 'x' || c == 'X')
			next_state = 25;
		else
			next_state = -1;
		break;

	case 25:
		if (isHexAlpha(c))
			next_state = 26;
		else
			next_state = - 1;
		break;

	case 26:
		if (isHexAlpha(c))
			next_state = 26;
		else
			next_state = - 1;
		break;
	}

	return next_state;
}


void createToken(int state,char lexeme[], int line_num)
{
	word_out << line_num;
	if (state == 17)
	{
		int key_res = isKeywordOrReserved(lexeme);
		//cout << "< " << (key_res == -1 ? token_attr.at(state) : key_res == 0 ? "Keyword" : "Boolean") << ", " << lexeme << " >" << endl;
		word_out << " <" << (key_res == -1 ? token_attr.at(state) : key_res == 0 ? "Keyword" : "BoolConst") << "," << lexeme << ">" << endl;
		if (key_res == -1)
			table_out << "ID" << table_entries++ << "\t" << lexeme << endl;
	}
	else
	{
		//cout << "< " << token_attr.at(state) << ", " << lexeme << " >" << endl;
		word_out << " <" << token_attr.at(state) << "," << lexeme << ">" << endl;
	}
}


/// <summary>
/// A Mapping Functions that fills the token_attr with their corresponding attributes with keys as the final states.
/// </summary>
void createMapper()
{
	token_attr.insert(pair<int, const char*>(2, "StringConst"));
	token_attr.insert(pair<int, const char*>(4, "BoolOP"));
	token_attr.insert(pair<int, const char*>(6, "BoolOP"));
	token_attr.insert(pair<int, const char*>(7, "Punctuation"));
	token_attr.insert(pair<int, const char*>(8, "ArithOP"));
	token_attr.insert(pair<int, const char*>(9, "ArithOP"));
	token_attr.insert(pair<int, const char*>(18, "ArithOP"));
	token_attr.insert(pair<int, const char*>(11, "Comment"));
	token_attr.insert(pair<int, const char*>(14, "AssignOP"));
	token_attr.insert(pair<int, const char*>(15, "CompOP"));
	token_attr.insert(pair<int, const char*>(16, "CompOP"));
	token_attr.insert(pair<int, const char*>(17, "Ident"));
	//token_attr.insert(pair<int, const char*>(19, "IntegerB10"));
	//token_attr.insert(pair<int, const char*>(24, "IntegerB10"));
	token_attr.insert(pair<int, const char*>(19, "IntConst"));
	token_attr.insert(pair<int, const char*>(24, "IntConst"));
	token_attr.insert(pair<int, const char*>(20, "DoubleConst"));
	token_attr.insert(pair<int, const char*>(23, "DoubleConst"));
	//token_attr.insert(pair<int, const char*>(26, "IntegerB16"));
	token_attr.insert(pair<int, const char*>(26, "IntConst"));

}



/// <summary>
/// Checking if the identifier belongs to 
/// keyword or a reserved word 
/// </summary>
/// <param name="lexeme"></param>
/// <returns></returns>
int isKeywordOrReserved(char lexeme[])
{
	const char* keywords[] = { "void" , "int", "double", "bool", "string", "class", "interface",
						 "null", "this", "extends", "implements", "for", "while", "if",
						 "else", "return", "break", "New", "NewArray", "Print", "ReadInteger", "ReadLine"};

	const char* reserved[] = { "true", "false" };

	for (int i = 0; i < 22; i++)
	{
		if (strcmp(lexeme, keywords[i]) == 0)
			return 0;
	}

	for (int i = 0; i < 2; i++)
	{
		if (strcmp(lexeme, reserved[i]) == 0)
			return 1;
	}
	return -1;
}



///////////////////////////////////////////////////////////////////
///				Helper Functions to check type of input			///
///////////////////////////////////////////////////////////////////
bool isLetter(char c)
{
	if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
		return true;
	return false;
}

bool isPunct(char c)
{
	if (c == '.' || c == ',' || c == ';' || c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']')
		return true;
	return false;
}

bool isDigit(char c)
{
	if (c >= '0' && c <= '9')
		return true;
	return false;
}

bool isHexAlpha(char c)
{
	if (isDigit(c) || c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == 'E' || c == 'F' ||
		c == 'a' || c == 'b' || c == 'c' || c == 'd' || c == 'e' || c == 'f')
		return true;

	return false;
}