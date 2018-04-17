// ResearchTask.cpp: 
//

//0 heur means critical path; -1 means min partial makespan; -2 means max partial intensity

#pragma once
#include "stdafx.h"

#include <iostream>
#include "string"
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <iterator>

#include <ctime>

#include <comutil.h>  
#include <stdio.h>  
#pragma comment(lib, "comsuppw.lib")  
#pragma comment(lib, "kernel32.lib")  

#define NOMINMAX
#include <windows.h>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#pragma comment(lib, "User32.lib")

#include "Solver.h"

//#define HEUR
#define MIXED_Z
#define SLICE //Process only some part of files to make it quick.
//#define MIXED
//#define EXACT
//#define SPR


#define C_DEPTH 2
#define DEPTH 3 //Fast
#define FILES 200

std::map<std::string, float> getOptSolutions(std::string dir, std::string fn, int tasksNum)
{
	std::map<std::string, float> res;
	std::string nm = dir + "\\" + fn;
	std::ifstream file(nm);
	std::string line;
	int param, inst;
	float mspan;
	while (std::getline(file, line))
	{
		//std::cout << "Something\n";
		std::stringstream ss;
		ss << line;
		ss >> param >> inst >> mspan;
		std::string name = "j" + std::to_string(tasksNum) + std::to_string(param) + "_" + std::to_string(inst) + ".sm";
		res[name] = mspan;
	}
	return res;
}

// File read&write functions
std::string readFile(std::string fn) {
	std::ifstream file(fn);
	std::string line, src;
	while (std::getline(file, line)) { src += line; src += '\n'; }
	return src;
}

std::vector<std::string> readDirectory(STRSAFE_LPWSTR dir) {

	std::vector<std::string> r;

	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize;
	TCHAR szDir[MAX_PATH];
	size_t length_of_arg;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;

	// Prepare string for use with FindFile functions.  First, copy the
	// string to a buffer, then append '\*' to the directory name.

	StringCchCopy(szDir, MAX_PATH, dir);
	StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

	// Find the first file in the directory.
	hFind = FindFirstFile(szDir, &ffd);

	if (INVALID_HANDLE_VALUE == hFind) {
		return std::vector<std::string>();
	}

	// List all the files in the directory with some info about them.

	do {
		std::wstring ws(ffd.cFileName);
		std::string str(ws.begin(), ws.end());
		r.push_back(str);
	} while (FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);
	return r;
}

