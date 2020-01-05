
#ifndef _PARSEQ_H
#define _PARSEQ_H
#include <vector>
#include <queue>
#include <string>
#include <iostream>
using namespace std;
//void printQue(queue<string> queryQ);
string getSecondE(queue<string> queryQ);
queue<string> generateQ(string s, string& delimiter1,string& delimiter2,string& delimiter3);
int checkSeq(queue<string>& queryQ, string& path);
int compareSeq(string query, string delimiter1,string delimiter2,string delimiter3, string& path);
#endif
