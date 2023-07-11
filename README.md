# tcp client server connection

Simple TCP server and client.

## Build

Run the following command:
```sh
make
```
After build, `client` and `server` programs will be in the `bin/` directory.

## Run

Open 2 terminals in `bin/` directory.
In the first terminal run:
```sh
./client 127.0.0.1 1234
```

In the second terminal run:
```sh
./server 1234
```

Hash `#` character closes `client` and `server`.
