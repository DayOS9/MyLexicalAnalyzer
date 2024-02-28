Name: Daois Sanchez Mora
ID  : 1221797369
Sun Feb 11 23:49:52 MST 2024
*/
#include <iostream>
#include <cstdlib>
#include "parser.h"
#include <map>
#include <unordered_set>
#include <unordered_map>

std::map<std::string, int> list;//to list token and line number for documenting error (semantic)
std::string semanticErrors = "";//string to store all the error messages for semantic errors
std::string epsilonErrors = "";//string to store all the epsilon errors
std::unordered_map<std::string, REG*> lister;//to store all tokenNames and their REGs together for lexical analyzing
std::string printer = "";//string to hold all the output for the lexical analysis
std::string texter = "";//string to hold the inputtext for analyzing
int globalP;//a global pointer to the positon in the input text

using namespace std;

struct REG_node {//this is the individual nodes which represents the states of a REG
	REG_node* first_neighbor;
	std::string first_label;
	REG_node* second_neighbor;
	std::string second_label;
};
struct REG {//the actual REG which is a header pointing to the start of the REG and the accepting state of the REG
	REG_node* start;
	REG_node* accept;
};
struct REG* baseCase(std::string label) {//this is an REG for when a character or epsilon is passed. This is considered the base case (simplest one)
	REG_node* one = new REG_node;
	REG_node* two = new REG_node;
	REG* reg = new REG;

	//here we essentially just create two nodes, the second one being the accepting state. We connect the two with a single transition based on the char passed in
	one->first_neighbor = two;
	one->first_label = label;
	one->second_neighbor = NULL;
	one->second_label = "";
	two->first_neighbor = NULL;
	two->second_neighbor = NULL;
	two->first_label = "";
	two->second_label = "";

	reg->start = one;
	reg->accept = two;
	return reg;
}

struct REG* starCase(REG* curReg) {
	//we need to add two more nodes/states when performing star
	//one will be new start state and the other will be new accept state
	//therefore reg must point to new start and accept node
	//the old one I guess we should ignored for right now(free it somehow)
	//also make old accept node epsilon transition to original start state
	REG_node* newOne = new REG_node;
	REG_node* newTwo = new REG_node;
	REG* head = new REG;
	
	//make head point to new start and accept
	head->start = newOne;
	head->accept = newTwo;
	
	//initialize accept state values
	newTwo->first_neighbor = NULL;
	newTwo->first_label = "";
	newTwo->second_neighbor = NULL;
	newTwo->second_label = "";

	//initalize start node values
	newOne->first_neighbor = curReg->start;
	newOne->first_label = "_";
	newOne->second_neighbor = newTwo;
	newOne->second_label = "_";

	//modify old accept state so that it points to new accept and old start
	curReg->accept->first_neighbor = curReg->start;
	curReg->accept->first_label = "_";
	curReg->accept->second_neighbor = newTwo;
	curReg->accept->second_label = "_";

	//now free up the old (using delete) the old reg since we have 
	//captured the nodes
	delete(curReg);
	return head;
}

struct REG* dotCase(REG* reg1, REG* reg2) {
	//here we only need to make first accept state
	//of the first reg point to the start state
	//of the second reg. Also, must then make reg head 
	//point to new/one accept state which is in reg2
	REG* head = new REG;
	//have head point to start of reg1 and accept of reg2
	head->start = reg1->start;
	head->accept = reg2->accept;

	//now make epsilon transition from end of reg1 to start of reg2
	reg1->accept->first_neighbor = reg2->start;
	reg1->accept->first_label = "_";

	//free up old heads
	delete(reg1);
	delete(reg2);
	return head;
}

struct REG* orCase(REG* reg1, REG* reg2) {
	//here we must add two new nodes: a new start and accept
	//the start state must have two transitions going to each
	//reg start node. Then the accept states in each reg must go
	//to once accept state.
	REG_node* newOne = new REG_node;
	REG_node* newTwo = new REG_node;
	REG* head = new REG;

	//have the new start state point to both regs
	newOne->first_neighbor = reg1->start;
	newOne->first_label = "_";
	newOne->second_neighbor = reg2->start;
	newOne->second_label = "_";

