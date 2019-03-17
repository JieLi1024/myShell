#ifndef simplecommand_hh
#define simplecommand_hh

#include <string>
#include <vector>
#define MAXFILENAME 1024


// struct ArgCollector {
// 	//stores and sorts wildcard expanded arguments
// 	int maxArgs;
// 	int nArgs;
// 	char** argArray;
//   char* currentArg;
  
 

// 	ArgCollector();

// 	//void addArg( char* arg );
//   void addArg();
// 	void sortArgs();
// 	void clear();

// 	static ArgCollector _currentCollector;
// };

struct SimpleCommand {

  // Simple command is simply a vector of strings
  std::vector<std::string *> _arguments;

  //static ArgCollector *_currentArgCollector;
  //static char* currentArg;

  SimpleCommand();
  ~SimpleCommand();
  void insertArgument( std::string * argument );
  void envExpansion(std::string * argument);
  void tildeExpansion(std::string * argument);
  void print();
};

#endif
