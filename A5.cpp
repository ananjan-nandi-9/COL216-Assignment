#include <bits/stdc++.h>

using namespace std;
vector < map < string, int >> reg;
map < string, int > cnt;
vector < map < int, string >> command;
vector < int > cur, order;
int num = 0;
int sclock = 0;
map < string, int > res_reg, res_wrd;
map < int, char > hexa;
vector < map < string, int >> labels, reg_use;
deque < pair < vector < string > , vector < int >>> requests[8]; //dram request queue
vector<string>isblock(8,"-");
vector<float>metric(8);
map<pair<int,string>,int>blockcnt;
int curexc=-1; 
string dram[1024][1024];
int row_buffer = -1;
int updates = 0;
bool executing = 0;
int row_delay, column_delay, loc;
int dramtime = 0;
int rc = 0, cc = 0;
int executelw(vector < string > v, vector < int > line, int i);
int executesw(vector < string > v, vector < int > line, int i);
vector < map < string, vector < string >>> dep; //maintains dependencies
int N, M;
int curval = 0;
int curcycle = 0;
int curcore = -1;
int curtype = -1;
string curreg = "lorem";
int curmem = -1;
int currow = -1;
int curcol = -1;
vector<int> swcnt;
vector<bool> e;
vector<int> mrmtime;
bool pending,waiting=0;
int currowcnt=0;
int switchtime=0;
map<string,int> lwrow[8];
bool fw[8];
pair<string,int> fwinfo[8];

bool comp(pair < vector < string > , vector < int >> a, pair < vector < string > , vector < int >> b) //added
{
    if (a.second[3] < b.second[3]) return false;
    else if (a.second[3] > b.second[3]) return true;
    else return a.second[0] > b.second[0];
}
string get_hexa(int d) {
    unsigned int g;
    if (d < 0)
        g = pow(2, 32) + d;
    else g = d;
    string ret = "";
    while (g != 0) {
        int w = g % 16;
        g /= 16;
        ret.push_back(hexa[w]);
    }
    std::reverse(ret.begin(), ret.end());
    if (ret == "") ret = "0";
    return ret;
}
string eraseTrail(string s) {
    int low = -1, high = s.size();
    for (int i = 0; i < s.size(); ++i) {
        if (s[i] != ' ' && s[i] != '\t')
            break;
        low = i;
    }
    for (int i = s.size() - 1; i >= 0; --i) {
        if (s[i] != ' ' && s[i] != '\t')
            break;
        high = i;
    }
    return s.substr(low + 1, high - low - 1);
}
bool isReg(string s, int line, int i) {
    if (res_reg[s] == 0) {
        cout << "Invalid Register Name, error in instruction: " << line << '\n';
        e[i]=true;
	return true;
    }
    return false;
}
int str2intc(string s, int line, int j) {
    int ans = 0, i = 0;
    if (s[0] == '-') {
        i = 1;
        if (s.size() == 1) {
            cout << "Expected an integer, error in instruction: " << line << '\n';
            e[j]=true;
	    return 0;
        }
    }
    for (; i < s.size(); ++i) {
        int val = (s[i] - '0');
        if (val < 0 || val > 9) {
            cout << "Expected an integer, error in instruction: " << line << '\n';
            e[j]=true;
	    return 0;
        }
        ans *= 10;
        ans += val;
    }
    if (s[0] == '-') {
        return -ans;
    }
    return ans;
}

int str2int(string s, int line) {
    int ans = 0, i = 0;
    if (s[0] == '-') {
        i = 1;
        if (s.size() == 1) {
            cout << "Expected an integer, error in instruction: " << line << '\n';
            exit(0);
        }
    }
    for (; i < s.size(); ++i) {
        int val = (s[i] - '0');
        if (val < 0 || val > 9) {
            cout << "Expected an integer, error in instruction: " << line << '\n';
            exit(0);
        }
        ans *= 10;
        ans += val;
    }
    if (s[0] == '-') {
        return -ans;
    }
    return ans;
}

