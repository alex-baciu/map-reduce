#pragma once
#include "mpi.h"
#include <stdio.h> 
#include <string> 
#include <iostream>
#include <filesystem>
#include <fstream>
#include<vector>
#include <map>
#include <utility>

using namespace std;
namespace fs = experimental::filesystem;

/*functii file system*/
vector<string> ReadFile(string filePath);
vector<string> ReadDirectory(string directoryPath);
void WriteFile(string path, map<string, map<string, int>> map);
void DeleteDirectory(string path);

/*functii map-reduce*/
map<string, map<string, int>> MapWithDirectIndex(vector<string> filenames);
vector<pair<string, pair<string, int>>> GetInversedPairsOfDirectIndexesMap(map<string, map<string, int>> directMap);
map<string, map<string, int>> Reduce(vector<pair<string, pair<string, int>>> inversedSortedPairs);
void GenerateMapByResponsabilities(vector<pair<string, pair<string, int>>> inversedPairs, vector<vector<char>> responsabilities, int p, map<int, string>& responsabilitiesMap);
void GenerateTemporaryFiles(map<int, string>responsabilitiesMap, string tempPath, int processRank);
map<string, map<string, int>> ReadTemporaryFiles(string path, int processRank, int p);

/*functii utilitare*/
vector<string> Split(string text, string operators);
bool IsCharacterInString(string text, char c);