	//have old accept states point to new accept state
	//and set the new accept state values
	newTwo->first_neighbor = NULL;
	newTwo->first_label = "";
	newTwo->second_neighbor = NULL;
	newTwo->second_label = "";

	reg1->accept->first_neighbor = newTwo;
	reg1->accept->first_label = "_";
	reg2->accept->first_neighbor = newTwo;
	reg2->accept->first_label = "_";
	
	//now have new head point to new start and accept
	head->start = newOne;
	head->accept = newTwo;
	
	//free up old heads
	delete(reg1);
	delete(reg2);
	return head;
}

std::unordered_set<REG_node*> match_one_char(std::unordered_set<REG_node*> set, std::string c) {//when passing in a char, check to see what nodes are reachable given the set of nodes
	std::unordered_set<REG_node*> newSet = {};
	//iterate over set of nodes that can be reached from set with c
	for(auto itr : set) {
		if(itr->first_label == c) {
			if(itr->first_neighbor == NULL) {
				//continue;
			}
			else {
				newSet.insert(itr->first_neighbor);
			}
		}
		if(itr->second_label == c) {
			if(itr->second_neighbor == NULL) {
				//continue;
			}
			else {
				newSet.insert(itr->second_neighbor);
			}
		}
	}
	//check if new set empty
	//if empty, means you cannot traverse to any other node with the input supplied and therefore should return empty
	if(newSet.empty()) {
		return newSet;
	}
	//find all nodes that can be reached from consuming no input (epsilon) newSet
	//in this case, we were able to traverse to another node with the current input, now we check for epsilon
	
	bool changed = true;
	std::unordered_set<REG_node*> newNewSet = {};
	while(changed) {
		changed = false;
		for(auto itr : newSet) {
			newNewSet.insert(itr);
			if(itr->first_label == "_") {
				if(itr->first_neighbor == NULL) {
					//continue;
				}
				else {
					newNewSet.insert(itr->first_neighbor);
				}
			}
			if(itr->second_label == "_") {
				if(itr->second_neighbor == NULL) {
					//continue;
				}
				else {
					newNewSet.insert(itr->second_neighbor);
				}
			}
		}
		if(newSet.size() != newNewSet.size()) {//if new elements were added, repeat otherwise return what you have
			changed = true;
			newSet = newNewSet;
			newNewSet = {};
		}
	}	
	return newSet;
}

//start of match
int match(REG* r, std::string s, int p) {
	std::string convert;//this is just to convert the character to string
	//first call match_one_char with epsilon to then get states in which we can read chars
	//make a set storing first node in reg
	std::unordered_set<REG_node*> tmpSet = {r->start};
	tmpSet = match_one_char(tmpSet, "_");
	if(tmpSet.empty()) {
		tmpSet = {r->start};
	}
	//then with the nodes we have received, we will then run matchonechar again with characters starting from p
	//return if empty set or accepting node in set
	int pos = -1;// this can change whenever
	while(s[p] != '\"' || s[p] != ' ' || s[p] != '\t' || s[p] != '\n') {
		convert = s[p];
		tmpSet = match_one_char(tmpSet, convert);
		if(!tmpSet.empty()) {
			for(auto itr : tmpSet) {
				if(itr == r->accept) {
					pos = p;
					break;
				}
			}
			p++;
		}
		else {
			return pos;
		}
	}
	return pos;
}

//start of My_GetToken
//this function is suppose to obtain the token (lexeme/name of token) from a substring in the input text
std::string my_getToken(std::unordered_map<std::string, REG*> tList, std::string s, int p) {
	std::string returner;//this is the string that will contain the token name and the matching substring
	std::string cVert;
	int pos = -1;
	int checker;
	for(auto x : tList) {//we check every REG and see which one gives us the highest match
		//cVert = s[p];
		checker = match(x.second, s, p);
		if(checker >= pos && checker != -1) {
			pos = checker;
			returner = x.first + ", " + "\"" + s.substr(p, pos - p + 1) + "\"\n";
			//since the unordered map we are obtaining the REGs from has the token list reversed, we can assure
			//that the highest match will be obtained and will break any ties since checker >= pos
		}
	}
	if(pos == -1) {
		returner = "ERROR";
	}
	//before returning, update the p value in the string ( add one to it )
	globalP = pos + 1;
	return returner;
}

