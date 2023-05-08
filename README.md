# Network-Programming---Messaging-App
Demonstrates foundational file transfer principles such as packet segmentation and ACK mechanism.
Uses Client-Server techniques for communication. The communication process is done through specific Terminal commands on a Linux Based platform.

Official documentation can be found in Beej's Guide to Network Programming

File Transfer uses UNIX sockets to implement simple client and server
programs which interact with each other to accomplish a file transfer in a connectionless manner. Unlike simply receiving a message and sending it back, the messages use a specific packet format and implement acknowledge ACK for the simple file transfer using UDP sockets. The Server waits for RTT time which is constantly updated depending on the standard deviation of previous RTTs to compute resonable wait times that ensure optimal file transfer wait times and performance.

The Messaging App also functions on the Linux Terminal using specific commands mentioned in the C files. Each user is able to share text messages and media files and maintain their specific accounts. The App permits users to form groups, send private messages to other users and be part of multiple group sessions at the same time. Additionally there is an Admin feature that allows an authoritative user to kick out participants out of a session. 

