#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <stdlib.h>
#include <sstream>
#include <bitset>
#include <map>
#include <math.h>
#include <cmath>
#include <queue>

using namespace std;

struct inst {
	string operand = ""; //ex: ADD
	string code = "";
};
queue<inst> IQ; //store instruction
int RSorderAdd = 0, RSorderMul = 0; //instruction放進RS的順序
string RStagName[5] = { "RS1","RS2","RS3","RS4","RS5" };

struct RScontain {
	int tag = 0;
	char operand = '\0';
	string goal = "";
	string val1 = "";
	string val2 = "";
	inst instruction;
	int ready = 0;//ready to dispatch
};
bool RSvalAdd[3], RSvalMul[2]; //是否可以dispatch
bool RShaveAdd[3], RShaveMul[2]; //有沒有放inst在這個RS

int cycle = 1;
map<string, int>RF = { {"F1",0},{"F2",2},{"F3",4},{"F4",6},{"F5",8} };
bool RFval[5] = { 1,1,1,1,1 };//check RF的val是否可以用
map<string, string>RAT{ {"F1"," "},{"F2"," "},{"F3"," "},{"F4"," "},{"F5"," "} };
RScontain RSAdd[3], RSMul[2];
int RSAddSize = 0, RSMulSize = 0;

struct bufferContain {
	RScontain inbuffer;
	int result;
};

int wrAddSize = 0, wrMulSize = 0;
vector<bufferContain> bufferAdd, bufferMul;
string addcode, mulcode;

bool issue();
void outcome();
bool excute();
bool wr();

bool wr()
{
	bool haveWr1 = 0, haveWr2 = 0;
	if (wrAddSize > 0 && bufferAdd.size() > 0)
	{
		//RF值更新
		RF[bufferAdd[0].inbuffer.goal] = bufferAdd[0].result;

		RFval[int(bufferAdd[0].inbuffer.goal[1]) - 48 - 1] = 1;
		string pos = "RS" + to_string(bufferAdd[0].inbuffer.tag + 1);

		//寫回在RS上也需要這個值的RS
		for (int i = 0; i < 3; i++)
		{
			bool change = 0;
			if (RSAdd[i].val1 == pos)//bufferAdd[0].inbuffer.goal
			{
				RSAdd[i].val1 = to_string(bufferAdd[0].result);
				change = 1;

			}
			if (RSAdd[i].val2 == pos)//bufferAdd[0].inbuffer.goal
			{
				RSAdd[i].val2 = to_string(bufferAdd[0].result);
				change = 1;

			}
			//判斷這個RS的值是否都有了
			if (change == 1 && RSAdd[i].val1[0] != 'R' && RSAdd[i].val2[0] != 'R') {
				RSAdd[i].ready = 1;
				//wrAddSize++;
			}
		}
		for (int i = 0; i < 2; i++)
		{
			bool change = 0;
			if (RSMul[i].val1 == pos)//bufferAdd[0].inbuffer.goal
			{
				RSMul[i].val1 = to_string(bufferAdd[0].result);
				change = 1;

			}
			if (RSMul[i].val2 == pos)//bufferAdd[0].inbuffer.goal
			{
				RSMul[i].val2 = to_string(bufferAdd[0].result);
				change = 1;

			}
			//判斷這個RS的值是否都有了
			if (change == 1 && RSMul[i].val1[0] != 'R' && RSMul[i].val2[0] != 'R') {
				RSMul[i].ready = 1;
				//wrMulSize++;
			}

		}
		string s = "RS" + to_string(bufferAdd[0].inbuffer.tag + 1);
		if (RAT[bufferAdd[0].inbuffer.goal] == s)
			RAT[bufferAdd[0].inbuffer.goal] = ""; //RAT的暫存值刪除

		wrAddSize--;
		addcode = "(RS" + to_string(bufferAdd[0].inbuffer.tag + 1) + ") " + bufferAdd[0].inbuffer.val1
			+ bufferAdd[0].inbuffer.operand + bufferAdd[0].inbuffer.val2;


		RScontain tmpp;//把dispatch後的RS[i]初始化
		RSAdd[bufferAdd[0].inbuffer.tag] = tmpp;
		RShaveAdd[bufferAdd[0].inbuffer.tag] = 0;
		RSAddSize--;

		bufferAdd.erase(bufferAdd.begin());
		RSorderAdd++;
		if (RSorderAdd == 3)
			RSorderAdd = 0;

		haveWr1 = 1;
	}
	else
		addcode = "empty";

	if (wrMulSize > 0 && bufferMul.size() > 0)
	{
		//更新RF值
		RF[bufferMul[0].inbuffer.goal] = bufferMul[0].result;
		RFval[int(bufferMul[0].inbuffer.goal[1]) - 48 - 1] = 1;

		string pos = "RS" + to_string(bufferMul[0].inbuffer.tag + 4);

		//寫回在RS上也需要這個值的RS
		for (int i = 0; i < 2; i++)
		{
			bool change = 0;
			if (RSMul[i].val1 == pos)//bufferMul[0].inbuffer.goal
			{
				RSMul[i].val1 = to_string(bufferMul[0].result);
				change = 1;
			}
			if (RSMul[i].val2 == pos)//bufferMul[0].inbuffer.goal
			{
				RSMul[i].val2 = to_string(bufferMul[0].result);
				change = 1;
			}
			//判斷這個RS的值是否都有了
			if (change == 1 && RSMul[i].val1[0] != 'R' && RSMul[i].val2[0] != 'R') {
				RSMul[i].ready = 1;
				//wrMulSize++;
			}

		}
		for (int i = 0; i < 3; i++)
		{
			bool change = 0;
			if (RSAdd[i].val1 == pos)///bufferMul[0].inbuffer.goal
			{
				RSAdd[i].val1 = to_string(bufferMul[0].result);
				change = 1;
			}
			if (RSAdd[i].val2 == pos)//bufferMul[0].inbuffer.goal
			{
				RSAdd[i].val2 = to_string(bufferMul[0].result);
				change = 1;
			}

			//判斷這個RS的值是否都有了
			if (change == 1 && RSAdd[i].val1[0] != 'R' && RSAdd[i].val2[0] != 'R') {
				RSAdd[i].ready = 1;
				//wrAddSize++;
			}
		}

		string s = "RS" + to_string(bufferMul[0].inbuffer.tag + 4);
		if (RAT[bufferMul[0].inbuffer.goal] == s)
			RAT[bufferMul[0].inbuffer.goal] = ""; //RAT的暫存值刪除

		wrMulSize--;
		mulcode = "(RS" + to_string(bufferMul[0].inbuffer.tag + 4) + ") " + bufferMul[0].inbuffer.val1
			+ bufferMul[0].inbuffer.operand + bufferMul[0].inbuffer.val2;


		RScontain tmpp;//把dispatch後的RS[i]初始化
		RSMul[bufferMul[0].inbuffer.tag] = tmpp;
		RShaveMul[bufferMul[0].inbuffer.tag] = 0;
		RSMulSize--;

		bufferMul.erase(bufferMul.begin());
		RSorderMul++;
		if (RSorderMul == 2)
			RSorderMul = 0;

		haveWr2 = 1;
	}
	else
		mulcode = "empty";

	if (haveWr1 == 1 || haveWr2 == 1)
		return 1;
	return 0;


}

