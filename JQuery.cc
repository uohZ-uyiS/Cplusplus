/***********************************************
	1.this programming is for json query tool form scratch. Three fils included: JQuery.cc, parseq.h, parse.cc. 
	2.This is V4 version:remove recursion, optimize the memory usage, add feature: parse the query include(..[*]..), haven't do the parse for the value in 		  nested arrray .
	3.for the query string, [*], * , .. , [ : ] and exactly same path can find the matched results.
    4.test file: test.json, random.json, twitter.json, bb.json, wiki.json, rowstest.json, wiki3.json, wiki524.json
	5.test and develope on Ubuntu 16.04, g++ 5.4.0, C++11
	6.Execution : g++ -std=c++11 parseq.cc JQuery.cc  -o output
	8. measure memory usyage by valgrind --tool=massif --pages-as-heap=yes ./output
	9. Athour by Siyu Zhou
***********************************************/

#include <iostream>
#include <cstdarg>
#include <fstream>
#include <string>
#include <cctype>
#include <algorithm>
#include <vector>
#include <exception>
#include <regex>
#include <unistd.h>
#include "parseq.h"
#include <chrono> 
using namespace std;
using namespace std::chrono;

//trim the space at the end of each line of the input file
//@str is the read from json file by lines
string& Trim(string &str){
	if(str.empty()){
		return str;
	}
	str.erase(0, str.find_first_not_of(" "));
	str.erase(str.find_last_not_of(" ") +1);
	str.erase(0, str.find_first_not_of('\t'));
	str.erase(str.find_last_not_of('\t') +1);
	//for rowatest.json
	str.erase(0, str.find_first_not_of('\r'));
	str.erase(str.find_last_not_of('\r') +1);
	return str;

}

void removeSpace(string& lineString, string rs, string newRs){
	while(lineString.find(rs) != string::npos){			
		lineString.replace(lineString.find(rs),rs.length(), newRs);
	}

}

void readLine(ifstream& infile, string& lineString){
	if(!infile.eof()){
		do{
	        getline(infile,lineString);
		Trim(lineString);
		removeSpace(lineString, ": ", ":");
		removeSpace(lineString, "[ ", "[");
		removeSpace(lineString, "] ", "]");
		removeSpace(lineString, ", ", ",");
		removeSpace(lineString, " \"", "\"");
		removeSpace(lineString, "} ", "}");
		removeSpace(lineString, " :", ":");
		removeSpace(lineString, " ]", "]");
		//cout<<"CurrentLine:"<<lineString.length()<<lineString<<endl;
		}while(lineString.length()==0 && !infile.eof());
		//cout<<"Return:"<<lineString<<endl;
		return;
	}else return;
}

//find the correct address for the pointer p
//&p not change
//change: *p, infile
//p = Pointer(p,infile,lineString)
//if want to chek p++, p=p++; Pointer(p)
char*& Pointer(ifstream& infile, char*& p, string& lineString){
	if(*p=='\0'){
		readLine(infile, lineString);
		p = &lineString[0];
	}
	return p;
}

//get value of *(p+1) without change pointer address
//nothing changed
//if want to get *(p+1), should p=p+1, getWo(p+1)
char getWo(char* p, ifstream& infile){
	if(*p=='\0'){
		string lineString;
		streampos pre = infile.tellg();
		readLine(infile,lineString);
		p = &lineString[0];
		infile.seekg(pre);
		return *p;
	}
	return *p;
}
//input p, output *(p+1)
//p not changed
char getNextVa(char* p, ifstream& infile){
	p=p+1;
	if(*p=='\0'){
		string lineString;
		streampos pre = infile.tellg();
		readLine(infile,lineString);
		p = &lineString[0];
		if(!infile.eof()){
		infile.seekg(pre);}
		return *p;
	}
	return *p;
}

char getNextTwoVa(char* p, ifstream& infile){
	streampos pre=infile.tellg();	
	p=p+1;
	string lineString;
	if(*p=='\0'){
		readLine(infile,lineString);
		p = &lineString[0];
	}
	p=p+1;
	if(*p=='\0'){
		readLine(infile,lineString);
		p = &lineString[0];
	}

	if(!infile.eof()){
		infile.seekg(pre);}
	return *p;
	
}
char getNextThirdVa(char* p, ifstream& infile){
	streampos pre=infile.tellg();	
	p=p+1;
	string lineString;
	if(*p=='\0'){
		readLine(infile,lineString);
		p = &lineString[0];
	}
	p=p+1;
	if(*p=='\0'){
		readLine(infile,lineString);
		p = &lineString[0];
	}
	p=p+1;
	if(*p=='\0'){
		readLine(infile,lineString);
		p = &lineString[0];
	}

	if(!infile.eof()){
		infile.seekg(pre);}
	return *p;
	
}


