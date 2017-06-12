# MFTP (My FTP Server)

A basic implementation of a server/client FTP server.

The File Transfer Protocol (FTP) is a standard network protocol used for the transfer of computer files between a client and server on a computer network. MFTP is built on a client-server model architecture and uses separate control and data connections between the client and the server.

## Description

**Server:**

Server is ran in a separate terminal window than the client. Server can handle multiple connections from clients at a single time. Each time a request is made to server, a new data socket is created on a new open port to facilitate transfers of data.

Server will run indefinitely until forced to shut down through `control-C`. Also Server will not accept input, only echo received commands from clients. If command is accepted server will reply to client with an `A`.

Accepted input:

* `Ctrl-C` (Exit Server)

**Client:**

Client is ran in a separate terminal window then server. Client can either connect to localhost or you can specify a port number when starting client. From client you can view your local directory, change local directory, get a file from server, put a file onto the server, and show a file from server.

Accepted input:

* `ls` (local ls, client side show directory)
* `rls` (remote ls, server side show directory)
* `cd` (local cd, client side change directory)
* `rcd` (remote cd, server side change directory)
* `get <name>` (get file from server)
* `put <name>` (put file on server)
* `show <name>` (echo file from server in client)
* `exit` (Log off client off server)

## Archive

```txt
myFTP
├── Client                  : Holds client code
│   ├── Makefile            : Build client
│   ├── clientTest.txt      : test file unique to client
│   ├── mftp.c              : Source code for client
│   └── mftp.h              : Header
└── Server                  : Holds server code
    ├── Makefile            : Builds server
    ├── mftp.h              : Header
    ├── mftpserve.c         : Source code for server
    └── serverTest.txt      : test file unique to server
```

## Building and Installing

* Clone repo and open server and client directory in      separate terminal windows.
* To build the server enter on server window:

```bash
make server
````

* To build client enter on client window:

```bash
make client
```

* To remove enter on either window:

```bash
make clean
```

## Running mftp (Client)

* To start client enter:

```bash
./client localhost
OR
./client <port number>
```

## Running mftp (Server)

* To start server enter:

```bash
./server
```

Until a client connects, server will just wait.

## Usage

* When successfully connected, client will be presented with

```bash
** Connection Established **
MFTP>
```

You can then enter any of the commands specified above.

**Example input:**

Show the local files in client directory.

```txt
MFTP> ls
total 80
-rwxr-xr-x@ 1 spencerkitchen  staff    225 Jun 11 17:02 Makefile
-rwxr-xr-x  1 spencerkitchen  staff  15140 Jun 11 17:24 client
drwxr-xr-x  3 spencerkitchen  staff    102 Jun 11 17:24 client.dSYM
-rw-r--r--@ 1 spencerkitchen  staff     86 Jun 11 17:22 clientTest.txt
-rw-r--r--@ 1 spencerkitchen  staff  12266 May  6  2016 mftp.c
-rw-r--r--@ 1 spencerkitchen  staff    584 May  6  2016 mftp.h
```

Show remote files in server directory

```txt
MFTP> rls
A
total 80
-rwxr-xr-x@ 1 spencerkitchen  staff    224 Jun 11 17:03 Makefile
-rw-r--r--@ 1 spencerkitchen  staff    584 May  6  2016 mftp.h
-rw-r--r--@ 1 spencerkitchen  staff  12230 May  6  2016 mftpserve.c
-rwxr-xr-x  1 spencerkitchen  staff  14896 Jun 11 17:24 server
drwxr-xr-x  3 spencerkitchen  staff    102 Jun 11 17:24 server.dSYM
-rw-r--r--@ 1 spencerkitchen  staff     86 Jun 11 17:23 serverTest.txt
```

Get file from server

```txt
MFTP> get serverTest.txt
A
MFTP> ls
total 88
-rwxr-xr-x@ 1 spencerkitchen  staff    225 Jun 11 17:02 Makefile
-rwxr-xr-x  1 spencerkitchen  staff  15140 Jun 11 17:24 client
drwxr-xr-x  3 spencerkitchen  staff    102 Jun 11 17:24 client.dSYM
-rw-r--r--@ 1 spencerkitchen  staff     86 Jun 11 17:22 clientTest.txt
-rw-r--r--@ 1 spencerkitchen  staff  12266 May  6  2016 mftp.c
-rw-r--r--@ 1 spencerkitchen  staff    584 May  6  2016 mftp.h
-r-xr--r-x  1 spencerkitchen  staff     86 Jun 11 17:28 serverTest.txt
```

Put file onto server from client

```txt
MFTP> put clientTest.txt
A
MFTP> rls
A
total 88
-rwxr-xr-x@ 1 spencerkitchen  staff    224 Jun 11 17:03 Makefile
-r-xr--r-x  1 spencerkitchen  staff     86 Jun 11 17:29 clientTest.txt
-rw-r--r--@ 1 spencerkitchen  staff    584 May  6  2016 mftp.h
-rw-r--r--@ 1 spencerkitchen  staff  12230 May  6  2016 mftpserve.c
-rwxr-xr-x  1 spencerkitchen  staff  14896 Jun 11 17:24 server
drwxr-xr-x  3 spencerkitchen  staff    102 Jun 11 17:24 server.dSYM
-rw-r--r--@ 1 spencerkitchen  staff     86 Jun 11 17:23 serverTest.txt
```

Show file from server on client

```txt
MFTP> show serverTest.txt
A
This file originated on the server side, The Client DOES NOT have a copy of this file.
```

## Built With

* C99
* GNU Make

## Author

* **Spencer Kitchen**
* Email: <sckitchen.dev@gmail.com>