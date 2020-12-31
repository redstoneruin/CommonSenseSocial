/**
 * Author: Ryan Steinwert
 *
 * Implementation file for database rule manager
 */
#define PARSE_BUF_SIZE 2048
#define VARIABLE_STRING "%VAR%"

#include <stdio.h>
#include <string.h>
#include <string>

#include "CSDBRuleManager.h"


/**
 * Constructor for rule manager type
 */
CSDBRuleManager::CSDBRuleManager()
{

}


/**
 * Deconstructor for rule manager
 */
CSDBRuleManager::~CSDBRuleManager()
{
}



/**
 * Loads the rule set from the given file
 * @param path The path of the file
 * @reutrn 0 if successful, error code if not
 */
int CSDBRuleManager::loadRules(const char* path)
{
	int ret;
	char buf[PARSE_BUF_SIZE];
	FILE* file;

	file = fopen(path, "r");

	if(file == nullptr) return -1;

	// read by line into the parse buffer
	while(fscanf(file, "%s", buf) == 1) 
	{
		if(strcmp(buf, "match") == 0) {
			ret = parseMatch(file);
			if(ret != 0) {				
				fclose(file);
				return ret;
			}
		} else {
			fclose(file);
			return -2;
		}
	}

	fclose(file);
	return 0;
}


/**
 * Parse a match statement from the given rules file
 * @param file The file to continue parsing, must be open
 * @return 0 if successfully parsed the match statement, error code if could not
 */
int CSDBRuleManager::parseMatch(FILE* file)
{
	unsigned long nextSep;
	char buf[PARSE_BUF_SIZE];
	rule_s rule;
	std::vector<std::string> pathVector;
	std::vector<std::string> varVector;

	// first parse the match path
	if(fscanf(file, "%s", buf) != 1) return -1;

	std::string pathString(buf);

	// break up the path

	while(true) 
	{
		nextSep = pathString.find_first_of('/');

		// check if some name exists
		if(nextSep == 0) return -2;

		std::string name = pathString.substr(0,nextSep);

		// determine whether its a variable
		if(pathString[0] == '{') {
			// check if string is long enough for a variable
			if(nextSep < 2) return -2;
			
			pathVector.push_back(std::string(VARIABLE_STRING));
			std::string newVar = name.substr(1,name.find_last_of('}')-1);
			
			// check if variable exists already
			for(std::string var : varVector) {
				if(var.compare(newVar) == 0) return -4;
			}

			varVector.push_back(newVar);
		} else if(name.find_last_of(':') != std::string::npos){
			pathVector.push_back(name.substr(0,name.length()-1));
		} else {
			pathVector.push_back(name);
		} 

		if(nextSep == std::string::npos) break;

		if(nextSep+1 >= pathString.length()) return -2;

		pathString = pathString.substr(nextSep+1);
	}


	// continue to parse until closing bracket found
	while(fscanf(file, "%s", buf) == 1) 
	{
		std::string str(buf);
		
		// check for the close bracket
		if(str.find_first_of('}') != std::string::npos) {
			return 0;
		}	
	}

	return -2;
}