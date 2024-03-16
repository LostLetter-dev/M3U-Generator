#include <iostream>
#include <filesystem>
#include <vector>
#include <string>

#include <fstream>
#include <algorithm>
#include <random>


// Saves me a future headache by ensuring that the file is compatible* on linux and windows.

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#else
#include <sndfile.h>
#endif

// Namespaces.
using namespace std;
namespace fs = std::filesystem;

// Function definitions below.
void addMP3ToVector(const fs::path& directory, vector<string>& files);
int grabLength(const string& filepath, const string& filename);
void writePlaylist(const vector<pair<string, int>>& fileLengths, const string& directory, ostream& output);
string encodeURL(const string& input);
void shuffleVector(vector<pair<string, int>>& vec);

// Main

int main() {

    // Testing variables.

    // Init variables that get changed by funcs.
    vector<pair<string, int>> fileLengths;
    vector<string> audioFiles;
    fs::path dirPath = "audio/";

    // Init URL to 'scrape' files from.
    ifstream readURL("url.txt");
    string URL;
    int isfp = 0; // Assuming that it isn't a filepath by default.

    if (getline(readURL, URL)) {
        readURL.close();
        if (URL.substr(0, 8) != "https://") {
            isfp = 1;
            string dirPath = URL;
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
        
        // Prints list of MP3 files found and their lengths for logging.
        cout << endl << "---- Processing files! This may take a moment! -----" << endl << endl;
        for (const auto& file : audioFiles) {
            cout << "Processing: " << file << endl;
            int length = grabLength(dirPath, file);

            string encodedFile = (isfp == 0) ? encodeURL(file) : file;

            if (length == 0) {
                cout << "Warning, unable to get " << file << "'s length in seconds." << endl << "This will cause errors in URL playlists." << endl;
            }

            cout << "Length in seconds: " << length << endl; // Logging.
            fileLengths.push_back(make_pair(encodedFile, length));
            
        }

        cout << endl << "Done!" << endl << endl;
        
        // Shuffles the final vector.
        if (shuffle) {
            shuffleVector(fileLengths);
        }

    } else {
        cerr << "Program assumes folder 'audio' exists in the same directory." << endl;
        cerr << "Invalid directory or filepath doesn't exist." << endl;
        return -1;
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

    cout << endl << " [ Hit any key to exit... ]" << endl << endl;
    getchar();

    return 0;
}

// Function code below here.

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

int grabLength(const string& filepath, const string& filename) {
    int lengthInSeconds = 0;
    string filen = filepath + filename + ".mp3";

#ifdef _WIN32 // Windows-specific code.
    // This is where the fun begins.
    //Open parameters for MCI_OPEN command and Status paramaters.
    MCI_OPEN_PARMS mciOpenParms;
    MCI_STATUS_PARMS mciStatusParms;


    //Set the device type to mpegvideo and the filename.
    mciOpenParms.lpstrDeviceType = "mpegvideo";
    mciOpenParms.lpstrElementName = filen.c_str();

    // Open file for reading.
    if (mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD_PTR)&mciOpenParms) == 0) {
        DWORD dwLength = mciStatusParms.dwReturn = 0; // Variable to store the length of the audio file
        mciStatusParms.dwItem = MCI_STATUS_LENGTH; // Set the item to retrieve the length of the audio

        if (mciSendCommand(mciOpenParms.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&mciStatusParms) == 0) {
            dwLength = mciStatusParms.dwReturn; // Retrieve length.
        }

        // Close file.
        mciSendCommand(mciOpenParms.wDeviceID, MCI_CLOSE, 0, NULL);

        lengthInSeconds = dwLength / 1000; // Convert milliseconds to seconds
    } else {
        cerr << "Error opening file: " << filen << endl;
        return -1;
    }

#else // Linux-specific code.
    SF_INFO sfinfo;
    SNDFILE* sndfile = sf_open(filen.c_str(), SFM_READ, &sfinfo);

    if (!sndfile) {
        cerr << "Error opening file: " << filen << endl;
        return -1;
    }
    
    lengthInSeconds = static_cast<int>(sfinfo.frames / static_cast<double>(sfinfo.samplerate));
    // Divide frames by sample rate to get seconds of length.
    // Cast to integer value.

    sf_close(sndfile);
#endif

    return lengthInSeconds;

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
