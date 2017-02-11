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

The server greets any new client with `!HELLO: <text>`. After that, the server answers each command
with `!<command>:` ok or `!<command>: <text>`. e.g. the command pUt hello world is answered
with `!PUT: ok` and get is answered with `!GET: hello world`.

## Install

Just run the makefile :)
```
$ make
```

## Usage

Run the server via the following command (default port is 1337) :

```
$./cubbyhole <port>
Server started, listenting for clients on port 1337...
```

To connect to the server run :

```
$ telnet localhost 1337
```

In the example above, we used the 1337 port. Here is an example of commands you can execute on the client :

```
$ telnet localhost 1337
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
!hello: welcome brave adventurer
help
!help: A cubby hole is a small hiding place where one can hide things.
!help: We now define the cubby hole protocol that allows users to store
!help: one line messages on a server.
!help: As the hole is really small, the server will only store one
!help: message at a time, but keeps and shares it across different
!help: connections. If a new message is put in the cubby hole, the
!help: old message is lost.
!help:
!help: The following commands should be supported:
!help:
!help: PUT <msg> Places a new message in the cubby hole.
!help: GET       Takes the message out of the cubby hole and displays it.
!help: LOOK      Displays message without taking it out of the cubby hole.
!help: DROP      Takes the message out of the cubby hole without displaying it.
!help: HELP      Displays some help message.
!help: QUIT      Terminates the connection.
!help:
!help: Have Fun and play around!
put Hello World !
!PUT: ok
get
!GET: Hello World !
quit
!goodbye: see you next time
Connection closed by foreign host.
```
## Licence

[![CC0](https://licensebuttons.net/p/zero/1.0/88x31.png)](https://creativecommons.org/publicdomain/zero/1.0/)

Part of the Network Protocols and Architectures course at TU Berlin  
http://www.inet.tu-berlin.de/menue/teaching0/ws201617/npa_1617/parameter/en/
