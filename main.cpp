
// TEST FOR SOFTWARE ENGINEER- WEBSEARCH TEAM
// Nguyen Manh Cuong - cuongnmse05630@fpt.edu.vn

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <vector>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <sys/types.h>
#include <dirent.h>
#include <algorithm>
#include <queue>

using namespace std;

const string TEMP_DIR = "./tmp/";
const string STR_MAX = "\255";

// ============ Min Heap ============

struct Node {
    string value;
    int index_file;
};

class Heap {
    vector<Node> heap_arr;
    int heap_size = 0;

public:
    Heap(){}

    void build();
    void swap(Node &node_1, Node &node_2);
    void heapify(int index);
    void add(Node &node);
    Node top();
    void pop();
    void replaceTop(Node node);
    void display();

    int size() { return heap_size; }
    int left(int index) { return 2 * index + 1; }
    int right(int index) { return 2 * index + 2; }
};

void Heap::add(Node &node) {
    heap_arr.push_back(node);
    heap_size++;
    build();
}

void Heap::heapify(int index) {
    int left_ = left(index);
    int right_ = right(index);
    int min = index;

    if(left_ < heap_size && heap_arr[left_].value < heap_arr[index].value) {
        min = left_;
    }

    if(right_ < heap_size && heap_arr[right_].value < heap_arr[min].value) {
        min = right_;
    }

    if(min != index) {
        swap(heap_arr[min], heap_arr[index]);
        heapify(min);
    }
}

void Heap::swap(Node &node_1, Node &node_2) {
    Node tmp = node_1;
    node_1 = node_2;
    node_2 = tmp;
}

void Heap::build() {
    int mid = (heap_size -1)/2;
    for(int i = mid; i >=0; i--) {
        heapify(i);
    }
}

void Heap::display() {
    for(auto &node : heap_arr) {
        cout << node.value << endl;
    }
    cout << endl;
}

Node Heap::top() {
    return heap_arr[0];
}

void Heap::replaceTop(Node node) {
    heap_arr[0] = node;
    heapify(0);
}

void Heap::pop() {
    heap_arr[0].value = STR_MAX;
    heapify(0);
}

// ========================================

/**
 * Compare two strings lexicographically, callback function for std::sort algorithm.
 * @param str_1     string.
 * @param str_2     another string.
 * @return True if str_1 stands before str_2, false otherwise.
 */
bool compare(const string &str_1, const string &str_2) {
    return str_1 < str_2;
}

/**
 * Sort vector of strings and save it to temporary file.
 * @param outFile   temporary file.
 * @param lines     vector of strings.
 */
void sortAndSave(std::ofstream &outFile, vector<string> &lines) {

    sort(lines.begin(), lines.end(), compare);

    for(int i = 1; i < lines.size(); i++) {
        outFile << lines[i] << '\n';
    }

    outFile.close();
}

/**
 * Segment main file into temporary files, each file is limited by a maximum of bytes.
 * @param path_file_in      directory path of main file.
 * @param minHeap           Min Heap
 * @param memoryLimit       maximum of bytes.
 */
void segmentFileToRuns(const string path_file_in,
                       Heap &minHeap,
                       const unsigned int memoryLimit)
{
    ifstream inFile(path_file_in.c_str());

    vector<string> lines;

    string line;
    ofstream tempFile;

    int numOfReadingBytes = 0,
            numOfTempFiles = 0;

    Node newNode;

    // Start segmenting

    while(getline(inFile, line)) {
        lines.push_back(line);
        numOfReadingBytes += line.size();

        if(numOfReadingBytes >= memoryLimit) {
            string out_file = TEMP_DIR + "tmp-" + to_string(numOfTempFiles) + ".txt";

            tempFile.open(out_file.c_str());
            sortAndSave(tempFile, lines);

            // Get first line of sorted array as a key to sort min heap
            newNode.value = lines[0];

            // We can't save ifstream variable - tempFile as a value
            // because it can point to multiple file
            // save index of file to retrieve temporary file corresponding to the string key line.
            newNode.index_file = numOfTempFiles;

            minHeap.add(newNode);

            lines.clear();
            numOfReadingBytes = 0;
            numOfTempFiles++;
        }
    }


    if(numOfReadingBytes != 0) {
        string out_file = TEMP_DIR + "tmp-" + to_string(numOfTempFiles) + ".txt";
        tempFile.open(out_file.c_str());
        sortAndSave(tempFile, lines);
        newNode.value = lines[0];
        newNode.index_file = numOfTempFiles;
        minHeap.add(newNode);
    }

    inFile.close();
}

/**
 * Merge temporary files into one
 * @param outFile   output file
 * @param minHeap   Min Heap
 */
