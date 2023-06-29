/*
 * A wrapper that gets mustplay information from benzene
 * Written by Luke Schultz
 * June 2023
 */

#include <iostream>
#include <string.h>
#include <vector>

#define MAXLINE 2000
#define BLANK   0
#define BLACK   1
#define WHITE   2

void initialize(std::string);
void runParent();
std::vector<bool> getMustplay(uint8_t, uint8_t*, bool);
bool isTerminal(uint8_t, uint8_t*, bool);
void runTests();