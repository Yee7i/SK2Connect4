# SK2Connect4

This program handles multiple client connections and runs them through the server, allowing multiple instances of connect 4 games.
It accomplishes that by using socket connections and multithreading in C++.

To open the program:
1. Compile and run the server. The compiler should be running under Linux in order to have the required libraries. Server runs on port 1101 by default.
2. Compile and run the client. Parameters at startup: IP address and port. Example: ./Program.exe 127.0.0.1 1101
3. Once two clients connect, a game will start.

How to play:
The goal of the game is to connect four of your signs in a row vertically, horizontally or diagonally. Players can place one sign at a time.

Capabilities:
Running multiple games concurrently.
Allows up to 1000 connections (extendable).
Closing games simultaneously.
