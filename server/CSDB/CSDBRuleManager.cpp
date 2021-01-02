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
bool CSDBRuleManager::hasPerms(const char* path, request_info_s requestInfo)
{
	unsigned long nextSep;
	bool wantsRead, wantsWrite;
	std::vector<std::string> pathVector;
	std::string pathString(path);

	if(requestInfo.isAdmin) return true;

	wantsRead = wantsWrite = false;

	if(strchr(requestInfo.perms, 'r') != nullptr) wantsRead = true;
	if(strchr(requestInfo.perms, 'w') != nullptr) wantsWrite = true;

	while(true) 
	{
		nextSep = pathString.find_first_of('/');

		pathVector.push_back(pathString.substr(0,nextSep));

		if(nextSep == std::string::npos) break;

		pathString = pathString.substr(nextSep+1);

	}

	for(rule_s* rule : rules)
	{
		prereq_s* passedPrereq;
		
		if((passedPrereq = passesRule(*rule, pathVector, requestInfo)) != nullptr) {
			if(wantsRead && wantsWrite) {
				if(passedPrereq->read && passedPrereq->write) return true;
			} else if(wantsRead && passedPrereq->read) {
				return true;
			} else if(wantsWrite && passedPrereq->write) {
				return true;
			}
		}
	}

	return false;
}


/**
 * Check if a rule is passed, given that the path matches
 * @param rule The rule to check
 * @param uid The current user's uid
 * @return The prereq that was passed, null if not
 */
prereq_s* CSDBRuleManager::passesRule(rule_s rule, std::vector<std::string> pathVector, request_info_s requestInfo)
{

	if(!isPathMatch(pathVector, rule)) return nullptr;

	prereq_s* prereqIndex;

	// go through each prereq
	prereqIndex = rule.prereq;

	while(prereqIndex != nullptr)
	{
		if(!prereqIndex->hasCheck) return prereqIndex;

		bool passes;
		int strComparison;
		const char* param1str;
		const char* param2str;
		param_s param1 = prereqIndex->param1;
		param_s param2 = prereqIndex->param2;
		OPERATOR op = prereqIndex->op;

		switch(param1.type) {
		case PTYPE::STRING:
			param1str = param1.value;
			break;
		case PTYPE::NUMBER:
			param1str = param1.value;
			break;
		case PTYPE::PATH_VAR:
			param1str = pathVector[lookupPathVar(rule, param1.value)].c_str();
			break;
		case PTYPE::AUTH_UID:
			param1str = requestInfo.uid;
			break;
		default:
			param1str = "";
			break;
		}

		switch(param2.type) {
		case PTYPE::STRING:
			param2str = param2.value;
			break;
		case PTYPE::NUMBER:
			param2str = param2.value;
			break;
		case PTYPE::PATH_VAR:
			param2str = pathVector[lookupPathVar(rule, param2.value)].c_str();
			break;
		case PTYPE::AUTH_UID:
			param2str = requestInfo.uid;
			break;
		default:
			param2str = "";
			break;
		}

		strComparison = strcmp(param1str, param2str);

		switch(op) {
		case OPERATOR::EQUAL:
			passes = strComparison == 0;
			break;
		case OPERATOR::LESS_THAN:
			passes = strComparison < 0;
			break;
		case OPERATOR::GREATER_THAN:
			passes = strComparison > 0;
			break;
		case OPERATOR::LESS_EQUAL:
			passes = strComparison <= 0;
			break;
		case OPERATOR::GREATER_EQUAL:
			passes = strComparison >= 0;
			break;
		default:
			passes = false;
		}

		//printf("compared [%s] and [%s], got %d\n", param1str, param2str, strComparison);

		if(passes) return prereqIndex;

		prereqIndex = prereqIndex->next;	
	}

	return nullptr;
}


/**
 * Get in index of a path variable in for path vector
 * @param rule The rule to search
 * @param varName The variable name to find
 * @return The index of the path variable in the path vector
 */