string vectorToString(vector<string> v){
	string result="";
	for(auto const& e:v){
		result+=e;	
	}
	return result;
}

//update the index in [] for the json parse
//the number of . after [] denotes the number of elements in the array
void updateIdex(vector<string>& trackatt){
	while(find(trackatt.begin(), trackatt.end(),"[]")!=trackatt.end()){
		vector<string>::iterator itr= find(trackatt.begin(), trackatt.end(),"[]");
		//get the position of the match
		int pos = distance(trackatt.begin(), itr);
		if(itr!=trackatt.cend()){
			++itr;
			int i=0;
			while(*itr =="."){
				++itr;
				++i;
			}
			
			trackatt[pos]="["+to_string(i-1)+"]";
			if(i>1){
				//delete duplicate .
				trackatt.erase(trackatt.begin()+pos+1,trackatt.begin()+pos+i);
			}
		}
	}//while
}

//check parent finished the parse 
//void checkparent(string& jsonString, vector<string>& trackatt,int i){
void checkparent(ifstream& infile, char* p, vector<string>& trackatt,string lineString){
	int k =-1;
	while(*p=='}'&& trackatt.size()>1){
		int vback = trackatt.size()-1;
		if(*p =='}'&&getNextVa(p, infile)!=']' ){
			if(trackatt.back()=="."){
				while(trackatt[vback]=="."&&vback>-1){
					vback+=k;
				}
				if(trackatt[vback]!="[]"){
					trackatt.pop_back();
					trackatt.pop_back();
				}

			}
		}else if(*p=='}'&& getNextVa(p, infile)==']'){
		    	while(trackatt.back()=="."||trackatt.back()=="[]"){
				trackatt.pop_back();
			}
			trackatt.pop_back();
			++p;
			p=Pointer(infile, p, lineString);
			
		}
		++p;
		p=Pointer(infile, p, lineString);
	}
}

void detectKey(ifstream& infile, char*& p, string& lineString, string& key, vector<string>& trackatt){
	if(*p=='"'){
		while(getNextVa(p, infile)!='"'){
			++p;
			p=Pointer(infile, p, lineString);
			key += *p;
		}
		trackatt.push_back(key);
		++p;
		p=Pointer(infile, p, lineString);
		++p;
		p=Pointer(infile, p, lineString);

	}else return;
	return;
}

void matchQuery(string query,string& value, vector<string>& trackatt){		
	vector<string> pathVector;
	string pathString="";
	pathVector =trackatt;
	//index in trackatt should be update,since it should need to keep track of whole jsonString
	//update index for [] in pathVector and get the path for pathString
	updateIdex(pathVector);
	pathString = vectorToString(pathVector);
//cout<<"PathString "<<pathString<<endl;
	pathVector.clear();
//cout<<"Value "<<value<<endl;
	trackatt.pop_back();
	trackatt.pop_back();
	//search for the match
	//if query is ..*
	if(query == "..*"){
		cout<<pathString<<value<<endl;
		return;
	}
	
	//if the last char of query is *
	char lastChar= query[query.length()-1];
	char slastChar= query[query.length()-2];
	auto lastTwo = string(1,slastChar) + lastChar;
	if(lastTwo==".*"){
		query = query.substr(0,query.length()-2);
	}

	//if[*],[:]exist, call the parseQ fuction
	if(query.find("[*]") != string::npos || query.find(":") != string::npos ||query.find("..") != string::npos){
	//exist
		int m=-1;
		try{
			m = compareSeq(query, "[*]",":","..", pathString);
		}catch(exception e){
			cout<<"Catch you"<<endl;
		}
		if(m!=-1){
			cout<<pathString<<value<<endl;
		}
		return;
	}else{
	//do exatly match
		int index;
		index = -1;
		if(pathString.find(query)!= string::npos){
			index=pathString.find(query);
		}else{
			return;
		}
		bool check = (pathString[index+query.length()]=='.'||pathString[index+query.length()]=='['||pathString[index+query.length()]==':');
		if(index==0 && check){
				cout<<pathString<<value<<endl;
					
		}
		return;
	}
	return;

}

