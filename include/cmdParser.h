#ifndef PARSER_H
#define PARSER_H

#include <AppState.h>
#include <stdint.h>

class Parser
{
public:
    Parser();
    void parseCommand(const char *cmd, AppState &state);

private:
    void printHelp();
    int parseInteger(const char *str, const char **endptr);
};

#endif // PARSER_H
