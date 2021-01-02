#pragma once

/**
 * Author: Ryan Steinwert
 *
 * Definitions for database rule manager
 */
#include <vector>
#include <string>


enum PTYPE {
	NULL_VALUE,
	STRING,
	NUMBER,
	PATH_VAR,
	AUTH_UID
};


enum OPERATOR {
	EQUAL,
	LESS_THAN,
	GREATER_THAN,
	LESS_EQUAL,
	GREATER_EQUAL
};



typedef struct param_t {
	PTYPE type;
	char* value;
} param_s;


typedef struct prereq_t {
	param_s param1;
	param_s param2;
	OPERATOR op;
	bool hasCheck;
	bool read;
	bool write;
	prereq_t* next;
} prereq_s;


typedef struct rule_t {
	int pathSize;
	int numPathVars;
	char** collectionPath;
	char** pathVariables;
	prereq_s* prereq;
} rule_s;


class CSDBRuleManager {
public:
	CSDBRuleManager();
	~CSDBRuleManager();

	int loadRules(const char* path);

	bool hasPerms(const char* path, const char* uid, const char* perms);

private:
	std::vector<rule_s*> rules;

	int parseMatch(FILE* file);

	int parsePrereq(char* buf, rule_s* rule);

	void initRule(rule_s* rule);
	void addPrereq(rule_s* rule, prereq_s* prereq);

	bool isPathMatch(std::vector<std::string> pathVector, rule_s rule);
};