/* 
 * File:   main.cpp
 * Author: gouldkj
 *
 * Copyright (C) 2020 gouldkj@miamioh.edu
 */

#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <vector>
#include "ChildProcess.h"

using namespace std;
using StrVec = std::vector<std::string>;

/**
 * Method that receives the address of the argument list and forks and executes 
 * the commands given by the user. Uses the ChildProcess class to fork and 
 * execute the command.
 * 
 * @param argList The address of the vector containing the arguments to be 
 * executed by the method.
 * 
 * @return This method returns the child process that ran the given commands.
 */
ChildProcess runCommands(const StrVec& argList) {
    // Prints the commands being run.
    std::cout << "Running:";
    for (auto& a : argList) {
        std::cout << " " << a;
    }
    // Ends the current line of output once all commands are printed.
    std::cout << std::endl;
    
    // Create a child process to run the command.
    ChildProcess child;
    // Calls the forkNexec method from ChildProcess.cpp with the argList as the 
    // parameter.
    child.forkNexec(argList);
    // Returns the exit code of the child process.
    return child;
}

/**
 * Takes a line of input and separates the line into an array of words before
 * returning that array.
 * 
 * @param line A pointer to the string of input received from the main method.
 *
 * @return This method returns the vector containing the separated
 */
StrVec splitString(const std::string& line) {
    // The vector that will hold the separated words.
    StrVec argList;
    // String that will be used to retrieve each word from the string.
    std::string word;
    // Creates the input string stream from the line parameter.
    std::istringstream is(line);
    
    // Retrieves each space separated word from the string stream using 
    // std::quoted to handle any words that might be inside quotations. Pushes
    // each word onto the argList vector.
    while (is >> std::quoted(word)) {
        argList.push_back(word);
    }
    
    // Returns the vector of separated commands.
    return argList;
}

/**
 * @param argList
 * 
 * @return Method does not have a return statement but does print out the exit 
 * code for every child process that executes a command.
 */
void processCommands(const StrVec& argList) {
    // Creates a vector that contains all of the child processes created when 
    // the program runs serially or in parallel.
    std::vector<ChildProcess> childList;
    // Creates the input file stream for the text file given in the arguments.
    std::ifstream in(argList[1]);
    
    // Retrieves commands from the script file through the in file stream.
    std::string line;
    while (std::cout << "", std::getline(in, line) && (line != "exit")) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        const StrVec cmdList = splitString(line);
        
        // Creates child process instance that will run the commands.
        ChildProcess child = runCommands(cmdList);
    
        // Determines if the program is running in serial or parallel by 
        // checking the first value stored in argList.
        if (argList[0] == "SERIAL") {
            std::cout << "Exit code: " << child.wait() << std::endl;
        } else {
            childList.push_back(child);
        }
    }
    // After all the child processes have been created, this code will print
    // out the exit code for each process in the order they were created.
    for (auto& a : childList) {
        std::cout << "Exit code: " << a.wait() << std::endl;
    }
}

/**
 * Helper method to break down a URL into hostname, port and path. For
 * example, given the url: "https://localhost:8080/~raodm/one.txt"
 * this method returns <"localhost", "8080", "/~raodm/one.txt">
 *
 * Similarly, given the url: "ftp://ftp.files.miamioh.edu/index.html"
 * this method returns <"ftp.files.miamioh.edu", "80", "/index.html">
 *
 * @param url A string containing a valid URL. The port number in URL
 * is always optional.  The default port number is assumed to be 80.
 *
 * @return This method returns a std::tuple with 3 strings. The 3
 * strings are in the order: hostname, port, and path.  Here we use
 * std::tuple because a method can return only 1 value.  The
 * std::tuple is a convenient class to encapsulate multiple return
 * values into a single return value.
 */
std::tuple<std::string, std::string, std::string>
breakDownURL(const std::string& url) {
    // The values to be returned.
    std::string hostName, port = "80", path = "/";
    
    // Extract the substrings from the given url into the above
    // variables.  This is very simple 174-level logic problem.

    // First find index positions of sentinel substrings to ease
    // substring operations.
    const size_t hostStart = url.find("//") + 2;
    const size_t pathStart = url.find('/', hostStart);
    const size_t portPos   = url.find(':', hostStart);
    // Compute were the hostname ends. If we have a colon, it ends
    // with a ":". Otherwise the hostname ends with the last
    const size_t hostEnd   = (portPos == std::string::npos ? pathStart :
                              portPos);
    // Now extract the hostName and path and port substrings
    hostName = url.substr(hostStart, hostEnd - hostStart);
    path     = url.substr(pathStart);
    if (portPos != std::string::npos) {
        port = url.substr(portPos + 1, pathStart - portPos - 1);
    }
    // Return 3-values encapsulated into 1-tuple.
    return {hostName, port, path};
}