bool excute()
{
	int ans1, ans2;
	bool haveExc1 = 0, haveExc2 = 0;
	if (RSAddSize > 0)
	{
		for (int i = 0; i < 3; i++)
		{
			if (RSAdd[i].ready == 1)//RShaveAdd[RSorderAdd] == 1 && 
			{
				if (RSAdd[i].operand == '+')
					ans1 = stoi(RSAdd[i].val1) + stoi(RSAdd[i].val2);
				else
					ans1 = stoi(RSAdd[i].val1) - stoi(RSAdd[i].val2);

				RFval[int(RSAdd[i].goal[1]) - 48 - 1] = 0;

				bufferContain tmp;
				tmp.inbuffer = RSAdd[i];
				tmp.result = ans1;
				bufferAdd.push_back(tmp);
				wrAddSize++;

			}
		}
		haveExc1 = 1;

	}

	if (RSMulSize > 0)
	{
		for (int i = 0; i < 2; i++)
		{
			if (RSMul[i].ready == 1)//RShaveMul[RSorderMul] == 1 &&
			{
				if (RSMul[i].operand == '*')
					ans2 = stoi(RSMul[i].val1) * stoi(RSMul[i].val2);
				else
					ans2 = stoi(RSMul[i].val1) / stoi(RSMul[i].val2);

				RFval[int(RSMul[i].goal[1]) - 48 - 1] = 0;

				bufferContain tmp;
				tmp.inbuffer = RSMul[i];
				tmp.result = ans2;
				bufferMul.push_back(tmp);
				wrMulSize++;

			}
		}
		haveExc2 = 1;
	}

	if (haveExc1 == 1 || haveExc2 == 1)
		return 1;
	return 0;
}