std::wstring s2ws(const std::string s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

void OutputResults(std::vector<std::string> fns, std::string dir)
{
	std::vector<int> nums = { 2, 3, 9, 10, 11, 12, 15, 16, 21 };

	
	std::map<std::pair<int, int>, int> wins;
	std::map<std::pair<int, int>, int> hits;
	std::map<std::pair<int, int>, float> avg_loss;
	std::map<std::pair<int, int>, float> delta;
	std::map<std::pair<int, int>, float> sigma;
	for (auto i = -2; i <= DEPTH; i++)
	{
		
		for (auto j = -2; j <= 21; j++)
		{
			wins[std::pair<int, int>(j, i)] = 0;
			hits[std::pair<int, int>(j, i)] = 0;
			avg_loss[std::pair<int, int>(j, i)] = 0.0;
			sigma[std::pair<int, int>(j, i)] = 0.0;
			delta[std::pair<int, int>(j, i)] = 0.0;
		}
	}
	std::ofstream output;
	output.open(dir + "\\output.txt");
	//for (auto i : nums) output << i << ' ' << SimpleHeuristics(i).description << '\n';
	//exit(0);
	int bugs = 0;
	std::map<std::string, float> optTimes = getOptSolutions(dir, "optSol.txt", 30);
	float avg_time[C_DEPTH + 1];
	memset(avg_time, 0, sizeof(avg_time));
	for (unsigned i = 2; i < fns.size(); i++) {
		//std::cout << dir << '\n';
		if ((dir == "ResearchTests30") && (optTimes.count(fns.at(i)) == 0)) continue;
#ifdef SLICE
		if (i % 3 != 1) continue;
#endif SLICE
		float bestTime = INT32_MAX;
		std::map<std::pair<int, int>, float> tim;


		std::string src = readFile(dir + "\\" + fns.at(i));
		std::cout << fns.at(i) + ":\n";
		float optTime = optTimes[fns.at(i)];
#ifdef SPR
		for (auto h = 0; h <= 21; h++)
		{
			Problem * t3 = new Problem(src);
			SprecherSolver s3(t3, (h != 0) ? new SimpleHeuristics(h) : NULL);
			//std::cout << s3.h->name << '\n';
			s3.solve();
			std::cout << "SprSolver" << h << ' ' << s3.solution.second << '\n';
			output << fns.at(i) << " SprSolver " << h << ' ' << s3.solution.second << ' ' << s3.solution.second / optTime << " of optimal\n";
			tim[std::pair<int, int>(h, -2)] = s3.solution.second;
#ifdef  VALIDATE
			int ch3 = s3.CheckFeasibility();
			if (ch3 > 0) {
				std::cout << "ERROR " << ch3 << '\n'; //output.close(); exit(0);
				//for (auto & i : s3.solution.first) std::cout << i.second.getStatus() << ' ';
				//std::cout << '\n';
				//exit(0);
			}
#endif VALIDATE
			if (tim[std::pair<int, int>(h, -2)] == optTime) hits[std::pair<int, int>(h, -2)]++;
			avg_loss[std::pair<int, int>(h, -2)] += tim[std::pair<int, int>(h, -2)] / optTime;
			delta[std::pair<int, int>(h, -2)] += tim[std::pair<int, int>(h, -2)] - optTime;
			sigma[std::pair<int, int>(h, -2)] += pow(tim[std::pair<int, int>(h, -2)] - optTime, 2);
			if (tim[std::pair<int, int>(h, -2)] < bestTime) { bestTime = tim[std::pair<int, int>(h, -2)]; }
			if (tim[std::pair<int, int>(h, -2)] < optTime) {
				std::cout << h << ' ' << tim[std::pair<int, int>(h, -2)] << ' ' << optTime << " Error!!!!!!!\n"; exit(0);
			}
		}
#endif SPR

		//continue;
#ifdef EXACT
		Problem * t2 = new Problem(src);
		ExactSolver s2(t2, new SimpleHeuristics(5));
		s2.solve();
		std::cout << "ExactSolver: " << s2.solution.second << '\n';
		#ifdef VALIDATE
		int ch2 = s2.CheckFeasibility();
		if (ch2 > 0) {
			std::cout << "ERROR " << ch2 << '\n'; //output.close(); exit(0);
		}
		#endif VALIDATE
#endif EXACT	
	//continue;


		int ch;
#ifdef HEUR
			for (auto h : nums)
			{
				Problem * t1 = new Problem(src);

				HeurSolver s1(t1, new SimpleHeuristics(h));
				//std::cout << "Here\n";
				tim[std::pair<int, int>(h, -1)] = s1.solve();
				//std::cout << "Here\n";
				//std::cout << "In here\n";
#ifdef VALIDATE
				ch = s1.CheckFeasibility();
				if (ch > 0) {
					std::cout << "ERROR " << ch << '\n'; //output.close(); exit(0);
					bugs++;
				}
#endif VALIDATE
				std::cout << "Mixed with heur " << h << " and depth " << -1 << ": " << tim[std::pair<int, int>(h, -1)] << '\n';
				output << fns.at(i) << ' ' << tim[std::pair<int, int>(h, -1)] << ' ' << h << ' ' << -1 << '\n';
				if (tim[std::pair<int, int>(h, -1)] == optTime) hits[std::pair<int, int>(h, -1)]++;
				avg_loss[std::pair<int, int>(h, -1)] += tim[std::pair<int, int>(h, -1)] / optTime;
				delta[std::pair<int, int>(h, -1)] += tim[std::pair<int, int>(h, -1)] - optTime;
				sigma[std::pair<int, int>(h, -1)] += pow(tim[std::pair<int, int>(h, -1)] - optTime, 2);
				if (tim[std::pair<int, int>(h, -1)] < bestTime) { bestTime = tim[std::pair<int, int>(h, -1)]; }
				if (tim[std::pair<int, int>(h, -1)] < optTime) {
					std::cout << "Error!!!!!!!\n"; exit(0);
				}
			}
#endif HEUR
			//time_t avgtime = 
#ifdef MIXED_Z
		for (int n = 0; n <= C_DEPTH; n++)
		{

			for (int j = 2; j <= 2; j++) //!!!RESTORE CR PATH (0) LATER
			{
				Problem * t1 = new Problem(src);
				MixedSolver s1(t1, NULL, n, j);
				time_t temp = clock();
				//std::cout << temp << '\n';
				tim[std::pair<int, int>(-j, n)] = s1.solve();
				std::cout << clock() - temp << '\n';
				avg_time[n] += (float)(clock() - temp) / (float)CLOCKS_PER_SEC / FILES;
				//std::cout << "In here\n";
#ifdef VALIDATE
				ch = s1.CheckFeasibility();
				if (ch > 0) {
					std::cout << "ERROR " << ch << '\n'; //output.close(); exit(0);
					bugs++;
				}
#endif VALIDATE
				std::cout << "Mixed with param " << -j << " and depth " << n << ": " << tim[std::pair<int, int>(-j, n)] << '\n';
				output << fns.at(i) << ' ' << tim[std::pair<int, int>(-j, n)] << ' ' << -j << ' ' << n << '\n';
				if (tim[std::pair<int, int>(-j, n)] == optTime) hits[std::pair<int, int>(-j, n)]++;
				avg_loss[std::pair<int, int>(-j, n)] += tim[std::pair<int, int>(-j, n)] / optTime;
				delta[std::pair<int, int>(-j, n)] += tim[std::pair<int, int>(-j, n)] - optTime;
				sigma[std::pair<int, int>(-j, n)] += pow(tim[std::pair<int, int>(-j, n)] - optTime, 2);
				if (tim[std::pair<int, int>(-j, n)] < bestTime) { bestTime = tim[std::pair<int, int>(-j, n)]; }
				if (tim[std::pair<int, int>(-j, n)] < optTime) {
					std::cout << "Error!!!!!!!\n"; exit(0);
				}
			}
		}
#endif MIXED_Z
		
		//continue;
		std::cout << fns.at(i) << ' ' << optTime << ' ' << "Optimal Time" << '\n';
		output << fns.at(i) << ' ' << optTime << ' ' << "Optimal Time" << '\n';
		//continue;

#ifdef MIXED
		for (auto h : nums) for (int n = 0; n <= DEPTH; n++)
		{
			Problem * t1 = new Problem(src);
			MixedSolver s1(t1, new SimpleHeuristics(h), n);

			tim[std::pair<int, int>(h, n)] = s1.solve();
			//std::cout << "In here\n";
#ifdef VALIDATE
			ch = s1.CheckFeasibility();
			if (ch > 0) {
				std::cout << "ERROR " << ch << '\n'; //output.close(); exit(0);
				bugs++;
			}
#endif VALIDATE
			std::cout << "Mixed with heur " << h << " and depth " << n << ": " << tim[std::pair<int, int>(h, n)] << '\n';
			output << fns.at(i) << ' ' << tim[std::pair<int, int>(h, n)] << ' ' << h << ' ' << n << '\n';
			if (tim[std::pair<int, int>(h, n)] == optTime) hits[std::pair<int, int>(h, n)]++;
			avg_loss[std::pair<int, int>(h, n)] += tim[std::pair<int, int>(h, n)] / optTime;
			delta[std::pair<int, int>(h, n)] += tim[std::pair<int, int>(h, n)] - optTime;
			sigma[std::pair<int, int>(h, n)] += pow(tim[std::pair<int, int>(h, n)] - optTime, 2);
			if (tim[std::pair<int, int>(h, n)] < bestTime) { bestTime = tim[std::pair<int, int>(h, n)]; }
			if (tim[std::pair<int, int>(h, n)] < optTime) {
				std::cout << "Error!!!!!!!\n"; exit(0);
			}
		}
#endif MIXED
		//output << fns.at(i) << " Optimal time: " << optTime << '\n';
		//if (i == 3)	exit(0);

		for (auto t : tim) if (t.second == bestTime) wins[t.first]++;
#ifdef VALIDATE
		if (bugs > 0) std::cout << bugs << '\n';
		std::cout << "So far...\n";
		for (auto i = 0; i <= 4; i++)
		{
			std::cout << "0 " << i << ": wins = " << wins[std::pair<int, int>(0, i)] << " hits = " << hits[std::pair<int, int>(0, i)] << " avgloss = " << avg_loss[std::pair<int, int>(0, i)] << '\n';
			for (auto j : nums)
			{
				std::cout << j << ' ' << i + 1 << ": wins = " << wins[std::pair<int, int>(j, i)] << " hits = " << hits[std::pair<int, int>(j, i)] << " avgloss = " << avg_loss[std::pair<int, int>(j, i)] << '\n';
				wins[std::pair<int, int>(j, i)];
				hits[std::pair<int, int>(j, i)];
				avg_loss[std::pair<int, int>(j, i)];
				continue;

			}
		}
#endif
		//break; //1 file!
	}
	for (auto i = 0; i <= C_DEPTH; i++) { std::cout << i << ": " << avg_time[i] << '\n'; output << i << ": " << avg_time[i] << '\n';}
	exit(0);
	for (auto j = 0; j <= 21; j++)
	{
		output << j << "Sprecher" << ": wins = " << wins[std::pair<int, int>(j, -2)] << " hits = " << hits[std::pair<int, int>(j, -2)] << " avgloss = " << avg_loss[std::pair<int, int>(j, -2)] / FILES<< " delta = " << delta[std::pair<int, int>(j, -2)] / FILES << " sigma = " << sqrt(sigma[std::pair<int, int>(j, -2)] / FILES) << '\n';
		
	}
	for (auto i = -1; i <= std::max(DEPTH, C_DEPTH); i++)
	{
		
		for (auto k = 0; k <= 2; k++) output << -k <<" " << i << ": wins = " << wins[std::pair<int, int>(-k, i)] << " hits = " << hits[std::pair<int, int>(-k, i)] << " avgloss = " << avg_loss[std::pair<int, int>(-k, i)] / FILES << " delta = " << delta[std::pair<int, int>(-k, i)] / FILES << " sigma = " << sqrt(sigma[std::pair<int, int>(-k, i)] / FILES) << '\n';
		for (auto j : nums)
		{
			output << j << ' ' << i+1 << ": wins = " << wins[std::pair<int, int>(j, i)] << " hits = " << hits[std::pair<int, int>(j, i)] << " avgloss = " << avg_loss[std::pair<int, int>(j, i)] / FILES << " delta = " << delta[std::pair<int, int>(j, i)] / FILES << " sigma = " << sqrt(sigma[std::pair<int, int>(j, i)] / FILES) << '\n';
			wins[std::pair<int, int>(j, i)];
			hits[std::pair<int, int>(j, i)];
			avg_loss[std::pair<int, int>(j, i)];
		}
	}
	output.close();
	return;
}

int main()
{
	std::cout << "Specify problem directory name:" << "\n";
	std::string dir; std::getline(std::cin, dir);
	if (dir == "") { dir = "ResearchTests30"; }
	std::cout << dir << '\n';
	BSTR lp = _com_util::ConvertStringToBSTR(dir.c_str());
	LPWSTR bb = lp;
	std::vector<std::string> fns = readDirectory(bb);
	
	OutputResults(fns, dir);
	return 0;
}

