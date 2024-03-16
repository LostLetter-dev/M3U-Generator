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
#include <algorithm>
#include <random>

// Namespaces.
using namespace std;
namespace fs = filesystem;

// Function definitions below.
void addMP3ToVector(const fs::path& directory, vector<string>& files);
int grabLength (const string& filepath, const string& filename);
void writePlaylist(const vector<pair<string, int>>& fileLengths, const string& directory, ostream& output);
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

    char shuffleChoice; // Make sure that the user actually wants to shuffle the final result.
    bool shuffle;
    while (true) {
        cout << "Do you want to shuffle this playlist? [y/n]" << endl;
        shuffleChoice = getchar();

        while (getchar() != '\n'); // Clear input buffer for invalid responses.

        if (shuffleChoice == 'Y' || shuffleChoice == 'y') {
            cout << "Playlist will be shuffled." << endl;
            shuffle = true;
            break;
        } else if (shuffleChoice == 'N' || shuffleChoice == 'n') {
            cout << "Playlist will not be shuffled." << endl;
            shuffle = false;
            break;
        } else {
            cout << "You did not enter a valid response." << endl;
        }
    }

    if (fs::exists(dirPath) && fs::is_directory(dirPath)) {
        addMP3ToVector(dirPath, audioFiles);
    
        // Prints list of MP3 files found.
        cout << endl << "----- Processing files! This might take a moment! -----" << endl;
        for (const auto& file : audioFiles) {
            cout << "Processing: " << file << endl; // Logging.

            int length = grabLength(dirPath, file); // Get length of file in seconds.
            
            // Check if the URL replacement is needed.
            string encodedFile = (isfp == 0) ? encodeURL(file) : file;

            if (length == 0) {
                cout << "Warning, unable to get " << file << "'s length in seconds." << endl << "This will cause issues in URL playlists." << endl;
            }

            cout << "Length in Seconds: " << length << endl; // Logging.
            fileLengths.push_back(make_pair(encodedFile, length)); // Add to final array.
        }

        cout << endl << "Done!" << endl << endl;

        // Shuffles the final vector.
        if (shuffle) {
            shuffleVector(fileLengths);
        }

    } else {
        cerr << "Program assumes folder 'audio' exists in the same directory." << endl;
        cerr << "Invalid directory or filepath doesn't exist." << endl;
    }

    cout << "Writing to playlist file..." << endl;

    ofstream outputFile("playlist.m3u");
    if (outputFile.is_open()) {
        
        writePlaylist(fileLengths, URL, outputFile);
        outputFile.close();
        cout << "Playlist generated successfully!" << endl;
    } else {
        cerr << "Unable to open file for writing!" << endl;
    }

    string x;
    cout << endl <<  " [ Hit any key to exit... ] " << endl << endl;
    getchar();

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


    cout << "file is invalid or audioProperties returned false." << endl;
    return 0; // Always return 0 if no conditions are met.
    /* Realistically, this should never be called unless the filescanner fails in some way.
    This is here to get rid of a compilation warning.*/
}

void writePlaylist(const vector<pair<string, int>>& fileLengths, const string& directory, ostream& output) {
    output << "#EXTM3U \n \n"; // Required for top of M3U file.

    for (const auto& fileInfo : fileLengths) {
        const string& file = fileInfo.first;
        int lengthInSeconds = fileInfo.second;
        output << "#EXTINF:" << lengthInSeconds << ",Author - " << file << endl; // #EXTINF:60,Author - Filename
        output << directory << file << ".mp3" << endl << endl; // Outputs path to file.
    }
}

string encodeURL(const string& input) {
    ostringstream encoded;
    encoded.fill('0');
    encoded << hex;

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
} // For when the file is a URL.

void shuffleVector(vector<pair<string, int>>& vec) {
    // Initialize random number generator
    random_device rd;
    mt19937 g(rd());

    // Shuffle the vector
    shuffle(vec.begin(), vec.end(), g);
}
