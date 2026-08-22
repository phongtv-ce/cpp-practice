#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

namespace fs = filesystem;

const long DEFAULT_MEM_KIB = 500 * 1024; // 500 MiB
const char *TMP_DIR = "./tmp";

struct Args
{
	string inputPath = "input.txt";
	string outputPath = "output.txt";
	long memKiB = DEFAULT_MEM_KIB;
};

// One line in the merge heap, plus which run file it came from.
struct HeapEntry
{
	string line;
	int runIndex;
};

// Make std::make_heap behave as a min-heap (smallest line first).
bool isGreater(const HeapEntry &a, const HeapEntry &b)
{
	return a.line > b.line;
}

void printUsage(const char *programName)
{
	cout << "Usage: " << programName << " [input output mem_kib]\n"
		 << "  input    unsorted text file (default: input.txt)\n"
		 << "  output   sorted result (default: output.txt)\n"
		 << "  mem_kib  memory budget per run, in KiB (default: " << DEFAULT_MEM_KIB << ")\n";
}

// Parse CLI. Returns false if arguments are invalid.
bool parseArgs(int argc, char **argv, Args &args)
{
	if (argc == 1)
	{
		return true; // keep defaults
	}

	if (argc != 4)
	{
		printUsage(argv[0]);
		return false;
	}

	args.inputPath = argv[1];
	args.outputPath = argv[2];

	char *end = nullptr;
	args.memKiB = strtol(argv[3], &end, 10);
	if (end == argv[3] || *end != '\0' || args.memKiB <= 0)
	{
		cerr << "mem_kib must be a positive integer.\n";
		printUsage(argv[0]);
		return false;
	}

	cout << "Memory limit: " << args.memKiB << " KiB\n";
	cout << "Input:  " << args.inputPath << "\n";
	cout << "Output: " << args.outputPath << "\n";
	return true;
}

string tmpRunPath(int runIndex)
{
	return string(TMP_DIR) + "/" + to_string(runIndex);
}

bool writeLines(const string &path, const vector<string> &lines)
{
	ofstream out(path);
	if (!out)
	{
		cerr << "Cannot write: " << path << "\n";
		return false;
	}
	for (const string &line : lines)
	{
		out << line << '\n';
	}
	return static_cast<bool>(out);
}

bool createTempDir()
{
	error_code err;
	fs::remove_all(TMP_DIR, err);
	fs::create_directory(TMP_DIR, err);
	if (err)
	{
		cerr << "Cannot create " << TMP_DIR << ": " << err.message() << "\n";
		return false;
	}
	return true;
}

void removeTempDir()
{
	error_code err;
	fs::remove_all(TMP_DIR, err);
}

// Read lines until estimated memory reaches the budget (~90% of mem_kib).
vector<string> readNextRun(ifstream &input, uint64_t budgetBytes)
{
	vector<string> run;
	string line;
	uint64_t usedBytes = 0;

	while (getline(input, line))
	{
		usedBytes += sizeof(string) + line.capacity();
		run.push_back(move(line));
		if (usedBytes >= budgetBytes)
		{
			break;
		}
	}
	return run;
}

// Split input into sorted run files under ./tmp/.
// Returns number of run files, or -1 on error.
// Special cases: empty input or one run that fits in memory -> write outputPath
// directly and return 0 (no merge needed).
int createSortedRuns(ifstream &input, const string &outputPath, uint64_t budgetBytes)
{
	int runCount = 0;

	while (true)
	{
		vector<string> run = readNextRun(input, budgetBytes);

		if (run.empty())
		{
			if (runCount == 0)
			{
				cout << "Input is empty.\n";
				if (!writeLines(outputPath, run))
				{
					return -1;
				}
			}
			break;
		}

		sort(run.begin(), run.end());

		// Whole file fits in one run -> write output directly.
		if (input.peek() == EOF && runCount == 0)
		{
			cout << "File fits in memory; writing output directly.\n";
			if (!writeLines(outputPath, run))
			{
				return -1;
			}
			return 0;
		}

		cout << "Writing sorted run: tmp/" << runCount << "\n";
		if (!writeLines(tmpRunPath(runCount), run))
		{
			return -1;
		}
		runCount++;
	}

	return runCount;
}

// Merge sorted run files ./tmp/0, ./tmp/1, ... into outputPath using a min-heap.
bool mergeRuns(const string &outputPath, int runCount)
{
	if (runCount <= 0)
	{
		return writeLines(outputPath, {});
	}

	vector<ifstream> runFiles(static_cast<size_t>(runCount));
	for (int i = 0; i < runCount; ++i)
	{
		runFiles[static_cast<size_t>(i)].open(tmpRunPath(i));
		if (!runFiles[static_cast<size_t>(i)])
		{
			cerr << "Cannot open: " << tmpRunPath(i) << "\n";
			return false;
		}
	}

	ofstream output(outputPath);
	if (!output)
	{
		cerr << "Cannot write: " << outputPath << "\n";
		return false;
	}

	// Put the first line of each run into the heap.
	vector<HeapEntry> heap;
	for (int i = 0; i < runCount; ++i)
	{
		HeapEntry entry;
		if (getline(runFiles[static_cast<size_t>(i)], entry.line))
		{
			entry.runIndex = i;
			heap.push_back(move(entry));
		}
	}

	if (heap.empty())
	{
		return true;
	}

	make_heap(heap.begin(), heap.end(), isGreater);

	while (!heap.empty())
	{
		pop_heap(heap.begin(), heap.end(), isGreater);
		HeapEntry smallest = move(heap.back());
		heap.pop_back();

		output << smallest.line << '\n';

		// Read next line from the same run file.
		HeapEntry next;
		if (getline(runFiles[static_cast<size_t>(smallest.runIndex)], next.line))
		{
			next.runIndex = smallest.runIndex;
			heap.push_back(move(next));
			push_heap(heap.begin(), heap.end(), isGreater);
		}
	}

	return static_cast<bool>(output);
}

bool externalSort(const Args &args)
{
	if (!createTempDir())
	{
		return false;
	}

	ifstream input(args.inputPath);
	if (!input)
	{
		cerr << "File not found: " << args.inputPath << "\n";
		removeTempDir();
		return false;
	}

	const uint64_t budgetBytes = static_cast<uint64_t>(args.memKiB) * 1024ULL * 90ULL / 100ULL;
	const int runCount = createSortedRuns(input, args.outputPath, budgetBytes);

	bool ok = (runCount >= 0);
	if (ok && runCount > 0)
	{
		cout << "Merging " << runCount << " runs into output.\n";
		ok = mergeRuns(args.outputPath, runCount);
	}

	removeTempDir();
	return ok;
}

int main(int argc, char **argv)
{
	Args args;
	if (!parseArgs(argc, argv, args))
	{
		return 1;
	}

	if (!externalSort(args))
	{
		return 1;
	}

	cout << "Completed!\n";
	return 0;
}
