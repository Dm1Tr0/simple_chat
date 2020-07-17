# Simple chat room

## Description
'Simple chat room' is nothing but a simple server that is capable of sendindg client - client and client - all clients messages.The client is also available

## Compilation
to compile the project run make in main folder of the project.
If you want to compile programs with no loging do:
```
make silent
```
## Comands
```
quit                  // exit chatroom
s_msg|who|text        //send a message
s_msg_all|text        //send a message to all users
list_users            //to see who is currently online
help                  //help
in_msg                //to see who  currently online
nick|new nickname     // to change a nickname          
```

## Usage
In order to connect to the server using the client just enter the IP address of a host the server works on.
if you want to run client on the same host as server then just simply do :
```
client 127.0.0.1
```

###Note!!
Be shure to enter a comand without spaces example
(\n is an Enter keystroke)
```
input : /s_msg|nick|blah blah blah\n 
input : /in_msg\n
input : /s_msg_all|hi people\n
```
otherwise nobody will get your messages.
Also don't forget that the only way you can read an input message is to use:
```
/in_msg\n
```
command.
## License
[MIT](https://choosealicense.com/licenses/mit/)
