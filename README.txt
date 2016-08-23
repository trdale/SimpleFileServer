Tom Dale
Prog 1
CS 372

Compile:

g++ -o ftserver ftserver.cpp
javac ftclient.java

Usage:

$ ftserver [port num]
$ java ftclient [hostname] [port num] -g|-l [filename] [dataport]

The server will take a port to sit and listen on, and wait for a client to connect.

The client will provide the hostname, and port to make an initial connection, then their command and either both a filename and data port or just a dataport

The Server will then send the requested information over through the specified dataport.

Max file size is 12096


Ex.

If client wishes to get a directory list

$ftserver 45201

$java ftclient 45201 -l 33026

This will connect the client to the specified port then since a directory list back on port 33026 to the client

If client wishes to get a specified file

$java ftclient 45201 -g file1 33026

This will connect the client to the port then send file1 back to the client. Whenever a file is sent .copy is attached to the end of the file name, since currently the files must run in the same directory to transfer.

KNOWN BUGS:

Cannot handle unknown file names, will cause an error on the client side and no file error message is sent.


Testing:

Tested on OSU flip2 server using [hostname] as flip2.engr.oregonstate.edu with server port 45832 client useing 32502 as data port


Resources:

Code taken from:
Beej's Guide to Network Programming  http://beej.us/guide/bgnet/
http://www.informit.com/guides/content.aspx?g=cplusplus&seqNum=245
http://www.rgagnon.com/javadetails/java-0542.html
http://stackoverflow.com/questions/10112038/parsing-commands-shell-like-in-c