//lexical analyzer
void lexicalAnalyzer(std::string inputText) {//this function is suppose to call getMyToken on the input string and perfrom lexical analyzing
	std::string tok;//this string will store the {tokenName, substring} from mygettoken call
	int index = 0;
	for(int i = 0; i < inputText.size(); i++) {//this function is just removing any whitespace from the beginning
		if(inputText[i] == ' ' || inputText[i] == '\t' || inputText[i] == '\n') {
			continue;
		}
		else {
			index = i;
			break;
		}
	}
	inputText = inputText.substr(index, inputText.size() - index + 1);//after removing whitespace, then make the changes effective by making the substring where it contains no whitespace
	index = 0;
	while(inputText[index] != '\"') {//while we don't reach the end of the input text which ends off with an "
		while(inputText[index] == '\t' || inputText[index] == '\n' || inputText[index] == ' ') {//skip any space in between the input text
			index++;
		}
		if(inputText[index] == '\"') {//check again because perhaps after advancing after whitespace, we encounter the "
			break;
		}
		tok = my_getToken(lister, inputText, index);//grab the longest matching token starting from the index we give
		printer += tok;//add it to the global string storing all the output values for the lexical analyzing
		if(tok == "ERROR") {
			cout << printer + "\n";
			exit(1);
		}
		index = globalP;//update the index so that we can start checking for the next longest after
	}
	cout << printer;
}


std::string checkEpsilon(std::string token, std::unordered_set<REG_node*> set, REG* header) {
	//this function checks if an expression generates epsilon in which we will throw an error
	for(auto itr : set) {
		if(itr == header->accept) {
			//if the set we have contains the accepting state, we must report error
			std::string tmp = " " + token;
			return tmp;
		}
	}
	return "";
}

// this syntax error function needs to be 
// modified to produce the appropriate message
void Parser::syntax_error(std::string lexeme)//no changes done except that it takes in the token name 
{
    cout << lexeme << " HAS A SYNTAX ERROR IN ITS EXPRESSION\n";
    exit(1);
}

void Parser::snytax_error() {// a call done outside of the parse expression functions
	cout << "SNYTAX ERORR\n";
	exit(1);
}

//this function checks if a token's id matches with a previous token that has been consumed
void Parser::semantic_error(std::string lexeme, int lineNum) {
	//variable to check if a match was made; if there is not match, add to list
	bool add = true;
	//create an iterator for the map; this also resets the iterator back to the beginning of map
	std::map<string, int>::iterator i = list.begin();

	//iterator through to check if the token already exits or not
	while(i != list.end()) {
		if(i->first == lexeme) {
			add = false;
			semanticErrors = semanticErrors + "Line " + std::to_string(lineNum) + ": " + lexeme + " already declared on line " + std::to_string(i->second) + "\n";
			break;
		}
		++i;
	}
	if(add) {
		list[lexeme] = lineNum; 
	}
}

// this function gets a token and checks if it is
// of the expected type. If it is, the token is
// returned, otherwise, synatx_error() is generated
// this function is particularly useful to match
// terminals in a right hand side of a rule.
// Written by Mohsen Zohrevandi
Token Parser::expect(TokenType expected_type, string lexeme)//no changes to this function is made, it does what I need it to do
{
	//only change is that snytax error was introduced in the event that the lexeme passed into exept(token name) is empty
    Token t = lexer.GetToken();
    if (t.token_type != expected_type) {
		if(lexeme.empty()) {
        	snytax_error();
		}
		else {
			syntax_error(lexeme);
		}
	}
    return t;
}

