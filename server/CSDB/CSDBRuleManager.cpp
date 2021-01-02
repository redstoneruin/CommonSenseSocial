/**
 * Author: Ryan Steinwert
 *
 * Implementation file for database rule manager
 */
#define PARSE_BUF_SIZE 2048

#define VARIABLE_STRING "%VAR%"
#define AUTH_UID_STRING "auth.uid"

#include <stdio.h>
#include <string.h>

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
	while(fgets(buf, PARSE_BUF_SIZE, file) != nullptr) 
	{
		if(strstr(buf, "match") != nullptr) {
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
 * Check whether the user has access to an item/collection at the given path
 * @param path The path of the item to access
 * @param uid The user id for the request
 * @param perms The permissions wanted, contains 'w', 'r' or both
 * @return True if the user has access to the given path, false if does not
 */
bool CSDBRuleManager::hasPerms(const char* path, const char* uid, const char* perms)
{
	unsigned long nextSep;
	bool wantsRead, wantsWrite;
	std::vector<std::string> pathVector;
	std::string pathString(path);

	wantsRead = wantsWrite = false;

	if(strchr(perms, 'r') != nullptr) wantsRead = true;
	if(strchr(perms, 'w') != nullptr) wantsWrite = true;

	while(true) 
	{
		nextSep = pathString.find_first_of('/');

		if(nextSep == 0) return false;

		pathVector.push_back(pathString.substr(0,nextSep));

		
		if(nextSep == std::string::npos) break;

		pathString = pathString.substr(nextSep+1);

	}

	for(rule_s* rule : rules)
	{
		if(isPathMatch(pathVector, *rule)) {

		}
	}

	return false;
}



/**
 * Check if the path vector matches a given rule, and therefore will apply
 * @param pathVector The path vector containing the name hierarchy
 * @param rule The rule struct to check for matches
 * @return True if this rule matches the path vector, false if does not
 */
bool CSDBRuleManager::isPathMatch(std::vector<std::string> pathVector, rule_s rule)
{
	return false;
}


/**
 * Parse a match statement from the given rules file
 * @param file The file to continue parsing, must be open
 * @return 0 if successfully parsed the match statement, error code if could not
 */
int CSDBRuleManager::parseMatch(FILE* file)
{
	int ret;
	unsigned long nextSep;
	char buf[PARSE_BUF_SIZE];
	rule_s* rule;
	std::vector<std::string> pathVector;
	std::vector<std::string> varVector;

	// first parse the match path
	if(fgets(buf, PARSE_BUF_SIZE, file) == nullptr) return -1;

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


	// create and initialize the rule
	rule = (rule_s*) malloc (sizeof(rule_s));
	initRule(rule);


	// continue to parse until closing bracket found
	while(fgets(buf, PARSE_BUF_SIZE, file) != nullptr) 
	{
		std::string str(buf);

		if(strstr(buf, "allow")) {
			if((ret = parsePrereq(buf, rule)) != 0) {
				free(rule);
				return ret;
			}
		}

		if(strchr(buf, '}') != nullptr) {
			// found closing character, add to rules and return
			rules.push_back(rule);
			return 0;
		}
	}

	free(rule);
	return -1;
}


/**
 * Parse a prereq from a file after the allow statement
 * @param file The file to continue parsing, must be open
 * @param rule The rule to fill with prereq
 * @return 0 if successful, error code if not
 */
int CSDBRuleManager::parsePrereq(char* buf, rule_s* rule)
{
	int ret, param1len, param2len;
	bool param1Valid, param2Valid;
	char permsBuf[3];
	char param1Buf[65];
	char param2Buf[65];
	char opBuf[4];
	prereq_s* prereq;

	//
	// TODO: allow more than just string type
	//

	prereq = (prereq_s*) malloc (sizeof(prereq_s));
	prereq->next = nullptr;

	prereq->param1.type = prereq->param2.type = PTYPE::STRING;


	if((ret = sscanf(buf, "%*s %2s %*s %64s %3s %64s", permsBuf, param1Buf, opBuf, param2Buf)) < 1) {
		free(prereq);
		return -1;
	} else if(ret == 1) {
		prereq->hasCheck = false;
	} else if(ret < 4) {
		free(prereq);
		return -1;
	} else {
		prereq->hasCheck = true;
	}

	if(strchr(permsBuf, 'r') != nullptr) prereq->read = true;
	else prereq->read = false;

	if(strchr(permsBuf, 'w') != nullptr) prereq->write = true;
	else prereq->write = false;

	if(!prereq->hasCheck) {
		addPrereq(rule, prereq);
		return 0;
	}

	// parse the operator
	if(strcmp(opBuf, "==") == 0) {
		prereq->op = OPERATOR::EQUAL;
	} else if(strcmp(opBuf, "<=") == 0) {
		prereq->op = OPERATOR::LESS_EQUAL;
	} else if(strcmp(opBuf, ">=") == 0) {
		prereq->op = OPERATOR::GREATER_EQUAL;
	} else if(strcmp(opBuf, ">") == 0) {
		prereq->op = OPERATOR::GREATER_THAN;
	} else if(strcmp(opBuf, "<") == 0) {
		prereq->op = OPERATOR::LESS_THAN;
	} else {
		// invalid op, failure
		free(prereq);
		return -2;
	}


	// check for special variables
	if(strcmp(param1Buf, AUTH_UID_STRING) == 0) {
		param1Valid = true;
		prereq->param1.type = PTYPE::AUTH_UID;
		prereq->param1.value = nullptr;
	}

	if(strcmp(param2Buf, AUTH_UID_STRING) == 0) {
		param2Valid = true;
		prereq->param1.type = PTYPE::AUTH_UID;
		prereq->param1.value = nullptr;
	}

	
	// check variable types
	for(int i = 0; i < rule->numPathVars && (!param1Valid || !param2Valid); i++) {
		if(!param1Valid && strcmp(rule->pathVariables[i], param1Buf) == 0) {
			param1Valid = true;
			prereq->param1.type = PATH_VAR;
		}

		if(!param2Valid && strcmp(rule->pathVariables[i], param2Buf) == 0) {
			param2Valid = true;
			prereq->param1.type = PATH_VAR;
		}
	}

	if(!param1Valid || !param2Valid) return -3;

	if(prereq->param1.type == PTYPE::STRING || prereq->param1.type == PTYPE::PATH_VAR) {

		// copy over params
		param1len = strlen(param1Buf);

		prereq->param1.value = (char*) malloc (sizeof(char) * (param1len+1));
		
		strncpy(prereq->param1.value, param1Buf, param1len+1);
	}

	if(prereq->param2.type == PTYPE::STRING || prereq->param2.type == PTYPE::PATH_VAR) {
		param2len = strlen(param2Buf);

		prereq->param2.value = (char*) malloc (sizeof(char) * (param2len+1));

		strncpy(prereq->param2.value, param2Buf, param2len+1);
	}

	addPrereq(rule, prereq);

	return 0;
}



/**
 * Initialize a rule with default values
 * @param rule Pointer to the rule to initialize
 */
void CSDBRuleManager::initRule(rule_s* rule)
{
	rule->pathSize = 0;
	rule->numPathVars = 0;
	rule->collectionPath = nullptr;
	rule->pathVariables = nullptr;
	rule->prereq = nullptr;
}


/**
 * Add a prereq onto the linked list of prereqs
 * @param rule Pointer to the rule to add the prereq
 * @param prereq Pointer to prereq to add
 */
void CSDBRuleManager::addPrereq(rule_s* rule, prereq_s* prereq)
{

	if(rule->prereq == nullptr) {
		rule->prereq = prereq;
		return;
	}

	prereq_s* prereqIndex = rule->prereq;

	while(prereqIndex->next != nullptr) {
		prereqIndex = prereqIndex->next;
	}

	prereqIndex->next = prereq;
}