/**
 * This method extracts commands from the downloaded file retrieved from the 
 * given url. The commands can then be executed in either serial or parallel
 * fashion.
 * 
 * @param sp String that tells the method whether or not to run in parallel
 * or serial.
 * 
 * @param is Input stream for the downloaded file so the method can read from
 * the file.
 * 
 * @param os Output stream
 * 
 * @return This method has no return statement, though it does print the exit
 * code for every child process to the terminal.
 * 
 */
void processData(const std::string sp, std::istream& is, std::ostream& os) {
    // Creates a vector to store all the child processes used to execute 
    // commands.
    std::vector<ChildProcess> childList;
    // For loop used to read and ignore HTTP headers.
    for (std::string hdr; std::getline(is, hdr) && !hdr.empty() && 
            hdr != "\r";) {}
    
    // While loop reads line by line from the file and only ends when the end
    // of the stream is reached or when the command exit is encountered.
    std::string line;
    while (std::cout << "", std::getline(is, line) && (line != "exit")) {
        if (line.empty() || (line[0] == '#')) {
            continue;
        }
        
        // Creates a constant vector to store all the commands on the line of
        // input.
        const StrVec cmdList = splitString(line);
        
        // Creates child process instance that will run the commands.
        ChildProcess child = runCommands(cmdList);
    
        // Determines if the program is running in serial or parallel by 
        // checking the string sp passed to the method.
        if (sp == "SERIAL") {
            // If the program is running in serial, then it waits for each
            // child process to finish.
            std::cout << "Exit code: " << child.wait() << std::endl;
        } else {
            // If the program is not running in serial, then it must be running
            // in parallel. The program does not wait for each process and 
            // instead pushes them onto the childList vector.
            childList.push_back(child);
        }
    }
    // After all the child processes have been created, this code will print
    // out the exit code for each process in the order they were created.
    for (auto& a : childList) {
        std::cout << "Exit code: " << a.wait() << std::endl;
    }
}

/**
 * This method takes the list of arguments and extracts the url and whether or
 * not the program should run serial or parallel. The method also produces the 
 * HTTP request that retrieves the specific file from the given url and passes
 * the file to the processData method for processing.
 * 
 * @param argList Vector of arguments passed by the main method. Used to 
 * determine the url and whether the program should run in parallel or serial.
 * 
 * @param is Input stream for the iostream that connects to the file downloaded
 * from the given url.
 * 
 * @param os Output stream to be passed to the processData method.
 * 
 * @return This method has no return statement, it only passes data onto the 
 * processData method.
 * 
 */
void processURL(const StrVec& argList, std::istream& is = std::cin, 
        std::ostream& os = std::cout) {
    // Place SERIAL or PARALLEL in its own string.
    std::string sp = argList[0];
    // Place url into its own string.
    auto url = argList[1];
    
    // Extract the hostname, port number and path from the url.
    std::string host, port, path;
    std::tie(host, port, path) = breakDownURL(url);
    
    // Download file from the given url.
    boost::asio::ip::tcp::iostream data(host, port);
    data << "GET " << path << " HTTP/1.1\r\n"
            << "Host: " << host << "\r\n"
            << "Connection: Close\r\n\r\n";
    
    // Passes downloaded file, sp string and output stream to the processData
    // method for executing the commands from the file.
    processData(sp, data, os);
}

/**
 * Takes user inputs from the console and creates a child process object to 
 * run the commands.
 * 
 * @param This method takes user input through std::cin as input.
 * 
 * @return This method has no return, though it will print out the exit codes 
 * for all the child processes that execute commands.
 */
int main() {
    // String to extract each line of input from the user.
    std::string line;
    
    // This loop will continue to receive and interpret input until the end of 
    // the stream or until the command 'exit' is encountered.
    while (std::cout << "> ", std::getline(cin, line) && (line != "exit")) {
        if (line.empty() || line[0] == '#') {
            // Skips this iteration of the loop if the if statement 
            // is satisfied.
            continue; 
        }
        
        // Calls the splitString method to separate the input line into an
        // array of strings.
        const StrVec argList = splitString(line);
        
        // Performs either serial and parallel operations or runs a basic 
        // command.
        std::size_t found = argList[1].find("http");
        if (found != std::string::npos) {
            processURL(argList);
        }
        if (argList[0] == "SERIAL" || argList[0] == "PARALLEL") {
            processCommands(argList);
        } else {
        // Create the child process that will run the command.
        ChildProcess child = runCommands(argList);
        
        // Prints out the exit code for the child process.
        std::cout << "Exit code: " << child.wait() << std::endl;
        }
    }
}

