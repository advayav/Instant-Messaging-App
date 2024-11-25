# Instant-Messaging-App
The project works with three users referred to as 0, 1, and 2 in the code and they work to send messages to the server and eventually to other users.

Main method:
    User processes are created by forking the parent processes
    Write pipe for the server to the user and read pipe for the user to the server are closed for the users
    Pipes connecting the users to each other are also closed
    User handler is called for each other so that the signals relaying can be done for them
    Read pipe for the server to the user and write pipe for the user to the server is closed for the parent process(server)
    Process IDs are stored for all the users so they can be used later on while signal relaying is taking place
    The server handler is called only once since there is only one server

User handler:
    Signal handler is created where a counter increments with every new message so that messages from the server to the user can be processed
    The function takes in the user ID, the write pipe for the user to the server, and the server-to-user array
    Signals are set for server-to-user transmission
        Only one setup since the user handler is called for each user
    The user sends the messages to the server based on user IDs
    Closes the write pipes for that user once all the messages have been sent and sends the signal to the server that the pipe is closed
    The user then processes the messages that the server is sending to it
	      The user checks if there are values to be read and closes the read end if there are none
	      String is parsed so the user can identify who the message is from and rearrange the message so that the correct one can be printed out
	      The received signal is printed out
    The loop runs till the write end of the pipe has been closed for that user


Server handler:
    Signal handlers are created for all three users where a counter increments every time there is a new message from that user
    The function takes in the user-to-server array, server-to-user array, and the array of process IDs for the users(children of the forked process)
    A counter variable is declared for each user so that we can keep track of when the write pipe is closed for that user
    The message from the user is parsed so that the server can know who the message is coming from
    For every user the server checks if there is a message
	      It closes the write pipe in the server to the user for that user if the user is done sending messages to the server and sends a signal to the user that the write pipe has been closed
	      If the write pipe is still open, the server writes the message and sends a signal to the user that the message is supposed to go to that there is an incoming message
    After all the messages have been processed, the server closes all the read pipes in the user-to-server array
    The write pipes in the server-to-user array are also closed and a signal is sent to each user that the pipe has been closed