unsigned int CSDBRuleManager::lookupPathVar(rule_s rule, const char* varName)
{
	unsigned int varIndex, varIndexInPath;

	for(unsigned int i = 0; i < rule.numPathVars; i++) 
	{
		char* var = rule.pathVariables[i];
		if(strcmp(var, varName) == 0) {
			varIndex = i;
			break;
		}
	}

	varIndexInPath = 0;
	for(unsigned int i = 0; i < rule.pathSize; i++) 
	{
		char* pathName = rule.collectionPath[i];
		if(strcmp(pathName, VARIABLE_STRING) == 0) {
			// found a variable
			if(varIndexInPath == varIndex) return i;
			varIndexInPath++;
		}

	}

	// should never reach here due to parsing rules
	return (unsigned int)-1;
}

/**
 * Check if the path vector matches a given rule, and therefore will apply
 * @param pathVector The path vector containing the name hierarchy
 * @param rule The rule struct to check for matches
 * @return True if this rule matches the path vector, false if does not
 */
bool CSDBRuleManager::isPathMatch(std::vector<std::string> pathVector, rule_s rule)
{
	// check whether the path is shorter than the rule path
	if(pathVector.size() < rule.pathSize) return false;


	for(unsigned int i = 0; i < rule.pathSize; i++)
	{
		char* ruleName = rule.collectionPath[i];
		std::string pathName = pathVector[i];

		if(strcmp(ruleName, VARIABLE_STRING) == 0) continue;

		if(pathName.compare(ruleName) != 0) return false;

	}

	return true;
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
		unsigned long colonIndex;
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
		} else if((colonIndex = name.find_last_of(':')) != std::string::npos) {
			pathVector.push_back(name.substr(0,colonIndex));
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

	fillRulePath(rule, pathVector, varVector);

	// continue to parse until closing bracket found
	while(fgets(buf, PARSE_BUF_SIZE, file) != nullptr) 
	{
		std::string str(buf);

		if(strstr(buf, "allow")) {
			if((ret = parsePrereq(buf, rule)) != 0) {
				if(rule->collectionPath != nullptr) free(rule->collectionPath);
				if(rule->pathVariables != nullptr) free(rule->pathVariables);
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

	if(rule->collectionPath != nullptr) free(rule->collectionPath);
	if(rule->pathVariables != nullptr) free (rule->pathVariables);
	free(rule);
	return -1;
}


/**
 * Fill a rule with the given path and path variables
 * @param rule Pointer to rule to fill
 * @param pathVector The vector for the path names in hierarchical order
 * @param The path variables in hierarchical order
 */
void CSDBRuleManager::fillRulePath(rule_s* rule, std::vector<std::string> pathVector, std::vector<std::string> varVector)
{
	rule->pathSize = pathVector.size();
	rule->numPathVars = varVector.size();

	rule->collectionPath = (char**) malloc (sizeof(char*) * rule->pathSize);
	rule->pathVariables = (char**) malloc (sizeof(char*) * rule->numPathVars);

	// fill the arrays
	for(unsigned int i = 0; i < rule->pathSize; i++) 
	{
		std::string str = pathVector[i];
		rule->collectionPath[i] = (char*) malloc (sizeof(char) * (str.length() + 1));
		strncpy(rule->collectionPath[i], str.c_str(), str.length()+1);
	}

	for(unsigned int i = 0; i < rule->numPathVars; i++) 
	{
		std::string str = varVector[i];
		rule->pathVariables[i] = (char*) malloc (sizeof(char) * (str.length() + 1));
		strncpy(rule->pathVariables[i], str.c_str(), str.length()+1);
	}
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

	param1Valid = param2Valid = false;

	// check for special variables
	if(strcmp(param1Buf, AUTH_UID_STRING) == 0) {
		param1Valid = true;
		prereq->param1.type = PTYPE::AUTH_UID;
		prereq->param1.value = nullptr;
	}

	if(strcmp(param2Buf, AUTH_UID_STRING) == 0) {
		param2Valid = true;
		prereq->param2.type = PTYPE::AUTH_UID;
		prereq->param2.value = nullptr;
	}

	
	// check variable types
	for(unsigned int i = 0; i < rule->numPathVars && (!param1Valid || !param2Valid); i++) {
		if(!param1Valid && strcmp(rule->pathVariables[i], param1Buf) == 0) {
			param1Valid = true;
			prereq->param1.type = PATH_VAR;
		}

		if(!param2Valid && strcmp(rule->pathVariables[i], param2Buf) == 0) {
			param2Valid = true;
			prereq->param2.type = PATH_VAR;
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