# linux-sound-server
require libasound2, libasound2-dev


compilation: g++ server.cpp -o server -lasound -pthread -Ofast