//parse json
//@jsonString: output from readJson() function
//@trackatt: keep track of the path for current attribute
//@i: the index of the char in the string
//@length:the length of the jsonString
void JQuery(string& query,ifstream& infile, string& lineString, vector<string>& trackatt, char*& p){	
	string value="";
	string key="";
	switch(*p){
		case '{':
			++p;
			p=Pointer(infile, p, lineString);
			trackatt.push_back(".");
			detectKey(infile,p,lineString, key,trackatt);
			break;

		case '[':
			++p;
			p=Pointer(infile, p, lineString);	
			//if current attribute is for array, push [] into the pathVector, update the index later
			if(*p=='{'||*p =='['|| isspace(*p)){
				trackatt.push_back("[]");
			}
			break;
		case ':':

			value="";
			if(getNextVa(p,infile)!='{' && getNextVa(p,infile)!='['){
				p = p+1;
				p = Pointer(infile, p, lineString);
				// "key":"test, test", "key":true
				if(*p=='"'){
					value = value + '"';
					while(getNextVa(p,infile)!='"'||*p=='\\'){
						value += getNextVa(p,infile);
						p++;
						p = Pointer(infile, p, lineString);
					}
					value +='"';
					p++;
					p = Pointer(infile, p, lineString);
				}else{
					//"key":true
					while(getNextVa(p,infile)!='}'&&getNextVa(p,infile)!=','){
						value += *p;
						++p;
						p = Pointer(infile, p, lineString);
						}
					value += *p;
					}
					
				trackatt.push_back(":");
				matchQuery(query,value, trackatt);
				//prepare a para ti checkparent(), should not modify infile and *p
				char* q4;
				q4=p;
				streampos pre4 = infile.tellg();
				q4++;
				q4 = Pointer(infile, q4, lineString);
				checkparent(infile, q4, trackatt, lineString);
				if(!infile.eof()){infile.seekg(pre4);}
			}else
			if(getNextVa(p,infile)=='[' && getNextTwoVa(p,infile)!='{'){
				//"key":[2,3],"key":[[2,3],[4,6]],"key":["ac","ab"],"key":[["ac","ab"],["ae","ad"]]
				value = value + '[';
				++p;
				p = Pointer(infile, p, lineString);
				++p;
				p = Pointer(infile, p, lineString);
				while((*p !=']' || getNextVa(p,infile)!=','|| getNextTwoVa(p,infile)!='"')&&(*p!=']' || getNextVa(p,infile)!='}')){
					value += *p;
					++p;
					p = Pointer(infile, p, lineString);
				}	
				value +=']';
				trackatt.push_back(":");
				matchQuery(query,value, trackatt);
				char* q3;
				q3=p;
				streampos pre3 = infile.tellg();
				++q3;
				q3 = Pointer(infile, q3, lineString);
				checkparent(infile,q3,trackatt, lineString);
				if(!infile.eof()){infile.seekg(pre3);}
			}else
			//handle special case "key":[],"key":{ }
			if(getNextVa(p,infile)=='[' && getNextTwoVa(p,infile)==']'){
				++p;
				p = Pointer(infile, p, lineString);	
				trackatt.push_back(":");
				value="[]";
				matchQuery(query,value, trackatt);
				char* q2; 
				q2 = p;
				streampos pre2 = infile.tellg();
				int temp2=0;
				while(temp2<2){
					++temp2;
					++q2;
					q2 = Pointer(infile, q2, lineString);
				}
				checkparent(infile,q2,trackatt, lineString);		
				if(!infile.eof()){infile.seekg(pre2);}
				++p;
				p = Pointer(infile, p, lineString);
				return;
			}else 
			//andle special case "key":{ }
			if(getNextVa(p,infile) =='{' && getNextTwoVa(p,infile)==' ' && getNextThirdVa(p,infile)=='}'){
				++p;
				p = Pointer(infile, p, lineString);			
				trackatt.push_back(":");
				value="{ }";
				matchQuery(query,value, trackatt);
				char* q1; 
				q1 = p;
				streampos pre1 = infile.tellg();
				int temp1=0;
				while(temp1<3){
					++temp1;
					++q1;
					q1 = Pointer(infile, q1, lineString);
				}
				checkparent(infile,q1,trackatt, lineString);
				if(!infile.eof()){infile.seekg(pre1);}
				++p;
				p = Pointer(infile, p, lineString);
			}
			++p;
			p = Pointer(infile, p, lineString);
			return;
			break;
		case ',':
			++p;
			p=Pointer(infile, p, lineString);
			detectKey(infile,p,lineString, key,trackatt);		
			break;

		default :
			p=p+1;
			p = Pointer(infile, p, lineString);
			detectKey(infile,p,lineString, key,trackatt);
			break;
		
	}//switch
return;

}



int main(){
	auto start = high_resolution_clock::now();
	string lineString;
	ifstream infile;
	infile.open ("wiki524.json");
	string query =".root[*].pageid";

	char *p;
	readLine(infile,lineString);
	p = &lineString[0];
	vector<string> trackatt;
	while(*p!='\0'){
		JQuery(query,infile, lineString, trackatt, p);
	}
	infile.close();
	cout<<"Done";
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<seconds>(stop - start); 
	cout << "Time taken by function: "<< duration.count() << " seconds" << endl; 
	return 1;
}   