vector < string > split(string s) {
    string cur = "";
    s += ',';
    vector < string > ans;
    for (int i = 0; i < s.size(); ++i) {
        if (s[i] == ',') {
            ans.push_back(cur);
            cur = "";
        } else
            cur += s[i];
    }
    return ans;
}
void get() //changed
{
    cout << '\n';
    cout << "Number of instructions: " << num << "\n";
    cout << "Number of row buffer updates: " << updates << "\n";
    for (auto i: cnt)
        cout << "Number of times " << i.first << " was executed is: " << i.second << "\n";
    for (int j = 0; j < N; ++j) {
        cout << "Core " << j << '\n';
        for (int i = 0; i < 10; ++i) {
            string req = "$s" + to_string(i);
            cout << "Value of " << req << " is: " << get_hexa(reg[j][req]) << "\n";
        }
        for (int i = 0; i < 10; ++i) {
            string req = "$t" + to_string(i);
            cout << "Value of " << req << " is: " << get_hexa(reg[j][req]) << "\n";
        }
    }
}

void executeadd(vector < string > v, vector < int > line, int i) {
    if (v.size() != 3) {
        cout << "add command needs 3 arguments, error in instruction: " << line[i] << '\n';
        e[i]=true; return;
    }
    if (isReg(v[0], line[i], i)||
    isReg(v[1], line[i], i)||
    isReg(v[2], line[i], i)) return;
    if (v[0] == "$zero") {
        cout << "Cannot manipulate $zero: " << line[i] << '\n';
        e[i]=true; return;
    }
    dep[i][v[0]] = vector < string > ({
        v[1],
        v[2]
    });
    reg[i][v[0]] = reg[i][v[1]] + reg[i][v[2]];
    cout << i << " " << v[0] << " " << get_hexa(reg[i][v[0]]) << '\n';
    num++;
}

void executeaddi(vector < string > v, vector < int > line, int i) {
    if (v.size() != 3) {
        cout << "addi command needs 3 arguments, error in instruction: " << line[i] << '\n';
        e[i]=true; return;
    }
    if (isReg(v[0], line[i], i)||
    isReg(v[1], line[i], i)) return;
    int x = str2intc(v[2], line[i], i);
    if (e[i]) return;
    if (v[0] == "$zero") {
        cout << "Cannot manipulate $zero: " << line[i] << '\n';
        e[i]=true; return;
    }
    dep[i][v[0]] = vector < string > ({
        v[1]
    });
    reg[i][v[0]] = reg[i][v[1]] + x;
    cout << i << " " << v[0] << " " << get_hexa(reg[i][v[0]]) << '\n';
    num++;
}

void executesub(vector < string > v, vector < int > line, int i) {
    if (v.size() != 3) {
        cout << "sub command needs 3 arguments, error in instruction: " << line[i] << '\n';
        e[i]=true; return;
    }
    if (isReg(v[0], line[i], i)||
    isReg(v[1], line[i], i)||
    isReg(v[2], line[i], i)) return;
    if (v[0] == "$zero") {
        cout << "Cannot manipulate $0: " << line[i] << '\n';
        e[i]=true; return;
    }
    dep[i][v[0]] = vector < string > ({
        v[1],
        v[2]
    });
    reg[i][v[0]] = reg[i][v[1]] - reg[i][v[2]];
    cout << i << " " << v[0] << " " << get_hexa(reg[i][v[0]]) << '\n';
    num++;
}

void executemul(vector < string > v, vector < int > line, int i) {
    if (v.size() != 3) {
        cout << "mul command needs 3 arguments, error in instruction: " << line[i] << '\n';
        e[i]=true;
	return;
    }
    if (isReg(v[0], line[i], i)||
    isReg(v[1], line[i], i)||
    isReg(v[2], line[i], i)) return;
    if (v[0] == "$zero") {
        cout << "Cannot manipulate $zero: " << line[i] << '\n';
        e[i]=true; return;
    }
    dep[i][v[0]] = vector < string > ({
        v[1],
        v[2]
    });
    reg[i][v[0]] = reg[i][v[1]] * reg[i][v[2]];
    cout << i << " " << v[0] << " " << get_hexa(reg[i][v[0]]) << '\n';
    num++;
}

