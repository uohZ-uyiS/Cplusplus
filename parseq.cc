#include <vector>
#include <queue>
#include <string>
#include <iostream>
#include <cstdarg>
#include "parseq.h"
using namespace std;

/*void printQue(queue<string> queryQ){
	while (!queryQ.empty()) { 
		cout << 'M' << queryQ.front()<<endl;
		queryQ.pop(); 
	}
}*/

//translate query string into queue, divide by[ : ],[*]

string getSecondE(queue<string> queryQ){
//cout<<"B1:"<<queryQ.size()<<endl;
	if(queryQ.size()>1){
		queryQ.pop();
		return queryQ.front();
	}else{
		return "END";
	}
}
queue<string> generateQ(string s, string& delimiter1, string& delimiter2, string& delimiter3){
	queue<string> queryQ;
	string token="";
	int pos = -1;
	int posCopy = -1;
	int prepos=-1;
	int aftpos = 0;

	while (( s.find(delimiter1) != string::npos) || ( s.find(delimiter2) != string::npos)||( s.find(delimiter3) != string::npos)) {
		int order=-1;
		

		if(s.find(delimiter1)<=s.find(delimiter2) && s.find(delimiter1)<=s.find(delimiter3)){
			order =1;//star first
		}else if(s.find(delimiter2)<=s.find(delimiter1) && s.find(delimiter2)<=s.find(delimiter3)){
			order =2;//[:]
			}else {
				order =3;
				}
		switch(order){
			case 1:
				pos=s.find(delimiter1); 
				token = s.substr(0, pos);
				if(token.length()>0){
					queryQ.push(token);
				}
				queryQ.push("[*]");
 				s.erase(0, pos + delimiter1.length());
				if(s.find(delimiter1) == -1 && s.find(delimiter2) == -1 && s.find(delimiter3) == -1 && s.length()!=0 ){
					queryQ.push(s);
				}
				break;

			case 2:
				//[ : ]
				pos=s.find(delimiter2);
				posCopy = pos;
				while(pos>-1 && s[pos]!='['){
					--pos;
				} 
				prepos = pos;
				while(posCopy<s.length() && s[posCopy]!=']'){
					++posCopy;
				} 
				aftpos = posCopy;
				token = s.substr(0, prepos);
				if(token.length()>0){
					queryQ.push(token);
				}
				queryQ.push(s.substr(prepos, aftpos-prepos+1));
 				s.erase(0, aftpos+1);
				if(s.find(delimiter1) == -1 && s.find(delimiter2) == -1 && s.find(delimiter3) == -1 && s.length()!=0){
					queryQ.push(s);
				}
				break;
			case 3:	
				pos=s.find(delimiter3) ; 
				token = s.substr(0, pos);
				if(token.length()>0){
					queryQ.push(token);
				}	
				queryQ.push("..");
 				s.erase(0, pos + delimiter3.length());
				
				if(s.find(delimiter1) == -1 && s.find(delimiter2) == -1 && s.find(delimiter3) == -1 && s.length()!=0){
					queryQ.push(s);
				}
				break;
		}
	}//while
	//printQue(queryQ);
	return queryQ;
    
}


