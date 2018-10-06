# Welcome to NamedPipe application!

 

This application is a c++ sample application, where you can see a named pipe communication, between two process(server and client).

 

## Environment and tools:

**Development environment**: windows 10

**Development Tools**: Visual Studio 2017 .

**Development Language**: C++

**Library**: Boost

 

# Project

## Goal

The main goal for this solution is share regular data (both string and number information) and a custom Object between client and server.

                When the client send a student information, the server receives this information, and stores it in memory.

                The client can request the full list of students stored on server.

 

## Structure

In this solution there are 3 projects:

 

**NamedPipeClient**: Will perform async/sync calls to named pipe server. In ever call a wrap message will be send to server and the given response will be processed.

 

**Domain**: a static library dll that contains common classes, both used for client and server projects.

 

**NamedPipeServer**: A single thread named pipe server, that receives both async/sync requests from named pipe client.

 

## Main elements

**Student**: A custom object, used to store the student name and the student code.

 

**WrapMessage**: This is a class used in this solution to share information between client and server.

 

**WrapMessage Content**:

 

* **List of Students**: a stl container, used to store student objects.

* **List of data**: a stl container to store string data information.

* **Command info**: a enum list of commands to be perform on client and server.

 

## Client Menu

To choose a option, write on terminal a number that represents an application option, see below

                1 - to send async student object.

                2 - to send sync student object.

                3 - to send async data.

                4 - to send sync data.

                5 - get student objects from server async.

                6 - get student objects from server sync.

                7 - get data from server async.

                8 - get data from server sync.

                9 - Quit.

               

## Executing

To start a run this application, you need to start first the server application, the client check que connection to the server before send

                a message.

 

## Basic workflow

Sending a custom object to server:

* Client ask for user to student client name and student code.

* Client stores the student object and requested command.

* Client makes serialization using boost library.

* Client send the serialized wrapmessage to server using named piped.

* Server receives the serialized wrapmessage.

* Server deserialize messaage and stores the student info in memory.

* Server send a message to client, signalizing the receive the request.

* Client receives the message from server.
