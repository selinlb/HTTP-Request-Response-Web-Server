# HTTP-Request-Response-Web-Server
A very simple HTTP request/response web server project written in C.

it's my first time trying system programming in c so feel free to point mistakes on my code. 

## how to use

**to compile the code:**
```
gcc server.c -o server
./server -portnum- (as pointed in usage)
```
  
  **then open your web browser:**
  ``localhost:-portnum-/-some file name(must be existed)-``
  
  **or you can simply just use telnet:**
  ``telnet localhost -portnum-``
  
  note that this code only response to GET request, so you have to use GET request for a response.
  
  ``GET -existed file name .type- HTTP/1.1\r\n``
