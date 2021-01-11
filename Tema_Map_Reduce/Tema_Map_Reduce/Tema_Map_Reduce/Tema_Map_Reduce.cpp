#include "utilities.h"

int main(int argc, char * argv[])
{
	if (argc != 3)
	{
		cout << "Application should have absolute test directory path and absolute results directory path.";
		return 0;
	}

/*********initializare path-uri*********/
	string absolutePathOfDirectory(argv[1]);
	string absolutePathOfWriteDirectory(argv[2] + string("\\"));

	int my_rank; 
	int p; 
	int tag = 0;
	const int maxBufferCapacity = 1000;

	MPI_Status status; 
	MPI_Init(&argc, &argv);     
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);     
	MPI_Comm_size(MPI_COMM_WORLD, &p);

/*********initializare date stare*********/

	/*initializare vector responsabilitate in functie de alfabet*/
	vector<vector<char>> alfabetResponsability;
	for (int i = 1; i < p - 1; ++i)
	{
		alfabetResponsability.push_back(vector<char>());
	}

	/*distribuire responsabilitati pentru procesle cu 0 < id < nrProc,
	procesul 0 = master, procesul p - 1 se ocupa de orice alt caracter*/
	for (char c = 'a'; c <= 'z'; ++c)
	{
		alfabetResponsability[(c - 'a') % (p - 2)].push_back(c);
	}

/*********proces master*********/
	if (my_rank == 0)
	{
		/*initializare vector responsabilitati in functie de proces - fisier, distributie egala*/
		vector<string> filenames = ReadDirectory(absolutePathOfDirectory);
		vector<vector<string>> processReadFiles;
		for (int i = 1; i < p; ++i)
		{
			processReadFiles.push_back(vector<string>());
		}

		/*responsabilitati in functie de proces - fisier, distributie egala*/
		for (unsigned int i = 0; i < filenames.size(); ++i)
		{
			processReadFiles[i % (p - 1)].push_back(filenames[i]);
		}

		/*trimitere responsabilitati fisier in functie de proces*/
		for (int i = 1; i < p; ++i)
		{
			string responsabilitiesOfProcess = "";

			for (unsigned int j = 0; j < processReadFiles[i - 1].size(); ++j)
			{
				responsabilitiesOfProcess += processReadFiles[i - 1][j] + "\n";
			}

			MPI_Send(responsabilitiesOfProcess.c_str(),
				strlen(responsabilitiesOfProcess.c_str()) + 1, MPI_CHAR, i, tag, MPI_COMM_WORLD);
		}

		/*asteptare semnal fisiere temporare pregatite*/
		int semnal = -1;
		for (int i = 1; i < p; ++i)
		{			
			MPI_Recv(&semnal, 1, MPI_INT, i, tag, MPI_COMM_WORLD, &status);

			/*verificare eroare aparuta*/
			if (semnal != 0)
			{
				MPI_Finalize();
				exit(1);
			}
		}		

		/*trimitere semnal start reducere fisiere temporare*/
		for (int i = 1; i < p; ++i)
		{
			semnal = 0;
			MPI_Send(&semnal, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
		}

		/*asteptare semnal procesare finalizata*/
		semnal = -1;
		for (int i = 1; i < p; ++i)
		{
			MPI_Recv(&semnal, 1, MPI_INT, i, tag, MPI_COMM_WORLD, &status);
			/*verificare eroare aparuta*/
			if (semnal != 0)
			{
				MPI_Finalize();
				exit(1);
			}
		}

		/*stergere director cu fisiere temporare*/
		DeleteDirectory(absolutePathOfWriteDirectory+"TEMP");	
	}
/*********proces worker*********/
	else
	{
		/*preluare mesaj reponsabilititate fisiere de citit*/
		char buffer[maxBufferCapacity];
		MPI_Recv(buffer, maxBufferCapacity, MPI_CHAR, 0, tag, MPI_COMM_WORLD, &status);

		/*procesare reponsabilititate fisiere de citit*/
		string filenamesMessage(buffer);
		vector<string> filenames = Split(filenamesMessage, string(" \n"));

		/*generare direct map din fisierele primite*/
		map<string, map<string, int>> mapWithDirectIndexes = MapWithDirectIndex(filenames);

		/*generare perechi dupa index-ul invers*/
		vector<pair<string, pair<string, int>>> inversedPairsOfDirectMap =
			GetInversedPairsOfDirectIndexesMap(mapWithDirectIndexes);

		/*generare map de trimis catre procesele care se ocupa de anumite cuvinte*/
		map<int, string> responsabilityMap;
		GenerateMapByResponsabilities(inversedPairsOfDirectMap, alfabetResponsability, p, responsabilityMap);

		/*creare director temporar*/
		fs::create_directory(absolutePathOfWriteDirectory + "TEMP");

		/*generare fisiere temporare pe baza responsabilitatilor*/
		GenerateTemporaryFiles(responsabilityMap, absolutePathOfWriteDirectory + "TEMP\\", my_rank);

		/*trimitere semnal terminare generare fisiere temporare*/
		int semnal = 0;
		MPI_Send(&semnal, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);

		/*asteptare semnal continuare cu reducerea fisierelor temporare*/
		MPI_Recv(&semnal, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);

		/*verificare eroare aparuta*/
		if (semnal != 0)
		{
			MPI_Finalize();
			exit(1);
		}

		/*generare mapare cu indexare inversa din fisierele temporare*/
		map<string, map<string, int>> mapWithInderectDirectIndexes =
			ReadTemporaryFiles(absolutePathOfWriteDirectory + "\\TEMP\\", my_rank, p);

		/*scriere fisiere finale*/
		WriteFile(absolutePathOfWriteDirectory, mapWithInderectDirectIndexes);

		/*trimitere semnal final*/
		semnal = 0;
		MPI_Send(&semnal, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
	}
	
	MPI_Finalize();

	return 0;
}