struct REG* Parser::parse_expr(std::string lexeme)//the main function that will go into each expr and check for syntax, semantic, epsilon, and create the REG recursively
{
	//there are two 3 possibilities:
	// expr -> char      expr -> LPAREN       expr -> underscore
	Token t = lexer.peek(1);
	if(t.token_type == CHAR) {
		Token tok = expect(CHAR, lexeme);
		return baseCase(tok.lexeme);
		
	}
	else if(t.token_type == UNDERSCORE) {
		Token tok = expect(UNDERSCORE, lexeme);
		return baseCase("_");
	}
	else if(t.token_type == LPAREN) {
		//since they all start with (expr) do this first
		expect(LPAREN, lexeme);
		REG* curReg = parse_expr(lexeme);//take this and add it into any of the three functions or throw syntax error
		expect(RPAREN, lexeme);
		//peek to check to see which operation to perform
		Token t = lexer.peek(1);
		if(t.token_type == DOT) {
			expect(DOT, lexeme);
			expect(LPAREN, lexeme);
			REG* curReg2 = parse_expr(lexeme);// we take this REG recursively and give it into the next function dotCase
			expect(RPAREN, lexeme);
			return dotCase(curReg, curReg2);
		}
		else if(t.token_type == OR) {
			expect(OR, lexeme);
			expect(LPAREN, lexeme);
			REG* curReg2 = parse_expr(lexeme);//this will be the second expression needed for OrCase
			expect(RPAREN, lexeme);
			return orCase(curReg, curReg2);
		}
		else if(t.token_type == STAR) {
			expect(STAR, lexeme);//only one REG needed which was a base case REG done recursively. 
			return starCase(curReg);
		}
		else {
			syntax_error(lexeme);
		}
	}
	else {
		syntax_error(lexeme);
	}

}

void Parser::parse_token()
{
	//token -> ID expr
	Token mostRecent = expect(ID, "");
	semantic_error(mostRecent.lexeme, mostRecent.line_no); //will be collecting and checking tokens to see if one has already been consumed
	REG* freg = parse_expr(mostRecent.lexeme);//will return an REG which was recursively contstructed (this is the first and last parse expression call)
	std::unordered_set<REG_node*> setter = {};//this set will contain the starting node of the REG
	setter.insert(freg->start);//insert starting node to the set
	setter = match_one_char(setter, "_");//pass in the set and check for epsilon errors
	epsilonErrors += checkEpsilon(mostRecent.lexeme, setter, freg);//print any error messages given back from the function
	lister[mostRecent.lexeme] = freg;// collecting list in form of {tokenName, REG*} for getMyToken()
   
}

void Parser::parse_token_list()
{
	//token_list -> token | token COMMA token_list
	//since both rules expect token regardless, parse it
	parse_token();
	Token t = lexer.peek(1);
	//check if it just a token or if there is more to it (the comma and token list)
	if(t.token_type == COMMA) {
		expect(COMMA, "");
		parse_token_list();
	}
	//else if(t.token_type == END_OF_FILE) {
	//	return;
	//}
	else {
		//snytax_error();
		return;
	}

}

void Parser::parse_tokens_section()
{
	//tokens_section -> token_list HASH
	parse_token_list();
	expect(HASH, "");
    
}

void Parser::parse_input()
{
	//input -> tokens_section INPUT_TEXT
	parse_tokens_section();
	Token text = expect(INPUT_TEXT, "");
	expect(END_OF_FILE, "");
	texter = text.lexeme.substr(1, text.lexeme.size() - 1);//here I am obtaining the input text and removing the first quote
    
}

// This function simply reads and prints all tokens
// I included it as an example. You should compile the provided code
// as it is and then run ./a.out < tests/test0.txt to see what this function does
// This function is not needed for your solution and it is only provided to
// illustrate the basic functionality of getToken() and the Token type.

void Parser::readAndPrintAllInput()
{
    Token t;

    // get a token
    t = lexer.GetToken();

    // while end of input is not reached
    while (t.token_type != END_OF_FILE) 
    {
        t.Print();         	// pringt token
        t = lexer.GetToken();	// and get another one
    }
        
    // note that you should use END_OF_FILE and not EOF
}

int main()
{
    // note: the parser class has a lexer object instantiated in it (see file
    // parser.h). You should not be declaring a separate lexer object. 
    // You can access the lexer object in the parser functions as shown in 
    // the example  method Parser::readAndPrintAllInput()
    // If you declare another lexer object, lexical analysis will 
    // not work correctly
    Parser parser; //create parser class so we can use parse input for the token and expression parts

	parser.parse_input();
	if(!semanticErrors.empty()) { //if there are error messages, print them
		cout << semanticErrors;
		exit(1);
	} //otherwise just continue
	if(!epsilonErrors.empty()) {
		cout << "EPSILON IS NOOOOOOOT A TOKEN !!!" + epsilonErrors + "\n";
		exit(1);
	}
	lexicalAnalyzer(texter);//passing in the input string without the first quote
	//now we will perform the lexical analyzing 
	

    //parser.readAndPrintAllInput();
	
}
