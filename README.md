# VoiceGames 

A games server implementing a generic gaming protocol designed by BCIT Data Communications(DC) and Internetworking students(Winter 2021)


## Developers:

Karel Chanivecky

Gurden Singh

## Tech stack

### Server: 
* C 
* POSIX
* BCIT Datacomm C data structure library

Implements a multi-threaded server with a lobby and serial TCP connection handling using select(3).
Additionally, it provides voice chat using UDP and a voice communication protocol also designed by BCIT DC students.

### Client

* Java
* Android API 27+
* AudioTrack
* AudioRecord

Android interface to support any game based on the base protocol designed by the BCIT DC students. It provides an API for
all kinds of turn based games. Additionally, it enables real-time audio communication among the players using a UDP
protocol. 
