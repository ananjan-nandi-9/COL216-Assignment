#include <bits/stdc++.h>

using namespace std;
vector < map < string, int >> reg;
map < string, int > cnt;
vector < map < int, string >> command;
vector < int > cur;
int num = 0;
int sclock = 0;
map < string, int > res_reg, res_wrd;
map < int, char > hexa;
vector < map < string, int >> labels, reg_use;
deque < pair < vector < string > , vector < int >>> requests; //dram request queue
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
int curcycle = 0;
int curcore = -1;
int curtype = -1;

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
void isReg(string s, int line) {
    if (res_reg[s] == 0) {
        cout << "Invalid Register Name, error in instruction: " << line << '\n';
        exit(0);
    }
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
        exit(0);
    }
    isReg(v[0], line[i]);
    isReg(v[1], line[i]);
    isReg(v[2], line[i]);
    if (v[0] == "$zero") {
        cout << "Cannot manipulate $zero: " << line[i] << '\n';
        exit(0);
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
        exit(0);
    }
    isReg(v[0], line[i]);
    isReg(v[1], line[i]);
    int x = str2int(v[2], line[i]);
    if (v[0] == "$zero") {
        cout << "Cannot manipulate $zero: " << line[i] << '\n';
        exit(0);
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
        exit(0);
    }
    isReg(v[0], line[i]);
    isReg(v[1], line[i]);
    isReg(v[2], line[i]);
    if (v[0] == "$zero") {
        cout << "Cannot manipulate $0: " << line[i] << '\n';
        exit(0);
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
        exit(0);
    }
    isReg(v[0], line[i]);
    isReg(v[1], line[i]);
    isReg(v[2], line[i]);
    if (v[0] == "$zero") {
        cout << "Cannot manipulate $zero: " << line[i] << '\n';
        exit(0);
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
        exit(0);
    }
    isReg(v[0], line[i]);
    isReg(v[1], line[i]);
    if (labels[i].find(v[2]) == labels[i].end()) {
        cout << "Label not found: " << line[i] << '\n';
        exit(0);
    }
    int x = labels[i][v[2]];
    if (reg[i][v[0]] == reg[i][v[1]]) {
        if (x < 0 || x >= cur[i]) {
            cout << "Instruction doesn't exist: " << line[i] << '\n';
            exit(0);
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
        exit(0);
    }
    isReg(v[0], line[i]);
    isReg(v[1], line[i]);
    if (labels[i].find(v[2]) == labels[i].end()) {
        cout << "Label not found: " << line[i] << '\n';
        exit(0);
    }
    int x = labels[i][v[2]];
    if (reg[i][v[0]] != reg[i][v[1]]) {
        if (x < 0 || x >= cur[i]) {
            cout << "Instruction doesn't exist: " << line[i] << '\n';
            exit(0);
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
        exit(0);
    }
    if (labels[i].find(v[0]) == labels[i].end()) {
        cout << "Label not found: " << line[i] << '\n';
        exit(0);
    }
    int x = labels[i][v[0]];
    if (x >= pow(2, 19) || x < 0) {
        cout << "Instruction doesn't exist: " << line[i] << '\n';
        exit(0);
    }
    if (x < 0 || x >= cur[i]) {
        cout << "Instruction doesn't exist: " << line[i] << '\n';
        exit(0);
    }
    num++;
    return x;
}

void executeslt(vector < string > v, vector < int > line, int i) {
    if (v.size() != 3) {
        cout << "slt command needs 3 arguments, error in instruction: " << line[i] << '\n';
        exit(0);
    }
    isReg(v[0], line[i]);
    isReg(v[1], line[i]);
    isReg(v[2], line[i]);
    if (v[0] == "$zero") {
        cout << "Cannot manipulate $0: " << line[i] << '\n';
        exit(0);
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
            cout << v[0] << " " << get_hexa(reg[i][v[0]]) << '\n';
            row_buffer = row;
            updates++;
        } else if (row_buffer == row) {
            cc = 1;
            dramtime += column_delay;
            cout << v[0] << " " << get_hexa(reg[i][v[0]]) << '\n';
        } else {
            rc = 2;
            dramtime += row_delay;
            dramtime += row_delay;
            cc = 1;
            dramtime += column_delay;
            cout << v[0] << " " << get_hexa(reg[i][v[0]]) << '\n';
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
            cout << val << " to " << val + 3 << " " << get_hexa(reg[i][v[0]]) << '\n';
            row_buffer = row;
            updates++;
        } else if (row_buffer == row) {
            cc = 1;
            dramtime += column_delay;
            cout << val << " to " << val + 3 << " " << get_hexa(reg[i][v[0]]) << '\n';
        } else {
            rc = 2;
            dramtime += row_delay;
            dramtime += row_delay;
            cc = 1;
            dramtime += column_delay;
            cout << val << " to " << val + 3 << " " << get_hexa(reg[i][v[0]]) << '\n';
            row_buffer = row;
            updates++;
        }
    }
}

