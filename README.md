# M3U-Generator

Simple program to generate M3U playlists from a website path or a directory path.
This also features automatic URL encoding (replacing spaces in an audio filename to URL-safe pathing, etc)
Program also shuffles the playlist if the user wills it.


## To use

Place compiled executable (or compile the thing yourself) in any directory and run it through a command line (recommended, probably not required)
if url.txt does not exist, create it and place the URL or filepath that you want to create a playlist of

This program assumes that you already have all of the files required downloaded, and reads from a seperate 'audio/' folder in the same directory as the program to generate information off of.
Copy and place your (MP3) files into the audio/ folder and run the program.

This will result in a .m3u playlist if there are no other errors.

## External libraries required/used

- TagLib

## Build command:

If you want to build the program by source instead of downloading a release, you can use the following command to compile it.

```
g++ -o M3UplaylistGenerator main.cpp -ltag
```