int executebeq(vector < string > v, vector < int > line, int i) {
    if (v.size() != 3) {
        cout << "beq command needs 3 arguments, error in instruction: " << line[i] << '\n';
        e[i]=true;
	return line[i];
    }
    if (isReg(v[0], line[i], i)||
    isReg(v[1], line[i], i)) return line[i];
    if (labels[i].find(v[2]) == labels[i].end()) {
        cout << "Label not found: " << line[i] << '\n';
        e[i]=true;
	return line[i];
    }
    int x = labels[i][v[2]];
    if (reg[i][v[0]] == reg[i][v[1]]) {
        if (x < 0 || x >= cur[i]) {
            cout << "Instruction doesn't exist: " << line[i] << '\n';
            e[i]=true;
	    return line[i];
        }
        num++;
        return x;
    }
    num++;
    return line[i]+1;
}

int executebne(vector < string > v, vector < int > line, int i) {
    if (v.size() != 3) {
        cout << "bne command needs 3 arguments, error in instruction: " << line[i] << '\n';
        e[i]=true;
	return line[i];
    }
    if (isReg(v[0], line[i], i)||
    isReg(v[1], line[i], i)) return line[i];
    if (labels[i].find(v[2]) == labels[i].end()) {
        cout << "Label not found: " << line[i] << '\n';
        e[i]=true;
	return line[i];
    }
    int x = labels[i][v[2]];
    if (reg[i][v[0]] != reg[i][v[1]]) {
        if (x < 0 || x >= cur[i]) {
            cout << "Instruction doesn't exist: " << line[i] << '\n';
            e[i]=true;
	    return line[i];
        }
        num++;
        return x;
    }
    num++;
    return line[i]+1;
}

int executej(vector < string > v, vector < int > line, int i) {
    if (v.size() != 1) {
        cout << "j command needs 1 argument, error in instruction: " << line[i] << '\n';
        e[i]=true;
	return line[i];
    }
    if (labels[i].find(v[0]) == labels[i].end()) {
        cout << "Label not found: " << line[i] << '\n';
        e[i]=true;
	return line[i];
    }
    int x = labels[i][v[0]];
    if (x >= pow(2, 19) || x < 0) {
        cout << "Instruction doesn't exist: " << line[i] << '\n';
        e[i]=true;
	return line[i];
    }
    if (x < 0 || x >= cur[i]) {
        cout << "Instruction doesn't exist: " << line[i] << '\n';
        e[i]=true;
	return line[i];
    }
    num++;
    return x;
}

void executeslt(vector < string > v, vector < int > line, int i) {
    if (v.size() != 3) {
        cout << "slt command needs 3 arguments, error in instruction: " << line[i] << '\n';
        e[i]=true; return;
    }
    if (isReg(v[0], line[i], i)||
    isReg(v[1], line[i], i)||
    isReg(v[2], line[i], i)) return;
    if (v[0] == "$zero") {
        cout << "Cannot manipulate $0: " << line[i] << '\n';
        e[i]=true; return;
    }
    dep[i][v[0]] = vector < string > ({
        v[1],
        v[2]
    });
    if (reg[i][v[1]] < reg[i][v[2]])
        reg[i][v[0]] = 1;
    else
        reg[i][v[0]] = 0;
    cout << i << " " << v[0] << " " << get_hexa(reg[i][v[0]]) << '\n';
    num++;
}

void dramexecute(vector < string > v, int i, int row, int column, bool t) { //added
    if (!t) {
        if (row_buffer == -1) {
            rc = 1;
            dramtime += row_delay;
            cc = 1;
            dramtime += column_delay;
            row_buffer = row;
            updates++;
        } else if (row_buffer == row) {
            cc = 1;
            dramtime += column_delay;
        } else {
            rc = 2;
            dramtime += row_delay;
            dramtime += row_delay;
            cc = 1;
            dramtime += column_delay;
            row_buffer = row;
            updates++;
        }
    } else {
        int val = (1024 * row) + column;
        updates++;
        if (row_buffer == -1) {
            rc = 1;
            dramtime += row_delay;
            cc = 1;
            dramtime += column_delay;
            row_buffer = row;
            updates++;
        } else if (row_buffer == row) {
            cc = 1;
            dramtime += column_delay;
        } else {
            rc = 2;
            dramtime += row_delay;
            dramtime += row_delay;
            cc = 1;
            dramtime += column_delay;
            row_buffer = row;
            updates++;
        }
    }
}

