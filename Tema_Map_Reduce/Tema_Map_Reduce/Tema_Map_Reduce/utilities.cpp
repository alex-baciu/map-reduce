#include "utilities.h"

vector<string> ReadFile(string filePath)
{
	std::ifstream file(filePath);
	vector<string> lines;

	if (file.is_open())
	{
		std::string line;
		while (std::getline(file, line))
		{
			lines.push_back(line);
		}
		file.close();
	}

	return lines;
}

vector<string> ReadDirectory(string directoryPath) 
{
	vector<string> filenames;

	for (const auto & filePath : fs::directory_iterator(directoryPath.data()))
	{
		if (filePath.path().generic_string().find(".txt"))
		{
			filenames.push_back(filePath.path().generic_string());
		}
	}
	
	return filenames;
}

void DeleteDirectory(string path) 
{
	fs::remove_all(path);
}

void WriteFile(string path, map<string, map<string, int>> map)
{
	for (const auto& wordToPairMap : map)
	{
		fstream file(path + wordToPairMap.first + ".txt", ios::out);
		
		for (const auto& fileToCountMap : wordToPairMap.second)
		{
			file << fileToCountMap.first.c_str() << ": " << fileToCountMap.second << "\n";
		}

		file.close();
	}
}

vector<string> UniversalSplit(const string &text)
{
	vector<string> words;
	int startIndex = 0;
	int count = 0;

	for (unsigned int i = 0; i < text.size(); i++)
	{
		if (text[i] == ' ' || text[i] == '?' || text[i] == ',' || text[i] == '!' || text[i] == '\n')
		{
			if (count != 0)
			{
				words.push_back(text.substr(startIndex, count));
				count = 0;
			}
			startIndex = i + 1;
		}
		else
		{
			count++;
		}
	}

	if (count != 0)
	{
		words.push_back(text.substr(startIndex, count));
	}

	return words;
}

map<string, map<string, int>> MapWithDirectIndex(vector<string> filenames)
{
	map<string, map<string, int>> result;

	for (unsigned int i = 0; i < filenames.size(); i++)
	{
		vector<string> textLines = ReadFile(filenames[i]);

		result.insert(pair<string, map<string, int>>(filenames[i], map<string, int>()));

		for (unsigned int j = 0; j < textLines.size(); j++)
		{
			map<string, int>& valuesMap = (*result.find(filenames[i])).second;
			vector<string> words = UniversalSplit(textLines[j]);

			for (unsigned int k = 0; k < words.size(); k++)
			{
				if (valuesMap.find(words[k]) == valuesMap.end())
				{
					valuesMap.insert(pair < string, int>(words[k], 1));
				}
				else
				{
					(*valuesMap.find(words[k])).second++;					
				}
			}
		}
	}

	return result;
}

vector<pair<string, pair<string, int>>> GetInversedPairsOfDirectIndexesMap(map<string, map<string, int>> directMap)
{
	vector<pair<string, pair<string, int>>> result;

	for (auto&fileToValuesMap : directMap)
	{
		for (auto&wordToCountMap : fileToValuesMap.second)
		{
			result.push_back(pair<string, pair<string, int>>
				(wordToCountMap.first, pair<string, int>(fileToValuesMap.first, wordToCountMap.second)));
		}
	}

	return result;
}

map<string, map<string, int>> Reduce(vector<pair<string, pair<string, int>>> inversedSortedPairs) 
{
	map<string, map<string, int>> result;

	for (const auto&it : inversedSortedPairs)
	{
		auto filenameToCountPair = result.find(it.first);

		if (result.find(it.first) == result.end())
		{

			result.insert(pair<string, map<string, int>>(it.first, map<string, int>()));
			filenameToCountPair = result.find(it.first);
		}

		auto file_it_inverse = (*filenameToCountPair).second.find(it.second.first);
		if (file_it_inverse == (*filenameToCountPair).second.end())
		{

			(*filenameToCountPair).second.insert(pair<string, int>(it.second.first, it.second.second));
		}
	}

	return result;
}

void GenerateMapByResponsabilities(vector<pair<string, pair<string, int>>> inversedPairs, vector<vector<char>> responsabilities, int p, map<int, string>& responsabilitiesMap)
{
	for (int i = 1; i < p; ++i)
	{
		responsabilitiesMap.insert(pair<int, string>(i, string()));
	}

	for (const auto& wordPair : inversedPairs)
	{
		bool isLetter = false;

		for (unsigned int i = 0; i < responsabilities.size() && isLetter == false; ++i)
		{
			for (const auto &responsability : responsabilities[i])
			{
				if (responsability == wordPair.first.c_str()[0])
				{
					isLetter = true;

					responsabilitiesMap[i + 1] += wordPair.first + " " +
						wordPair.second.first + " " +
						to_string(wordPair.second.second) + "\n";

					break;
				}
			}

			if (isLetter == true)
			{
				break;
			}
		}

		if (isLetter == false)
		{
			responsabilitiesMap[p - 1] += wordPair.first + " " +
				wordPair.second.first + " " +
				to_string(wordPair.second.second) + "\n";
		}
	}
}

void GenerateTemporaryFiles(map<int, string>responsabilitiesMap, string tempPath, int processRank)
{
	for (unsigned int i = 1; i <= responsabilitiesMap.size(); ++i)
	{
		fstream file;
		file.open(tempPath + to_string(i) + " " + to_string(processRank) + ".txt", ios::out);

		file << (*responsabilitiesMap.find(i)).second;

		file.close();
	}
}

map<string, map<string, int>> ReadTemporaryFiles(string path, int processRank, int p)
{
	map<string, map<string, int>>result;

	for (int i = 1; i <= p; ++i)
	{
		fstream file;
		file.open(path + to_string(processRank) + " " + to_string(i) + ".txt", ios::in);

		string line;

		while (getline(file, line))
		{
			vector<string> wordPair = UniversalSplit(line);

			if (wordPair.size() == 0)
				break;

			string word = wordPair[0];

			if (result.find(word) == result.end())
			{
				result.insert(pair<string, map<string, int>>(word, map<string, int>()));
			}


			if ((*result.find(word)).second.find(wordPair[1]) == (*result.find(word)).second.end())
			{
				(*result.find(word)).second.insert(pair<string, int>(wordPair[1], 0));
			}

			(*(*result.find(word)).second.find(wordPair[1])).second += atoi(wordPair[2].c_str());
		}

		file.close();
	}

	return result;
}

