// This is the master file. Edit THIS as the latest.

// Includes
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>

#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <taglib/mpegfile.h>
#include <taglib/tag.h>

#include <fstream>
#include <iostream>
#include <algorithm>
#include <random>

// Namespaces.
using namespace std;
namespace fs = filesystem;

// Function definitions below.
void addMP3ToVector(const fs::path& directory, vector<string>& files);
int grabLength (const string& filepath, const string& filename);
void writePlaylist(const vector<pair<string, int>>& fileLengths, const string& directory, ostream& output);
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
string encodeURL(const string& input);
void shuffleVector(vector<pair<string, int>>& vec);

// Main
int main() {
    // Variables that get changed by functions.
    vector<pair<string, int>> fileLengths;
    vector<string> audioFiles;
    fs::path dirPath = "audio/"; // Directory to pull audio files from.
    const char *tempfile = "tempfilemp3";

    // Initialise URL to 'scrape' files from. 
    ifstream readURL("url.txt");
    string URL;
    int isfp = 0; // Assuming that it isn't a filepath by default.

    if (getline(readURL, URL)) {
        readURL.close();
        if (URL.substr(0,8) != "https://") {
            isfp = 1; // Assume that the contents of url.txt is a filepath.
            fs::path dirPath = URL;
        }
    } else {
        cerr << "Can't read 'url.txt', please check permissions or create it." << endl << "It must be a valid directory or a URL to work." << endl;
    }

    if (fs::exists(dirPath) && fs::is_directory(dirPath)) {
        addMP3ToVector(dirPath, audioFiles);
    
        // Prints list of MP3 files found.
        for (const auto& file : audioFiles) {
            std::cout << file << std::endl; // Logging.

            int length = grabLength(dirPath, file); // Get length of file in seconds.
            
            // Check if the URL replacement is needed.
            string encodedFile = (isfp == 0) ? encodeURL(file) : file;

            std::cout << "Length in Seconds: " << length << std::endl; // Logging.
            fileLengths.push_back(std::make_pair(encodedFile, length)); // Add to final array.
        }

        std::cout << "Done!" << std::endl;

    } else {
        std::cerr << "Program assumes folder 'audio' exists in the same directory." << std::endl;
        std::cerr << "Invalid directory or filepath doesn't exist." << std::endl;
    }

    std::ofstream outputFile("playlist.m3u");
    if (outputFile.is_open()) {

        shuffleVector(fileLengths);
        
        writePlaylist(fileLengths, URL, outputFile);
        outputFile.close();
        std::cout << "Playlist generated successfully!" << std::endl;
    } else {
        std::cerr << "Unable to open file for writing!" << std::endl;
    }

    return 0;
}

// Function code below

void addMP3ToVector(const fs::path& directory, vector<string>& files) {
    for (const auto& entry : fs::directory_iterator(directory)) {
        // Check if file ends with '.mp3'
        // Probably should change this to add other audio filetypes at a later date.
        if (entry.path().extension() == ".mp3") {
            // Get file name without extension.
            string filename = entry.path().stem().string();
            files.push_back((filename));
            // Adds to the list.
        }
    }
}

int grabLength (const string& filepath, const string& filename) {
    string filen = filepath + filename + ".mp3";
    try {
        TagLib::FileRef file(filen.c_str()); // Convert string to const char*
        if (!file.isNull() && file.audioProperties()) {
            // Extracting metadata
            TagLib::AudioProperties* properties = file.audioProperties();
            int durationInSeconds = properties->length();

            return durationInSeconds;

        } else {
            cerr << "Error opening file or extracting audio properties." << endl;
        }
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 0;
    }
}

void writePlaylist(const vector<pair<string, int>>& fileLengths, const string& directory, ostream& output) {
    output << "#EXTM3U \n \n";

    for (const auto& fileInfo : fileLengths) {
        const string& file = fileInfo.first;
        int lengthInSeconds = fileInfo.second;
        output << "#EXTINF:" << lengthInSeconds << ",Author - " << file << endl;
        output << directory << file << ".mp3" << endl << endl;
    }
}

string encodeURL(const std::string& input) {
    ostringstream encoded;
    encoded.fill('0');
    encoded << std::hex;

    for (char ch : input) {
        if (isalnum(static_cast<unsigned char>(ch)) || ch == '-' || ch == '_' || ch == '.' || ch == '~') {
            encoded << ch;
        } else if (ch == ' ') {
            encoded << "%20";
        } else {
            encoded << '%' << setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(ch));
        }
    }

    return encoded.str();
}

void shuffleVector(vector<pair<string, int>>& vec) {
    // Initialize random number generator
    std::random_device rd;
    std::mt19937 g(rd());

    // Shuffle the vector
    std::shuffle(vec.begin(), vec.end(), g);
}