int executelw(vector < string > v, vector < int > line, int i) { //added
    if (v.size() != 2) {
        cout << "lw command needs 2 arguments, error in instruction: " << line[i] << '\n';
        e[i]=true;
	return line[i];
    }
    if (isReg(v[0], line[i], i)) return line[i];
    if (v[0] == "$zero") {
        cout << "Cannot manipulate $0: " << line[i] << '\n';
        e[i]=true;
	return line[i];
    }
    dep[i][v[0]] = vector < string > ({v[0]});
    string temp = v[1];
    eraseTrail(temp);
    int curr = 0;
    bool found = 0;
    if (temp[0] == '-' || isdigit(temp[0])) {
        found = 0;
    } else {
        found = 1;
    }
    for (int i = 1; i < temp.size(); i++) {
        if (!isdigit(temp[i])) {
            found = 1;
            break;
        }
    }
    if (found) {
        if (isdigit(temp[0]) || temp[0] == '(' || temp[0]=='-') {
            if(temp[0]=='-') curr=1;
            while (curr < temp.size() && isdigit(temp[curr])) {
                curr++;
            }
            if (curr == temp.size() || temp[curr] != '(' || temp[temp.size() - 1] != ')') {
                cout <<"Syntax error, instruction: " << line[i] << '\n';
                e[i]=true;
		return line[i];
            } else {
                temp = temp.substr(curr + 1, temp.size() - curr - 2);
            }
            if (res_reg[temp] == 0) {
                cout << "Not a register, error in instruction: " << line[i] << '\n';
                e[i]=true;
		return line[i];
            }
            int offset = str2intc(v[1].substr(0, curr), line[i], i);
            int val = offset + reg[i][temp];
            if (val > 1024*(1024 / N) || val < 0) {
                cout << "Out of Memory, error in instruction: " << line[i] << '\n';
                e[i]=true;
		return line[i];
            }
            if (val % 4 != 0) {
                cout << "Invalid Memory Request:  " << line[i] << '\n';
                e[i]=true;
		return line[i];
            }
            temp = to_string(val);
        } else {

            cout <<"Syntax error, instruction: " << line[i] << '\n';
        }
    }
    if (!found) {
        if (str2intc(temp, line[i], i) > 1024*(1024 / N) || str2intc(temp, line[i], i) < 0 || str2intc(temp, line[i], i) % 4 != 0) {
            cout << "Out of Memory, error in instruction: " << line[i] << '\n';
            e[i]=true;
	    return line[i];
        }
    }
    int val = str2intc(temp, line[i], i);
    val += i * (1024 / N)*1024;
    int row = val / 1024;
    int column = val - (1024 * row);
    int a=1024/N * i, b=1024/N * (i+1);
    if(row>=b || row<a)
    {
            cout << "Out of Memory, error in instruction: " << line[i] << '\n';
            e[i]=true;
	    return line[i];

    }

    if (e[i]) return line[i];
    reg[i][v[0]] = str2intc(dram[row][column], line[i], i);
    v[1] = to_string(val);
    lwrow[i][v[0]]=row;
    int f=-1;
    for(int k=0;k<requests[i].size();++k){
	    if (requests[i][k].second[3]==1 && requests[i][k].second[1]==row && requests[i][k].second[2]==column) 
		    f=k;
    }
    if (currow==row && curcol==column && curtype==1){
	    mrmtime[i]=1;
	    fw[i]=true;
	    fwinfo[i]={v[0],reg[i][curreg]};
	    reg_use[i][v[0]]--;
	    return line[i];
    }
    if (f!=-1){
	    mrmtime[i]=1;
	    fw[i]=true;
	    fwinfo[i]={v[0],reg[i][requests[i][f].first[0]]};
	    reg_use[i][v[0]]--;
	    return line[i];
    }
    requests[i].push_back({v,{i,row,column,0,num,-1}});
    if (currow==row) {
	    currowcnt++;
	    mrmtime[i]=1;
    } else {
	    mrmtime[i]=1;
    }
    cout << "Issued READ to " << v[0] << " on core " << i << " from row " << row << " column " << column << '\n';
    return line[i];
}