void addi(int RSindex)
{
	string::size_type begin, end;
	string code = RSAdd[RSindex].instruction.code;
	bool ready1 = 0;

	//第一個reg 
	begin = 0;
	end = code.find(',');
	string reg1 = code.substr(begin, end - begin);
	RSAdd[RSindex].goal = reg1;
	string s = "RS" + to_string(RSindex + 1);

	//第二個reg 
	begin = end + 1;
	end = code.find(',', begin);
	string reg2 = code.substr(begin, end - begin);
	RSAdd[RSindex].val1 = RAT[reg2];
	if (RFval[int(reg2[1]) - 48 - 1])
	{
		RSAdd[RSindex].val1 = to_string(RF[reg2]);
		ready1 = 1;
	}
	//immediate
	begin = end + 1;
	end = code.size();
	string reg3 = code.substr(begin, end - begin);
	RSAdd[RSindex].val2 = reg3;


	//RS等一下是否可以dispatch
	if (ready1 == 1)
		RSAdd[RSindex].ready = 1;
	else
		RSAdd[RSindex].ready = 0;

	RAT[reg1] = s;

}

void forRSAdd(int RSindex)
{
	string::size_type begin, end;
	string code = RSAdd[RSindex].instruction.code;
	bool ready1 = 0, ready2 = 0;
	if (RSAdd[RSindex].instruction.operand == "ADD" || RSAdd[RSindex].instruction.operand == "ADDI")
		RSAdd[RSindex].operand = '+';
	else
		RSAdd[RSindex].operand = '-';

	//第一個reg 
	begin = 0;
	end = code.find(',');
	string reg1 = code.substr(begin, end - begin);
	RSAdd[RSindex].goal = reg1;
	string s = "RS" + to_string(RSindex + 1);

	//第二個reg 
	begin = end + 1;
	end = code.find(',', begin);
	string reg2 = code.substr(begin, end - begin);
	RSAdd[RSindex].val1 = RAT[reg2];
	if (RFval[int(reg2[1]) - 48 - 1])
	{
		RSAdd[RSindex].val1 = to_string(RF[reg2]);
		ready1 = 1;
	}
	//reg3
	begin = end + 1;
	end = code.size();
	string reg3 = code.substr(begin, end - begin);
	RSAdd[RSindex].val2 = RAT[reg3];
	if (RFval[int(reg3[1]) - 48 - 1])
	{
		RSAdd[RSindex].val2 = to_string(RF[reg3]);
		ready2 = 1;
	}

	//RS等一下是否可以dispatch
	if (ready1 == ready2 == 1)
		RSAdd[RSindex].ready = 1;
	else
		RSAdd[RSindex].ready = 0;

	RAT[reg1] = s;
}

void forRSMul(int RSindex)
{
	string::size_type begin, end;
	string code = RSMul[RSindex].instruction.code;
	bool ready1 = 0, ready2 = 0;

	if (RSMul[RSindex].instruction.operand == "DIV")
		RSMul[RSindex].operand = '/';
	else
		RSMul[RSindex].operand = '*';


	//第一個reg 
	begin = 0;
	end = code.find(',');
	string reg1 = code.substr(begin, end - begin);
	RSMul[RSindex].goal = reg1;
	string s = "RS" + to_string(RSindex + 4);
	RAT[reg1] = s;

	//第二個reg 
	begin = end + 1;
	end = code.find(',', begin);
	string reg2 = code.substr(begin, end - begin);
	RSMul[RSindex].val1 = RAT[reg2];
	if (RFval[int(reg2[1]) - 48 - 1])
	{
		RSMul[RSindex].val1 = to_string(RF[reg2]);
		ready1 = 1;
	}
	//reg3
	begin = end + 1;
	end = code.size();
	string reg3 = code.substr(begin, end - begin);
	RSMul[RSindex].val2 = RAT[reg3];
	if (RFval[int(reg3[1]) - 48 - 1])
	{
		RSMul[RSindex].val2 = to_string(RF[reg3]);
		ready2 = 1;
	}

	//RS是否可以dispatch
	if (ready1 == ready2 == 1) {
		RSMul[RSindex].ready = 1;
	}
	else
		RSMul[RSindex].ready = 0;
}

