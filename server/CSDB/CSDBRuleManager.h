#pragma once

/**
 * Author: Ryan Steinwert
 *
 * Definitions for database rule manager
 */
#include <vector>

enum PTYPE {
	NULL_VALUE,
	STRING,
	NUMBER,
	COLLECTION_NAME,
	AUTH_USER
};


enum OPERATOR {
	EQUAL,
	LESS_THAN,
	GREATER_THAN,
	LESS_EQUAL,
	GREATER_EQUAL
};


union param_value {
	char* str;
	char* matchVariable;
	long num;
};


typedef struct param_t {
	PTYPE type;
	union param_value value;
} param_s;


typedef struct prereq_t {
	param_s param1;
	param_s param2;
	OPERATOR op;
	prereq_t* next;
} prereq_s;


typedef struct rule_t {
	int pathSize;
	int numPathVars;
	char** collectionPath;
	char** pathVariables;
	bool read;
	bool write;
	prereq_s* prereq;
} rule_s;


class CSDBRuleManager {
public:
	CSDBRuleManager();
	~CSDBRuleManager();

	int loadRules(const char* path);

private:
	std::vector<rule_s> rules;

	int parseMatch(FILE* file);

	void initRule(rule_s* rule);
	void addPrereq(rule_s* rule, prereq_s* prereq);
};