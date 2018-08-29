#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#define CONST_STR_LEN(x) x, x ? sizeof(x) - 1: 0

int sendRespondDataToClient(int sockfd, const char *data, const int dataLen)
{
  char contenLenStr[1024] = "";
  snprintf(contenLenStr, sizeof(contenLenStr), "Content-Length: %d\r\n", dataLen);

  write(sockfd, CONST_STR_LEN("HTTP/1.1 200 OK\r\n"));
  write(sockfd, CONST_STR_LEN("Content-Type: text/html\r\n"));
  write(sockfd, CONST_STR_LEN("Connection: close\r\n"));
  write(sockfd, contenLenStr, strlen(contenLenStr));
  write(sockfd, CONST_STR_LEN("Server: tinyhttp\r\n"));
  write(sockfd, CONST_STR_LEN("\r\n")); //head end
  write(sockfd, data, dataLen);
  return 0;
}
int main(void)
{
  struct addrinfo hints, *res = NULL, *saveres = NULL;
  int ret = -1;
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET6;
  hints.ai_flags = AI_PASSIVE;
  char ipaddr[64] = "";
  int listenfd = -1;
  int clifd = -1;
  union{
    struct sockaddr_in6 ipv6;
    struct sockaddr_in ipv4;
    struct sockaddr plain;
  }cliaddr;
  socklen_t cliaddrLen = sizeof(cliaddr);
  char readBuf[1024] = "";
  int readSize = -1;

  
  if((ret = getaddrinfo(NULL, "http", &hints, &res)) != 0)
  {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(ret));
    goto ERROR;
  }
  if((listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
  {
    fprintf(stderr, "socket error: %s\n", strerror(errno));
    goto ERROR;
  }
  if(bind(listenfd, res->ai_addr, res->ai_addrlen) < 0)
  {
    fprintf(stderr, "bind error: %s\n", strerror(errno));
    goto ERROR;
  }
  if(listen(listenfd, 5) < 0)
  {
    fprintf(stderr, "listen error: %s\n", strerror(errno));
    goto ERROR;
  }
  switch(res->ai_family)
  {
    case AF_INET:
      fprintf(stderr, "Listen %s:%d\n", inet_ntop(res->ai_family, &((struct sockaddr_in *)res->ai_addr)->sin_addr, ipaddr, sizeof(ipaddr)), ntohs(((struct sockaddr_in *)res->ai_addr)->sin_port));
      break;
    case AF_INET6:
      fprintf(stderr, "Listen %s:%d\n", inet_ntop(res->ai_family, &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr, ipaddr, sizeof(ipaddr)), ntohs(((struct sockaddr_in6 *)res->ai_addr)->sin6_port));
      break;
  }
  while((clifd = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddrLen)) >= 0)
  {
    char cliStr[1024] = "";
    if((readSize = read(clifd, readBuf, sizeof(readBuf))) <= 0)
    {
      fprintf(stderr, "readSize = %d: %s\n", readSize, strerror(errno));
      close(clifd);
      continue;
    }
    else
    {
      write(1, readBuf, readSize);
    }
    switch(cliaddr.plain.sa_family)
    {
      case AF_INET:
        if(inet_ntop(cliaddr.plain.sa_family, (void *)&cliaddr.ipv4.sin_addr, cliStr, sizeof(cliStr)) == NULL)
        {
          fprintf(stderr, "%s %d, inet_ntop error: %s\n", __func__, __LINE__, strerror(errno));
        }
        break;
      case AF_INET6:
        if(inet_ntop(cliaddr.plain.sa_family, (void *)&cliaddr.ipv6.sin6_addr, cliStr, sizeof(cliStr)) == NULL)
        {
          fprintf(stderr, "%s %d, inet_ntop error: %s\n", __func__, __LINE__, strerror(errno));
        }
        break;
    }
    if(cliStr[0] != '\0')
    {
      fprintf(stderr, "New Connect: %s\n", cliStr);
    }
    sendRespondDataToClient(clifd, CONST_STR_LEN("<html><head><meta charset=\"utf-8\"><title>Hello, world</title></head><body><div style=\"color: pink; font-size: 2em;\">Hello, world</div></body></html>"));
    if(shutdown(clifd, SHUT_WR) < 0)
    {
      fprintf(stderr, "shutdown error: %s\n", strerror(errno));
    }
    else if(close(clifd) < 0)
    {
      fprintf(stderr, "%s %d, close error: %s\n", __func__, __LINE__, strerror(errno));
    }
    /*
    if(close(clifd) < 0)
    {
      fprintf(stderr, "%s %d, close error: %s\n", __func__, __LINE__, strerror(errno));
    }
    */
    /*
    else if((readSize = read(clifd, readBuf, sizeof(readBuf))) <= 0)
    {
      fprintf(stderr, "readSize = %d: %s\n", readSize, strerror(errno));
      if(close(clifd) < 0)
      {
        fprintf(stderr, "%s %d, close error: %s\n", __func__, __LINE__, strerror(errno));
      }
    }
    else
    {
      write(1, readBuf, readSize);
    }
    */
  }
  if(res != NULL)
  {
    freeaddrinfo(res);
    res = NULL;
  }
  if(listenfd >= 0)
  {
    close(listenfd);
    listenfd = -1;
  }
  return 0;
ERROR:
  if(res != NULL)
  {
    freeaddrinfo(res);
    res = NULL;
  }
  if(listenfd >= 0)
  {
    close(listenfd);
    listenfd = -1;
  }
  return -1;
}