int executesw(vector < string > v, vector < int > line, int i) { //added
    if (v.size() != 2) {
        cout << "sw command needs 2 arguments, error in instruction: " << line[i] << '\n';
        e[i]=true;
	return line[i];
    }
    if (isReg(v[0], line[i], i)) return line[i];
    string temp = v[1];
    eraseTrail(temp);
    int curr = 0;
    bool found = 0;
    if (temp[0] == '-' || isdigit(temp[0])) {
        found = 0;
    } else {
        found = 1;
    }
    for (int i = 1; i < temp.size(); i++) {
        if (!isdigit(temp[i])) {
            found = 1;
            break;
        }
    }
    if (found) {
        if (isdigit(temp[0]) || temp[0] == '(' || temp[0]=='-') {
            if(temp[0]=='-') curr=1;
            while (curr < temp.size() && isdigit(temp[curr])) {
                curr++;
            }
            if (curr == temp.size() || temp[curr] != '(' || temp[temp.size() - 1] != ')') {
                cout << "Syntax error, instruction: " << line[i] << '\n';
                e[i]=true;
		return line[i];
            } else {
                temp = temp.substr(curr + 1, temp.size() - curr - 2);
            }
            if (res_reg[temp] == 0) {
                cout << "Not a register, error in instruction: " << line[i] << '\n';
                e[i]=true;
		return line[i];
            }
            int offset = str2intc(v[1].substr(0, curr), line[i], i);
            int val = offset + reg[i][temp];
            if (val > 1024*(1024 / N) || val < 0) {
                cout << "Out of Memory, error in instruction: " << line[i] << '\n';
                e[i]=true;
		return line[i];
            }
            if (val % 4 != 0) {
                cout << "Invalid Memory Request:  " << line[i] << '\n';
                e[i]=true;
		return line[i];
            }
            temp = to_string(val);
        } else {
            cout << "Syntax error, instruction: " << line[i] << '\n';
        }
    }
    if (!found) {
        if (str2intc(temp, line[i], i) > 1024*(1024 / N) || str2intc(temp, line[i], i) < 0 || str2intc(temp, line[i], i) % 4 != 0) {
            cout << "Out of Memory, error in instruction: " << line[i] << '\n';
            e[i]=true;
	    return line[i];
        }
    }
    int val = str2intc(temp, line[i], i);
    val += i * (1024 / N) * 1024;
    int row = val / 1024;
    int column = val - (1024 * row);
    int a=1024/N * i, b=1024/N * (i+1);
    if(row>=b || row<a)
    {
            cout << "Out of Memory, error in instruction: " << line[i] << '\n';
            e[i]=true;
	    return line[i];

    }
    if (e[i]) return line[i];
    dram[row][column] = to_string(reg[i][v[0]]);
    v[1] = to_string(val);
    requests[i].push_back({v,{i,row,column,1,num,reg[i][v[0]]}});
    swcnt[i]++;
    if (currow==row) {
	    currowcnt++;
	    mrmtime[i]=1;
    } else {
	    mrmtime[i]=1;
    }
    cout << "Issued WRITE from " << v[0] << " on core " << i << " to row " << row << " column " << column << '\n';
    return line[i];
}