bool issue()
{
	bool haveis = 0;
	inst tmp = IQ.front();
	int RSindex;
	if (tmp.operand == "ADD" || tmp.operand == "ADDI" || tmp.operand == "SUB")
	{
		for (RSindex = 0; RSindex < 3; RSindex++) {
			if (RShaveAdd[RSindex] == 0) { //RS沒有東西

				break;
			}
		}

		RSAdd[RSindex].tag = RSindex;
		RSAdd[RSindex].instruction = tmp;

		//把值放進RS中
		if (tmp.operand == "ADDI")
			addi(RSindex);
		else
			forRSAdd(RSindex);

		if (tmp.operand == "SUB")
			RSAdd[RSindex].operand = '-';
		else
			RSAdd[RSindex].operand = '+';

		IQ.pop();
		RSAddSize++;
		RShaveAdd[RSindex] = 1;
		haveis = 1;
	}
	else
	{
		for (RSindex = 0; RSindex < 2; RSindex++) {
			if (RShaveMul[RSindex] == 0) { //RS沒有東西
				break;
			}
		}

		RSMul[RSindex].tag = RSindex;
		RSMul[RSindex].instruction = tmp;

		//把值放進RS中

		forRSMul(RSindex);

		if (tmp.operand == "MUL")
			RSMul[RSindex].operand = '*';
		else
			RSMul[RSindex].operand = '/';

		IQ.pop();
		RSMulSize++;
		RShaveMul[RSindex] = 1;

		haveis = 1;
	}

	if (haveis == 1)
		return 1;
	return  0;
}
void outcome(string codeAdd, string codeMul)
{
	cout << "Cycle:" << cycle << endl;
	cout << endl;

	cout << "RF" << endl;
	cout << "F1 |" << setw(4) << RF["F1"] << endl;
	cout << "F2 |" << setw(4) << RF["F2"] << endl;
	cout << "F3 |" << setw(4) << RF["F3"] << endl;
	cout << "F4 |" << setw(4) << RF["F4"] << endl;
	cout << "F5 |" << setw(4) << RF["F5"] << endl;
	cout << endl;

	cout << "RAT" << endl;
	cout << "F1 |" << setw(4) << RAT["F1"] << endl;
	cout << "F2 |" << setw(4) << RAT["F2"] << endl;
	cout << "F3 |" << setw(4) << RAT["F3"] << endl;
	cout << "F4 |" << setw(4) << RAT["F4"] << endl;
	cout << "F5 |" << setw(4) << RAT["F5"] << endl;
	cout << endl;

	cout << "RS" << endl;
	cout << "RS1 |" << setw(3) << RSAdd[0].operand << setw(1)<<"|" << setw(3) << RSAdd[0].val1 << setw(1) << "|" << setw(4) << RSAdd[0].val2 << "|" << endl;
	cout << "RS2 |" << setw(3) << RSAdd[1].operand << setw(1) << "|" << setw(3) << RSAdd[1].val1 << setw(1) << "|" << setw(4) << RSAdd[1].val2 << "|" << endl;
	cout << "RS3 |" << setw(3) << RSAdd[2].operand << setw(1) << "|" << setw(3) << RSAdd[2].val1 << setw(1) << "|" << setw(4) << RSAdd[2].val2 << "|" << endl;
	cout << "BUFFER:" << codeAdd << endl;
	cout << endl;

	cout << "RS" << endl;
	cout << "RS4 |" << setw(4) << RSMul[0].operand << "|" << setw(4) << RSMul[0].val1 << "|" << setw(4) << RSMul[0].val2 << "|" << endl;
	cout << "RS5 |" << setw(4) << RSMul[1].operand << "|" << setw(4) << RSMul[1].val1 << "|" << setw(4) << RSMul[1].val2 << "|" << endl;
	cout << "BUFFER:" << codeMul << endl;
	cout << endl;


}

int main()
{
	ifstream infile;
	infile.open("input.txt");

	if (!infile.is_open()) {
		cout << "Failed to open file.\n";
	}
	else {

		string operand, code;

		while (infile >> operand) {
			inst tmp;
			tmp.operand = operand;

			//load code
			getline(infile, code);
			tmp.code = code;
			tmp.code.erase(remove(tmp.code.begin(), tmp.code.end(), ' '), tmp.code.end()); //移除code的空格

			IQ.push(tmp);

		}

		//issue exe wr
		int check[3]{};

		if (issue())
			check[0] = 1;
		outcome("empty", "empty");
		int stopCycle = 0;
		while (cycle++)
		{
			check[0] = 0; check[1] = 0; check[2] = 0;
			if (wr())
				check[2] = 1;
			if (excute())
				check[1] = 1;
			if (!IQ.empty())
				if (issue())
					check[0] = 1;
			if (check[0] == 1 || check[1] == 1 || check[2] == 1)
				outcome(addcode, mulcode);
			
			if (IQ.empty() && bufferAdd.size() == 0 && bufferMul.size() == 0 && RSAddSize == 0 && RSMulSize == 0 && wrAddSize == 0 && wrMulSize == 0)
			{
				break;
			}
			

		}

		cycle++;
		wr();
		excute();
		outcome(addcode, mulcode);


		return 0;

	}
}