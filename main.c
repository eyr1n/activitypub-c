#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

char req[1024];
char res[1024];
char method[1024];
char path[1024];

int main(void) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    return 1;
  }
  struct sockaddr_in addr = {
      .sin_family = AF_INET,
      .sin_port = htons(10000),
      .sin_addr =
          {
              .s_addr = INADDR_ANY,
          },
  };
  if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    close(sock);
    return 1;
  }
  if (listen(sock, 5) == -1) {
    close(sock);
    return 1;
  }

  while (1) {
    int newsock = accept(sock, NULL, NULL);
    if (newsock == -1) {
      close(sock);
      return 1;
    }

    while (1) {
      if (recv(newsock, req, sizeof(req), 0) == 0) {
        break;
      }
      sscanf(req, "%1023s %1023s", method, path);

      if (strcmp(method, "GET") == 0) {
        if (strcmp(path, "/.well-known/webfinger") == 0) {
          char body[] = "{"
                        "\"subject\":\"acct:eyrin@activitypub-c.eyrin.jp\","
                        "\"links\":["
                        "{"
                        "\"rel\":\"self\","
                        "\"type\":\"application/activity+json\","
                        "\"href\":\"https://activitypub-c.eyrin.jp\""
                        "}"
                        "]"
                        "}\r\n";
          sprintf(res,
                  "HTTP/1.1 200 OK\r\nContent-Type: "
                  "application/json\r\nContent-Length: %zu\r\n\r\n%s",
                  strlen(body), body);
          send(newsock, res, strlen(res), 0);
        } else if (strcmp(path, "/") == 0) {
          char body[] = "\r\n{"
                        "\"@context\":["
                        "\"https://www.w3.org/ns/activitystreams\","
                        "\"https://w3id.org/security/v1\""
                        "],"
                        "\"id\":\"https://activitypub-c.eyrin.jp\","
                        "\"type\":\"Person\","
                        "\"preferredUsername\":\"eyrin\","
                        "\"name\":\"りｎ\","
                        "\"summary\":\"C言語でActivityPubしてみる\","
                        "\"inbox\":\"https://activitypub-c.eyrin.jp/inbox\""
                        "}\r\n";
          sprintf(res,
                  "HTTP/1.1 200 OK\r\nContent-Type: "
                  "application/activity+json\r\nContent-Length: %zu\r\n\r\n%s",
                  strlen(body), body);
          send(newsock, res, strlen(res), 0);
        } else {
          sprintf(res, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
          send(newsock, res, strlen(res), 0);
        }
      } else {
        sprintf(res,
                "HTTP/1.1 501 Not Implemented\r\nContent-Length: 0\r\n\r\n");
        send(newsock, res, strlen(res), 0);
      }
    }
    close(newsock);
  }

  close(sock);
  return 0;
}
