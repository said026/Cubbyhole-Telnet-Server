# Cubbyhole Telnet Server

We realize the cubbyhole protocol as simple, TCP based text protocol. Each command consists of a
single word (casing does not matter) that might be followed by a space and an arbitrary text and is
terminated with a newline. The following commands should be supported:

- `PUT <message>` Places a new message in the cubbyhole.
- `GET` Takes the message out of the cubbyhole and displays it.
- `LOOK` Displays message without taking it out of the cubbyhole.
- `DROP` Takes the message out of the cubbyhole without displaying it.
- `HELP` Displays some help message.
- `QUIT` Terminates the connection.

The server greets any new client with !HELLO: <text>. After that, the server answers each command
with !<command>: ok or !<command>: <text>. e.g. the command pUt hello world is answered
with !PUT: ok and get is answered with !GET: hello world.

## Install

Just run the makefile :)
```
make
```

## Usage

Run the server via the following command (default port is 1337) :

```
./cubbyhole <port>
Server started, listenting for clients on port 1337...
```

To connect to the server run :

```
telnet localhost 1337
```

In the example above, we used the 1337 port. Here is an example of commands you can execute on the client :