void mergeFilesToFile(
        ofstream &outFile,
        Heap &minHeap)
{
    int numOfTempFiles = minHeap.size();
    string newLine;
    auto * tempFiles = new ifstream[numOfTempFiles];
    vector<string> a;

    // Initialize ready temporary files to read.
    for(int i = 0; i < numOfTempFiles; i++) {
        string out_file = TEMP_DIR + "tmp-" + to_string(i) + ".txt";
        tempFiles[i].open(out_file.c_str());
    }

    int loop = 0;

    // Start merging

    while(loop < numOfTempFiles) {
        Node entry = minHeap.top();


        string line = entry.value;
        ifstream * inTempFile =  &tempFiles[entry.index_file];
        outFile << line << '\n';

        // Read next line and push it into min heap to retrieve min line later.
        // If file is EOF, pop min heap to retrieve next min line in heap later.
        if(getline(*inTempFile, newLine)) {
            entry.value = newLine;
        } else {
            entry.value = "\255";
            loop++;
        }

        minHeap.replaceTop(entry);
    }


    for(int i = 0; i < numOfTempFiles; i++) {
        tempFiles[i].close();
    }

    outFile.close();
}

/**
 * Get the size of a file.
 * @param filename The name of the file to check size for
 * @return The filesize, or 0 if the file does not exist.
 */
size_t getFilesize(const std::string& filename)
{
    struct stat st;
    if(stat(filename.c_str(), &st) != 0) {
        return 0;
    }
    return st.st_size;
}

/**
 * Remove a directory and all sub files of it.
 * @param path  directory path of removing directory.
 */
void removeDir(const char *path)
{
    struct dirent *entry = NULL;
    DIR *dir = NULL;
    dir = opendir(path);

    if(dir) {
        while(entry = readdir(dir))
        {
            DIR *sub_dir = NULL;
            FILE *file = NULL;
            char abs_path[100] = {0};
            if(*(entry->d_name) != '.')
            {
                sprintf(abs_path, "%s/%s", path, entry->d_name);
                if(sub_dir = opendir(abs_path))
                {
                    closedir(sub_dir);
                    removeDir(abs_path);
                }
                else
                {
                    if(file = fopen(abs_path, "r"))
                    {
                        fclose(file);
                        remove(abs_path);
                    }
                }
            }
        }
        remove(path);
    }

}

/**
 * Prepare for running
 * include removing old temporary directory and creating new temporary directory
 */
void prepare()
{
    removeDir(TEMP_DIR.c_str());
    const int dir_err = mkdir(TEMP_DIR.c_str(), 0777);
    if (-1 == dir_err)
    {
        printf("Error creating directory!n");
        exit(1);
    }
}

/**
 * External sorting
 * @param path_file_in      directory path of file input.
 * @param path_file_out     directory path of file output.
 * @param memoryLimit       maximum of bytes.
 */
void externalSorting(const string path_file_in,
                     const string path_file_out,
                     const unsigned int memoryLimit)
{

    Heap minHeap;
    prepare();
    ofstream outFile(path_file_out.c_str(), ofstream::out);

    segmentFileToRuns(path_file_in, minHeap, memoryLimit);
    mergeFilesToFile(outFile, minHeap);

    removeDir(TEMP_DIR.c_str());
}

/**
 * Read lines directly into memory, sort and write to file.
 * @param path_file_in      directory path of file input.
 * @param path_file_out     directory path of file output.
 */
void sortDirectly(const string path_file_in,
                  const string path_file_out) {

    ifstream inFile(path_file_in.c_str());
    vector<string> lines;
    string line;

    while(getline(inFile, line)) {
        lines.push_back(line);
    }
    inFile.close();
    ofstream outFile(path_file_out.c_str());
    sortAndSave(outFile, lines);
}

int main(int argc, char* argv[]) {

    // ============ Read arguments ============
    if (argc < 4) {
        cout << "Missing parameters" << endl;
        cout << "Usage:" << endl;
        cout << "coccoc [input file name] [output file name] [memory limit]" << endl;
        return -1;
    }

    const std::string path_file_in = argv[1];
    const std::string path_file_out = argv[2];
    const unsigned int memoryLimit = atoi(argv[3]);

    // ========================================

    clock_t begin = clock();

    // ============ Begin sorting ============
    size_t file_size = getFilesize(path_file_in);


    if (file_size < memoryLimit) {
        sortDirectly(path_file_in, path_file_out);
    } else {
        externalSorting(path_file_in, path_file_out, memoryLimit);
    }

    // ========================================

    cout << "====== Time: " << float(clock() - begin) / CLOCKS_PER_SEC * 1000 << " msec ======" << endl;

    return 1;
}