void execute() //changed
{
    vector < int > line(N, 0);
    while (sclock < M) {
        sclock++;
        cout << '\n';
        cout << "Clock Cycle : " << sclock << '\n';
	bool add=0;
	for (int i=0;i<N;++i) if (mrmtime[i]>0) add=1;
	if (dramtime == 0 && add){
		cout << "MRM Busy with addition of request" << '\n';
	} else if (dramtime == 0) {
	    if (!pending){
            	if(curexc>-1 && curtype==0) reg_use[curcore][curreg]--;
            	if(curexc>-1 && curtype==0) blockcnt[{curexc,curreg}]--;
            	if(curexc>-1 && isblock[curexc]!="-" && blockcnt[{curexc,isblock[curexc]}]==0) isblock[curexc]="-";
            	if(curexc>-1 && requests[curexc].size()==0) curexc=-1;
            	bool bbc=false;
	    	int rd=-1;
            	for(int qw=0;qw<N;++qw) if(!requests[qw].empty()) bbc=true; 
            	if (bbc) {
                	if(currowcnt==0) {
				switchtime+=1;
				curexc=-1;
			} else {
				switchtime+=1;
			}
                	if(curexc==-1)
                	{
                    	for(int cc=0;cc<N;++cc)
                    	{
                            int ccur=order[cc];
                        	if(requests[ccur].size()==0) continue;
                        	if(isblock[ccur]!="-")
                        	{
                            	curexc=ccur;
			    	rd=lwrow[ccur][isblock[ccur]];
                            	break;
                        	}
                    	}
                	}
                	if(curexc==-1)
                	{
                    	int nowsize=0;
                    	for(int dc=0;dc<N;++dc)
                    	{
                        	int cc=order[dc];
                        	if(nowsize<(requests[cc].size()))
                        	{
                            	nowsize=(requests[cc].size());
                            	curexc=cc;
                        	}
                    	}
                	}
                	// reorder below
			if (currowcnt==0){
                		deque < pair < vector < string > , vector < int >>>req1, req2;
                		int takerow;
				if (rd==-1){
					takerow=requests[curexc][0].second[1];
				} else {
					takerow=rd;
				}
                		for(auto ii:requests[curexc])
                		{
                    		if(ii.second[1]==takerow)
                        		req1.push_back(ii);
                   		else 
                        		req2.push_back(ii);
                		}
				currowcnt=req1.size();
                		for(int ii=0;ii<requests[curexc].size();++ii)
                		{
                    		if(ii<req1.size()) requests[curexc][ii]=req1[ii];
                    		else requests[curexc][ii]=req2[ii-req1.size()];
                		}
				if (requests[curexc].size()!=1)
					switchtime+=2*requests[curexc].size();
			}
			pending=1;
		}}
	        if (switchtime>0){
			switchtime--;
			cout << "Deciding next request" << '\n';
		} else {
		if (pending){
			pending=0;
                	// reorder above
                	pair < vector < string > , vector < int >> temp = requests[curexc][0];
			currowcnt--;
                	requests[curexc].pop_front();   
                	executing = 1;
                	curcycle = 0;
               		curcore = temp.second[0];
                	currow = temp.second[1];
                	curcol = temp.second[2];
                	curtype = temp.second[3];
                	curreg = temp.first[0];
			curval = temp.second[5];
                	curmem = 1024*temp.second[1]+temp.second[2];
			if (curtype==1) swcnt[curcore]--;
                	dramexecute(temp.first, temp.second[0], temp.second[1], temp.second[2], temp.second[3]);
                	if (curtype==0){
                    		cout << "Started executing READ to " << curreg << " on core " << curcore << " from row " << temp.second[1] << " column " << temp.second[2] << '\n';
                	} else {
                    		cout << "Started executing WRITE from " << curreg << " on core " << curcore << " to row " << temp.second[1] << " column " << temp.second[2] << '\n';
                	}
            } 
            else {
                cout << "DRAM Idle" << '\n';
                curreg = "lorem";
                        curcore = -1;
                        executing = 0;
                        curcycle = 0;
                curtype=-1;
                curmem=-1;
                currow=-1;
                curcol=-1;
		curval=-1;
            }
        }
	}	
        else {
            if (dramtime==1){
                if (curtype==0){
		    cnt["lw"]++; num++;
                    cout << curreg << " " << get_hexa(reg[curcore][curreg]) << '\n';
		} else {
		    cnt["sw"]++; num++;
                    cout << curmem << " to " << curmem + 3 << " " << get_hexa(curval) << '\n';
		}
            } 
            else {
                if (curtype==0)
                    cout << "Executing READ to " << curreg << " on core " << curcore << " from row " << currow << " column " << curcol << '\n';
                else
                    cout << "Executing WRITE from " << curreg << " on core " << curcore << " to row " << currow << " column " << curcol << '\n';
            }
            dramtime--;
            curcycle++;
            if (rc) {
                if (curcycle == row_delay) 
                    rc--;
            } 
            else if (cc) {
                if (curcycle == column_delay)
                    cc--;
            }
        }
        for (int i = 0; i < N; ++i) {
	    if (mrmtime[i]>0) mrmtime[i]--;
	    if (mrmtime[i]==0 && fw[i]){
		    fw[i]=false;
		    num++;
		    cnt["lw"]++;
		    cout << "Core: " << i << '\n';
		    cout << "Forwarded" << '\n';
		    cout << fwinfo[i].first << " " << get_hexa(fwinfo[i].second) << '\n';
		    continue;
	    }
            cout << "Core: " << i << '\n';
	    if (e[i]){
		    cout << "Error" << '\n';
		    continue;
	    }
            if (line[i] == cur[i] || eraseTrail(command[i][line[i]]) == "" || (curtype == 0 && dramtime == 1 && curcore == i)) {
                cout << "Core Idle" << '\n';
                continue;
            }
            string com = eraseTrail(command[i][line[i]]);
            int space = com.find(" ");
            string todo = com.substr(0, space), args = com.substr(space + 1, com.size() - space - 1);
            vector < string > v = split(args);
            for (int i = 0; i < v.size(); ++i)
                v[i] = eraseTrail(v[i]);
            if (res_wrd[todo] == 0) {
                cout << "Invalid instruction: " << line[i] << '\n';
                e[i]=true;
            }
	    waiting=0;
	    for(int i=0;i<N;++i) if (requests[i].size()>0) waiting=1;
                
            if (todo == "add") {
                if ((reg_use[i][v[1]] || reg_use[i][v[2]])) {
			cout << "Core Idle" << '\n';
            if(reg_use[i][v[1]]) isblock[i]=v[1]; else isblock[i]=v[2];
			continue;
		}
		cout << com << '\n';
		cnt[todo]++;
                executeadd(v, line, i);
		line[i]++;
            } else if (todo == "addi") {
                if ((reg_use[i][v[1]])) {
			cout << "Core Idle" << '\n';
            		isblock[i]=v[1];
			continue;
		}
		cnt[todo]++;
		cout << com << '\n';
                executeaddi(v, line, i);
		line[i]++;
            } else if (todo == "sub") {
                if ((reg_use[i][v[1]] || reg_use[i][v[2]])) {
			cout << "Core Idle" << '\n';
            if(reg_use[i][v[1]]) isblock[i]=v[1]; else isblock[i]=v[2];
			continue;
		}
		cnt[todo]++;
		cout << com << '\n';
                executesub(v, line, i);
		line[i]++;
            } else if (todo == "mul") {
                if ((reg_use[i][v[1]] || reg_use[i][v[2]])) {
                    if(reg_use[i][v[1]]) isblock[i]=v[1]; else isblock[i]=v[2];
			cout << "Core Idle" << '\n';
			continue;
		}
		cnt[todo]++;
		cout << com << '\n';
                executemul(v, line, i);
		line[i]++;
            } else if (todo == "beq") {
                if ((reg_use[i][v[1]] || reg_use[i][v[0]])) {
			cout << "Core Idle" << '\n';
            if(reg_use[i][v[1]]) isblock[i]=v[1]; else isblock[i]=v[0];
			continue;
		}
		cnt[todo]++;
		cout << com << '\n';
                line[i] = executebeq(v, line, i);
            } else if (todo == "bne") {
                if ((reg_use[i][v[1]] || reg_use[i][v[0]])) {
                if(reg_use[i][v[1]]) isblock[i]=v[1]; else isblock[i]=v[0];
			cout << "Core Idle" << '\n';
			continue;
		}
		cnt[todo]++;
		cout << com << '\n';
                line[i] = executebne(v, line, i);
            } else if (todo == "j") {
		cnt[todo]++;
		cout << com << '\n';
                line[i] = executej(v, line, i);
            } else if (todo == "slt") {
                if ((reg_use[i][v[1]] || reg_use[i][v[2]])) {
                    if(reg_use[i][v[1]]) isblock[i]=v[1]; else isblock[i]=v[2];
			cout << "Core Idle" << '\n';
			continue;
		}
		cnt[todo]++;
		cout << com << '\n';
                executeslt(v, line, i);
		line[i]++;
            } else if (todo == "lw") {
		if (dramtime==0 && waiting) {
			cout << "DRAM Waiting" << '\n';
			continue;
		}
		if (mrmtime[i]!=0 || switchtime!=0){
			cout << "MRM busy" << '\n';
			continue;
		}
		if (requests[i].size()>=8 || reg_use[i][v[0]]) {
			cout << "Core Idle" << '\n';
			continue;
		}
        	blockcnt[{i,v[0]}]++;
		cout << com << '\n';
                reg_use[i][v[0]]++;
                line[i] = executelw(v, line, i);
		line[i]++;
            } else {
		if (dramtime==0 && waiting) {
                        cout << "DRAM Waiting" << '\n';
                        continue;
                }
		if (mrmtime[i]!=0 || switchtime!=0){
                        cout << "MRM busy" << '\n';
                        continue;
                }
		if (requests[i].size()>=8 || reg_use[i][v[0]] || mrmtime[i]!=0) {
			cout << "Core Idle" << '\n';
			continue;
		}
		cout << com << '\n';
                line[i] = executesw(v, line, i);
		line[i]++;
            }
        }
    }
}
bool decide(int a, int b)
{
    return metric[a]>metric[b];
}
int main(int argc, char ** argv) { //changed
    N = str2int(argv[1], 0);
    M = str2int(argv[2], 0);
    assert(N<=8);
    row_delay = str2int(argv[3], 0);
    column_delay = str2int(argv[4], 0);
    vector < string > filename;
    for (int i = 0; i < N; ++i) {
        filename.push_back(argv[5 + i]);
    }
    ifstream input_file;
    for (int i = 0; i < N; ++i) {
        input_file.open(filename[i]);
        string line;
        map < int, string > temp;
        map < string, int > l;
        map < string, vector < string >> d;
        map < string, int > r;
        cur.push_back(0);
        int ins=0, lsins=0;
        while (getline(input_file, line)) {
            line = eraseTrail(line);
            if (line != "" && line[line.size() - 1] != ':') {
                ++ins;
                temp[cur[i]] = line;
                if((line[0]=='l' || line[0]=='s') && line[1]=='w') ++lsins;
                ++cur[i];
            }
            if (line[line.size() - 1] == ':') {
                if (line.find(" ") != std::string::npos) {
                    cout << "Labels cannot have whitespace" << '\n';
                    return 0;
                }
                l[line.substr(0, line.size() - 1)] = cur[i];
            }
        }
        metric[i]= ((float)ins)/lsins;
        temp[cur[i]] = "";
        cur[i]++;
        d["$zero"] = vector < string > ({"$zero"});
        for (int i = 0; i < 10; ++i) {
            string w = "$t" + to_string(i);
            res_reg[w] = 1;
            r[w] = 0;
            d[w] = vector < string > ({w});
        }
        for (int i = 0; i < 10; ++i) {
            string w = "$s" + to_string(i);
            res_reg[w] = 1;
            r[w] = 0;
            d[w] = vector < string > ({w});
        }
	r["$zero"] = 0;
        command.push_back(temp);
        labels.push_back(l);
        dep.push_back(d);
        reg.push_back(r);
        reg_use.push_back(map < string, int > ());
	swcnt.push_back(0);
	e.push_back(false);
	input_file.close();
	mrmtime.push_back(0);
    }
    res_reg["$zero"] = 1;
    res_wrd["add"] = 1;
    res_wrd["sub"] = 1;
    res_wrd["addi"] = 1;
    res_wrd["mul"] = 1;
    res_wrd["bne"] = 1;
    res_wrd["beq"] = 1;
    res_wrd["j"] = 1;
    res_wrd["slt"] = 1;
    res_wrd["lw"] = 1;
    res_wrd["sw"] = 1;
    for (auto i: res_wrd)
        cnt[i.first] = 0;
    for (int i = 0; i < 10; ++i) hexa[i] = ('0' + i);
    for (int i = 10; i < 16; ++i) hexa[i] = ('A' + (i - 10));
    for(int i=0;i<N;++i) order.push_back(i);
    for(int i=0;i<8;++i) fw[i]=false;
    sort(order.begin(),order.end(),decide);
    execute();
    get();
}
