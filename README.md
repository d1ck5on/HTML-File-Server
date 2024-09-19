# HTTP File Server

An HTTP1.1 server that acts as a file store.

## Quick start

Build & Start:
```
docker compose build server
docker compose up -d server
```

cURL examples:
```
$ curl -X POST localhost:8080/hello.txt

$ curl -X PUT localhost:8080/hello.txt -d hello

$ curl -X GET localhost:8080/hello.txt
hello
```

## Parameters
The server takes its parameters from the environment variables:

 - ```SERVER_HOST```: the host on which you want to listen
 - ```SERVER_PORT```: the port on which you want to listen
 - ```SERVER_WORKING_DIRECTORY```: the absolute path to the directory in which the files are to be stored.

 The command line can also pass parameters:
 ```
--host -> server host
--port -> server port
--working-directory -> server working directory
 ```

 The value of the parameter is an environment value if no command line parameter is passed.

 ## Methods

- **GET {url}**. Returns the file at working-directory/{url}. Returns a table of all files in the directory if the path is a directory.
- **POST {url}**. Creates a new file on the server at working-directory/{url}. The file data is passed in the body of the request. If the Create-Directory: True header is passed, the directory will be created.
- **PUT {url}**. Updates the contents of the file at path working-directory/{url}.
- **DELETE {url}**. Removes the file at working-directory/{url}. If the Remove-Directory header is passed, the directory will be removed.