//for the query with [*],[ : ], check whether it is match the current json path
//if match the functiom will return number cResult!=-1
int checkSeq(queue<string>& queryQ, string& path){
	int cResult= -1;
	//pos: track the path string
	int pos=0;
	//match: track whether the current element in Q equal to substring of json path
	int match=-1;
	// the start value of [ : ]
	int rs =-1;
	// the end value of [ : ]
	int re =-1;
	//the index in path
	int index = -1;
	int indexValue = -1;
	string si=queryQ.front();
	int siLength = si.length();
	while(!queryQ.empty() && siLength!=0 && pos<(path.length()-1)){
		if(si=="[*]"){
			cResult=0;
			if(path[pos]=='['){
				while(path[pos]!=']'&&pos<path.length()){
					++pos;
				}
            			++pos;
			}else{
				cResult=-1;
				return cResult;
			}
            		queryQ.pop();
        	}else if(si[0]=='[' &&si[siLength-1]==']'){
				int temp = si.find(':');
				rs = stoi(si.substr(1,temp));
				re = stoi(si.substr(temp+1,siLength));
				// the sart of [ in path
				index = pos;
				while(path[pos]!=']'){
					++pos;
				}
				indexValue = stoi(path.substr(index+1, pos));
				if(indexValue<re && indexValue>= rs){
					cResult = 2;		
				}else{
					cResult =-1;
					return cResult;
					}
            			++pos;
            			queryQ.pop();

        		}else if(si==".."){
					cResult=3;
					queryQ.pop();
					string sj=queryQ.front();
					string subpath="";
					subpath=path.substr(pos, path.length()-1);

					if(sj=="[*]"){
						int temparr=-1;
						if(subpath.find('[') != string::npos && getSecondE(queryQ) ==".."){
							//..[*]..text
							temparr = subpath.find(']'); 
							pos= pos+ temparr+1;
							queryQ.pop();
							cResult=5;
							
						}else if(subpath.find('[') != string::npos && getSecondE(queryQ)[0] =='.'){
							//..[*].text
							queryQ.pop();
							bool matcharr=false;
							while(subpath.find(']')!= string::npos && matcharr== false){
								temparr = subpath.find(']'); 
								pos= pos+ temparr+1;
								subpath=path.substr(pos, path.length()-1);
								if(subpath.find(queryQ.front()) != string::npos){
									matcharr=true;
									pos += queryQ.front().length();
									subpath=path.substr(pos, path.length()-1);
									queryQ.pop();
								}
//cout<<"A1:"<<matcharr<<endl;
							}
//cout<<"A2:"<<subpath<<","<<queryQ.front()<<endl;
							cResult=6;
						}else if(subpath.find('[') != string::npos && getSecondE(queryQ) =="END"){
							//..[*] path: have.text[0], path: have[0].text
							cResult =7;
							return cResult;
						}else{
							//path don't have []
							cResult=-1;
							return cResult;
						}
					}else
					{

					// in case r[0]..id == r[0]..idtr == r[0]..trid
						bool checkp = false;
						bool checka = false;
						if(subpath.find(sj) != string::npos){
						int temp = -1;
						do{
							temp=subpath.find(sj);
							pos = pos + temp + sj.length();
							checkp = (subpath[temp-1]=='.');
							checka = (path[pos]=='.'||path[pos]=='['||path[pos]==':');
							subpath = subpath.substr(temp+sj.length(), subpath.length()-1);
						}while(subpath.find(sj) != string::npos && (checkp==false || checka == false));

						}//if
						if(checka && checkp){
							cResult=4;
							queryQ.pop();
						}else{
							cResult=-1;
							return cResult;
						}
					}

				} else {

					match=path.compare(pos,si.length(),si);
					//in case r[0]..id[*].te == path:r[0]..id[*].test		
					bool checkd=(path[pos+si.length()]=='.'||path[pos+si.length()]=='['||path[pos+si.length()]==':');
					if(match==0 && checkd){
						cResult=1;
						pos+=si.length();
						queryQ.pop();
					}else{
						cResult = -1;
						return cResult;
					}  

		}//if

		if(!queryQ.empty()){
			si=queryQ.front();
			siLength = si.length();
		}else{
			return cResult;
			}
 		
	}
	return cResult;
}

int compareSeq(string query, string delimiter1,string delimiter2, string delimiter3,string& path){
	int cseq; 
	queue<string> queryQue;
	queryQue = generateQ(query, delimiter1,delimiter2,delimiter3);
	cseq = checkSeq(queryQue, path);
	//cout<<cseq<<endl;
	return cseq;
}
/*int main()
{
string query = "..root[*]..[*].text";
string path = ".root[15].aliases.en[1].text:";
string delimiter1 = "[*]";
string delimiter2 = ":";
string delimiter3 = "..";
//queue<string> test= generateQ(query, delimiter1,delimiter2,delimiter3);
//checkSeq(test,path);
compareSeq(query, delimiter1,delimiter2,delimiter3,path);
return 1;
 
}*/
