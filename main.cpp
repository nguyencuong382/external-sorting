
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
const string PREFIX_= "tmp-";
const string EXT = ".txt";

// ============ Min Heap ============

struct Node {
    string value;
    int index_file;
};

class Heap {
    vector<Node> heap_arr;
    int heap_size = 0;

public:

    // build a min heap tree from heap array
    void build();

    // swap two nodes
    void swap(Node &node_1, Node &node_2);

    // heapify the node with given index
    void heapify(int index);

    // add a new node to the heap
    void add(Node &node);

    // get root node
    Node top();

    // delete root node
    void pop();

    // shift down the node with given index
    void shiftDown(int index);

    // replace root node with new node
    void replaceTop(Node node);

    int size() { return heap_size; }
    int left(int index) { return 2 * index + 1; }
    int right(int index) { return 2 * index + 2; }

    ~Heap(){ heap_arr.clear(); }
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

Node Heap::top() {
    // To ensure working correctly, checking heap size is needed
    // However, heap serves merging files algorithm which check size before
    // so return root of heap directly
    return heap_arr[0];
}

void Heap::replaceTop(Node node) {
    heap_arr[0] = node;
    heapify(0);
}

void Heap::pop() {
    if(heap_size <= 0) {
        return ;
    }
    heap_arr[0] = heap_arr[--heap_size];
    heap_arr.pop_back();
    if(heap_size > 0) {
        shiftDown(0);
    }
}

void Heap::shiftDown(int index) {
    int left_ = left(index);
    int right_ = right(index);
    int min;

    if(right_ >= heap_size) {
        if(left_ >= heap_size) {
            return ;
        } else {
            min = left_;
        }
    } else {
        if(heap_arr[left_].value > heap_arr[right_].value) {
            min = right_;
        } else {
            min = left_;
        }
    }

    if(heap_arr[index].value > heap_arr[min].value) {
        swap(heap_arr[index], heap_arr[min]);
        shiftDown(min);
    }
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
 * Close file when done
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
 * Close file when done
 * @param inFile            input file.
 * @param minHeap           Min Heap
 * @param memoryLimit       maximum of bytes.
 */
void segmentFileToRuns(ifstream &inFile,
                       Heap &minHeap,
                       const unsigned int memoryLimit)
{
    vector<string> lines;

    string  line,
            out_file;

    ofstream tempFile;

    int numOfReadingBytes = 0,
        numOfTempFiles = 0;

    Node newNode;

    // Start segmenting

    while(getline(inFile, line)) {
        lines.push_back(line);
        numOfReadingBytes += line.size();

        if(numOfReadingBytes >= memoryLimit) {
            out_file = TEMP_DIR + PREFIX_ + to_string(numOfTempFiles) + EXT;

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
        out_file = TEMP_DIR + PREFIX_ + to_string(numOfTempFiles) + EXT;
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
 * Close file when done
 * @param outFile   output file
 * @param minHeap   Min Heap
 */
void mergeFilesToFile(ofstream &outFile, Heap &minHeap)
{
    int numOfTempFiles = minHeap.size();

    string  newLine,
            out_file;

    auto * tempFiles = new ifstream[numOfTempFiles];

    vector<string> a;

    // Initialize ready temporary files to read.
    for(int i = 0; i < numOfTempFiles; i++) {
        out_file = TEMP_DIR + PREFIX_ + to_string(i) + EXT;
        tempFiles[i].open(out_file.c_str());
    }

    // Start merging

    while(minHeap.size() > 0) {
        Node entry = minHeap.top();

        outFile << entry.value << '\n';
        ifstream * inTempFile =  &tempFiles[entry.index_file];

        // Read next line and push it into min heap to retrieve min line later.
        // If file is EOF, pop min heap to retrieve next min line in heap later.
        if(getline(*inTempFile, entry.value)) {
            minHeap.replaceTop(entry);
        } else {
            minHeap.pop();
        }
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
        cout << "Error creating directory!" << endl;
        exit(1);
    }
}

/**
 * External sorting
 * @param path_file_in      directory path of file input.
 * @param path_file_out     directory path of file output.
 * @param memoryLimit       maximum of bytes.
 */
void externalSorting(const char * path_file_in,
                     const char * path_file_out,
                     const unsigned int memoryLimit)
{
    prepare();

    Heap minHeap;
    ifstream inFile(path_file_in);

    if(!inFile.is_open()) {
        cout << "Can't open reading file: " << path_file_in << endl;
        exit(-1);
    }

    cout << "Segmenting file ..." << endl;
    segmentFileToRuns(inFile, minHeap, memoryLimit);

    ofstream outFile(path_file_out);
    if(!outFile.is_open()) {
        cout << "Can't open writing file: " << path_file_in << endl;
        exit(-1);
    }

    cout << "Merging files ..." << endl;
    mergeFilesToFile(outFile, minHeap);

    removeDir(TEMP_DIR.c_str());
}

/**
 * Read lines directly into memory, sort and write to file.
 * @param path_file_in      directory path of file input.
 * @param path_file_out     directory path of file output.
 */
void sortDirectly(const char * path_file_in,
                  const char * path_file_out) {

    ifstream inFile(path_file_in);
    vector<string> lines;
    string line;

    while(getline(inFile, line)) {
        lines.push_back(line);
    }
    inFile.close();
    ofstream outFile(path_file_out);
    sortAndSave(outFile, lines);
}

int main(int argc, char* argv[]) {

    // ============ Read arguments ============
    if (argc < 4) {
        cout << "Missing parameters" << endl;
        cout << "Usage:" << endl;
        cout << "coccoc [input file name] [output file name] [memory limit in bytes]" << endl;
        return -1;
    }

    const std::string path_file_in = argv[1];
    const std::string path_file_out = argv[2];
    const unsigned int memoryLimit = atoi(argv[3]);

    // ========================================

    clock_t begin = clock();
    size_t file_size = getFilesize(path_file_in);

    // ============ Begin sorting ============

    if(file_size == 0 ) {
        cout << "Data emtpy!" << endl;
    } else if (file_size <= memoryLimit) {
        cout << "--- Start sorting directly in memory ---" << endl;
        sortDirectly(path_file_in.c_str(), path_file_out.c_str());
    } else {
        cout << "--- Start sorting by external sorting ---" << endl;
        externalSorting(path_file_in.c_str(), path_file_out.c_str(), memoryLimit);
    }

    // ========================================

    cout << "====== Time: " << float(clock() - begin) / CLOCKS_PER_SEC * 1000 << " msec ======" << endl;

    return 1;
}