int executelw(vector < string > v, vector < int > line, int i) { //added
    if (v.size() != 2) {
        cout << "lw command needs 2 arguments, error in instruction: " << line[i] << '\n';
        exit(0);
    }
    isReg(v[0], line[i]);
    if (v[0] == "$zero") {
        cout << "Cannot manipulate $0: " << line[i] << '\n';
        exit(0);
    }
    dep[i][v[0]] = vector < string > ({
        v[0]
    });
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
        if (isdigit(temp[0]) || temp[0] == '(') {
            while (curr < temp.size() && isdigit(temp[curr])) {
                curr++;
            }
            if (curr == temp.size() || temp[curr] != '(' || temp[temp.size() - 1] != ')') {
                cout << "Syntax error, instruction: " << line[i] << '\n';
                exit(0);
            } else {
                temp = temp.substr(curr + 1, temp.size() - curr - 2);
            }
            if (res_reg[temp] == 0) {
                cout << "Not a register, error in instruction: " << line[i] << '\n';
                exit(0);
            }
            int offset = str2int(v[1].substr(0, curr), line[i]);
            int val = offset + reg[i][temp];
            if (val > pow(2, 19) || val < 0) {
                cout << "Out of Memory, error in instruction: " << line[i] << '\n';
                exit(0);
            }
            if (val % 4 != 0) {
                cout << "Invalid Memory Request:  " << line[i] << '\n';
                exit(0);
            }
            temp = to_string(val);
        } else {
            cout << "Syntax error, instruction: " << line[i] << '\n';
        }
    }
    if (!found) {
        if (str2int(temp, line[i]) > pow(2, 19) || str2int(temp, line[i]) < 0 || str2int(temp, line[i]) % 4 != 0) {
            cout << "Out of Memory, error in instruction: " << line[i] << '\n';
            exit(0);
        }
    }
    int val = str2int(temp, line[i]);
    val += i * pow(2, 20) / N;
    int row = val / 1024;
    int column = val - (1024 * row);
    reg[i][v[0]] = str2int(dram[row][column], line[i]);
    v[1] = to_string(val);
    num++;
    requests.push_back({v,{i,row,column,0,num}});
    return line[i]+1;
}

int executesw(vector < string > v, vector < int > line, int i) { //added
    if (v.size() != 2) {
        cout << "sw command needs 2 arguments, error in instruction: " << line[i] << '\n';
        exit(0);
    }
    isReg(v[0], line[i]);
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
        if (isdigit(temp[0]) || temp[0] == '(') {
            while (curr < temp.size() && isdigit(temp[curr])) {
                curr++;
            }
            if (curr == temp.size() || temp[curr] != '(' || temp[temp.size() - 1] != ')') {
                cout << "Syntax error, instruction: " << line[i] << '\n';
                exit(0);
            } else {
                temp = temp.substr(curr + 1, temp.size() - curr - 2);
            }
            if (res_reg[temp] == 0) {
                cout << "Not a register, error in instruction: " << line[i] << '\n';
                exit(0);
            }
            int offset = str2int(v[1].substr(0, curr), line[i]);
            int val = offset + reg[i][temp];
            if (val > pow(2, 19) || val < 0) {
                cout << "Out of Memory, error in instruction: " << line[i] << '\n';
                exit(0);
            }
            if (val % 4 != 0) {
                cout << "Invalid Memory Request:  " << line[i] << '\n';
                exit(0);
            }
            temp = to_string(val);
        } else {
            cout << "Syntax error, instruction: " << line[i] << '\n';
        }
    }
    if (!found) {
        if (str2int(temp, line[i]) > pow(2, 19) || str2int(temp, line[i]) < 0 || str2int(temp, line[i]) % 4 != 0) {
            cout << "Out of Memory, error in instruction: " << line[i] << '\n';
            exit(0);
        }
    }
    int val = str2int(temp, line[i]);
    val += i * pow(2, 20) / N;
    int row = val / 1024;
    int column = val - (1024 * row);
    dram[row][column] = to_string(reg[i][v[0]]);
    v[1] = to_string(val);
    num++;
    requests.push_back({
        v,
        {
            i,
            row,
            column,
            1,
            num
        }
    });
    return line[i]+1;
}

void execute() //changed
{
    vector < int > line(N, 0);
    while (sclock < M) {
        sclock++;
	cout << "Clock Cycle : " << sclock << '\n';
        if (dramtime == 0) {
            if (!requests.empty()) {
                pair < vector < string > , vector < int >> temp = requests[0];
                requests.pop_front();
                executing = 1;
                reg_use[temp.second[0]][temp.first[0]]--;
                curcycle = 0;
                curcore = temp.second[0];
                curtype = temp.second[3];
                dramexecute(temp.first, temp.second[0], temp.second[1], temp.second[2], temp.second[3]);
                if (curtype==0){
                    cout << "Started executing READ" << '\n';
                } else {
                    cout << "Started executing WRITE" << '\n';
                }
                vector<map<string,bool>>dup(N);
                deque<pair<vector<string>, vector<int>>>req1;
                sort(requests.begin(),requests.end(),comp);
                for(auto ii:requests)
                {
                    int ccore=ii.second[0];
                    vector<string> temp=dep[ccore][ii.first[0]];
                    if (!ii.second[3] && find(temp.begin(),temp.end(),ii.first[0])==temp.end()){
                            continue;
                    }
                    if (ii.second[3]==1){
                        if (dup[ccore][ii.first[1]]==0){
                            dup[ccore][ii.first[1]]=1;
                            req1.push_back(ii);
                            dup[ccore][ii.first[0]]=0;
                        } else {
                            continue;
                        }
                    } else {
                        if (dup[ccore][ii.first[0]]==0){
                            dup[ccore][ii.first[0]]=1;
                            req1.push_back(ii);
                            dup[ccore][ii.first[1]]=0;
                        } else {
                            continue;
                        }
                    }
                }
                requests=req1;
	            reverse(requests.begin(),requests.end());
	            req1.clear();
                // dram reorder operations
            } else {
		cout << "DRAM Idle" << '\n';
                curcore = -1;
                executing = 0;
                curcycle = 0;
		curtype=-1;
            }
        } else {
            dramtime--;
            curcycle++;
            if (rc) {
                if (curcycle == row_delay) {
                    rc--;
                }
            } else if (cc) {
                if (curcycle == column_delay) {
                    cc--;
                }
            }
	    if (curtype==0){
                        cout << "Executing READ" << '\n';
           } else if (curtype==1){
                        cout << "Executing WRITE" << '\n';
           }
        }
        for (int i = 0; i < N; ++i) {
            if (line[i] == cur[i] || eraseTrail(command[i][line[i]]) == "" || (curtype == 0 && dramtime == 1 && curcore == i)) {
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
                exit(0);
            }
            cnt[todo]++;
            if (todo == "add") {
                if (executing && (reg_use[i][v[1]] || reg_use[i][v[2]])) continue;
                executeadd(v, line, i);
		line[i]++;
		cout << i << " " << com << '\n';
            } else if (todo == "addi") {
                if (executing && (reg_use[i][v[1]])) continue;
                executeaddi(v, line, i);
		line[i]++;
		cout << i << " " << com << '\n';
            } else if (todo == "sub") {
                if (executing && (reg_use[i][v[1]] || reg_use[i][v[2]])) continue;
                executesub(v, line, i);
		line[i]++;
		cout << i << " " << com << '\n';
            } else if (todo == "mul") {
                if (executing && (reg_use[i][v[1]] || reg_use[i][v[2]])) continue;
                executemul(v, line, i);
		line[i]++;
		cout << i << " " << com << '\n';
            } else if (todo == "beq") {
                if (executing && (reg_use[i][v[1]] || reg_use[i][v[0]])) continue;
                line[i] = executebeq(v, line, i);
		cout << i << " " << com << '\n';
            } else if (todo == "bne") {
                if (executing && (reg_use[i][v[1]] || reg_use[i][v[0]])) continue;
                line[i] = executebne(v, line, i);
		cout << i << " " << com << '\n';
            } else if (todo == "j") {
                line[i] = executej(v, line, i);
            } else if (todo == "slt") {
                if (executing && (reg_use[i][v[1]] || reg_use[i][v[2]])) continue;
                executeslt(v, line, i);
		line[i]++;
		cout << i << " " << com << '\n';
            } else if (todo == "lw") {
		if (requests.size()>=16) continue;
                reg_use[i][v[0]]++;
                line[i] = executelw(v, line, i);
		line[i]++;
		cout << i << " " << com << '\n';
            } else {
		if (requests.size()>=16) continue;
                line[i] = executesw(v, line, i);
		line[i]++;
		cout << i << " " << com << '\n';
            }
        }
    }
}

int main(int argc, char ** argv) { //changed
    N = str2int(argv[1], 0);
    M = str2int(argv[2], 0);
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
        while (getline(input_file, line)) {
            line = eraseTrail(line);
            if (line != "" && line[line.size() - 1] != ':') {
                temp[cur[i]] = line;
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
	input_file.close();
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
    execute();
    get();
}
