

#include "packet-format.h"
#include <limits.h>
#include <netdb.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  char *file_name = argv[1];
  char *host = argv[2];
  int port = atoi(argv[3]);
  int window = atoi(argv[4]);

  FILE *file = fopen(file_name, "r");
  if (!file) {
    perror("fopen");
    exit(-1);
  }

  // Prepare server host address.
  struct hostent *he;
  if (!(he = gethostbyname(host))) {
    perror("gethostbyname");
    exit(-1);
  }

  struct sockaddr_in srv_addr = {
      .sin_family = AF_INET,
      .sin_port = htons(port),
      .sin_addr = *((struct in_addr *)he->h_addr),
  };

  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd == -1) {
    perror("socket"); 
    exit(-1);
  }

  int ack_rec = 1;
  uint32_t number = 1;
  ack_pkt_t ack_pkt;
  uint32_t seq_num = 0;
  uint32_t seq_ack = 1;
  data_pkt_t data_pkt;
  size_t data_len;
  struct timeval tv;
  int cont = 0;

  tv.tv_sec = 1;
  tv.tv_usec = 0;
  

  if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0){
    cont++;
  }


  if(window == 1){
    do { // Generate segments from file, until the the end of the file.
      // Prepare data segment.
      data_pkt.seq_num = htonl(seq_num++);

      // Load data from file.
      data_len = fread(data_pkt.data, 1, sizeof(data_pkt.data), file);

      
      // Send segment.
      if(ack_rec== 1){
        ack_pkt.seq_num = htonl(seq_ack);
        ssize_t sent_len = sendto(sockfd, &data_pkt, offsetof(data_pkt_t, data) + data_len, 0,
                  (struct sockaddr *)&srv_addr, sizeof(srv_addr));
        printf("Sending segment %d, size %ld.\n", ntohl(data_pkt.seq_num),offsetof(data_pkt_t, data) + data_len);
        if (sent_len != offsetof(data_pkt_t, data) + data_len) {
          fprintf(stderr, "Truncated packet.\n");
          exit(-1);
        }
      }
    
      ssize_t len = recvfrom(sockfd, &ack_pkt, sizeof(ack_pkt), 0,
                  (struct sockaddr *)&srv_addr, &(socklen_t){sizeof(srv_addr)});
      if(len >= 0  && ack_pkt.seq_num == data_pkt.seq_num + htonl(number)){
        printf("Received ACK %d / %d.\n" , ntohl(ack_pkt.seq_num),ntohl(ack_pkt.selective_acks));
        ack_rec = 1;
        cont = 0;
      }else{   
        
        ack_pkt.seq_num = htonl(seq_ack);
        ssize_t sent_len = sendto(sockfd, &data_pkt, offsetof(data_pkt_t, data) + data_len, 0,
                  (struct sockaddr *)&srv_addr, sizeof(srv_addr));
        printf("Sending segment %d, size %ld.\n", ntohl(data_pkt.seq_num),offsetof(data_pkt_t, data) + data_len);
        if (sent_len != offsetof(data_pkt_t, data) + data_len) {
          fprintf(stderr, "Truncated packet.\n");
          exit(-1);
        }
        ssize_t len = recvfrom(sockfd, &ack_pkt, sizeof(ack_pkt), 0,
                  (struct sockaddr *)&srv_addr, &(socklen_t){sizeof(srv_addr)});
        if(len >= 0  && ack_pkt.seq_num == data_pkt.seq_num + htonl(number)){
          printf("Received ACK %d / %d.\n" , ntohl(ack_pkt.seq_num),ntohl(ack_pkt.selective_acks));
          ack_rec = 1;
          cont = 0;
        }
        if (cont == 3)
        {
          exit(EXIT_FAILURE);
        }
        
      }
      seq_ack++;
    } while (!(feof(file) && data_len < sizeof(data_pkt.data)));
  }
  
  int base = 0;
  int seqN = 1;

  if (window != 1){
    do{
      
      while ((seqN - base) <= window){
        data_pkt.seq_num = htonl(seq_num);

        // Load data from file.
        data_len = fread(data_pkt.data, 1, sizeof(data_pkt.data), file);
        ack_pkt.seq_num = htonl(seq_ack);

        ssize_t sent_len = sendto(sockfd, &data_pkt, offsetof(data_pkt_t, data) + data_len, 0,
                  (struct sockaddr *)&srv_addr, sizeof(srv_addr));
        printf("Sending segment %d, size %ld.\n", ntohl(data_pkt.seq_num),offsetof(data_pkt_t, data) + data_len);
        if (sent_len != offsetof(data_pkt_t, data) + data_len) {
          fprintf(stderr, "Truncated packet.\n");
          exit(-1);
        }
        seq_num++;
        seqN++;
      }
      // recvfrom 
      ssize_t len = recvfrom(sockfd, &ack_pkt, sizeof(ack_pkt), 0,
                  (struct sockaddr *)&srv_addr, &(socklen_t){sizeof(srv_addr)});
      if(len > 0 ){
        printf("Received ACK %d / %d.\n" , ntohl(ack_pkt.seq_num), ntohl(ack_pkt.selective_acks));
        base = seqN -window;
      }else{
        

        for (size_t i = base; i < seqN ; i++)
        {
          data_pkt.seq_num = htonl(seq_num);

          // Load data from file.
          data_len = fread(data_pkt.data, 1, sizeof(data_pkt.data), file);
          ack_pkt.seq_num = htonl(seq_ack);
          fseek(file, i*1000, SEEK_SET);

          ssize_t sent_len = sendto(sockfd, &data_pkt, offsetof(data_pkt_t, data) + data_len, 0,
                    (struct sockaddr *)&srv_addr, sizeof(srv_addr));
          printf("Sending segment %d, size %ld.\n", ntohl(data_pkt.seq_num),offsetof(data_pkt_t, data) + data_len);
          if (sent_len != offsetof(data_pkt_t, data) + data_len) {
            fprintf(stderr, "Truncated packet.\n");
            exit(-1);
          }
          seq_num++;
        }
        
      } 
      seq_ack++;
    } while (!(feof(file) && data_len < sizeof(data_pkt.data)));

    for (size_t i = 0; i < window -1; i++){
      ssize_t len = recvfrom(sockfd, &ack_pkt, sizeof(ack_pkt), 0,
                (struct sockaddr *)&srv_addr, &(socklen_t){sizeof(srv_addr)});
      if(len > 0 ){
        printf("Received ACK %d / %d.\n" , ntohl(ack_pkt.seq_num), ntohl(ack_pkt.selective_acks));
      } 
    }
  }
  
  // Clean up and exit.
  close(sockfd);
  fclose(file);

  return 0;
}


/*



















#include "packet-format.h"
#include <limits.h>
#include <netdb.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  char *file_name = argv[1];
  char *host = argv[2];
  int port = atoi(argv[3]);
  int window = atoi(argv[4]);

  FILE *file = fopen(file_name, "r");
  if (!file) {
    perror("fopen");
    exit(-1);
  }

  // Prepare server host address.
  struct hostent *he;
  if (!(he = gethostbyname(host))) {
    perror("gethostbyname");
    exit(-1);
  }

  struct sockaddr_in srv_addr = {
      .sin_family = AF_INET,
      .sin_port = htons(port),
      .sin_addr = *((struct in_addr *)he->h_addr),
  };

  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd == -1) {
    perror("socket"); 
    exit(-1);
  }

  int ack_rec = 1;
  uint32_t number = 1;
  ack_pkt_t ack_pkt;
  uint32_t seq_num = 0;
  uint32_t seq_ack = 1;
  data_pkt_t data_pkt;
  size_t data_len;
  struct timeval tv;
  int cont;

  tv.tv_sec = 1;
  tv.tv_usec = 0;

  if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0){
    perror("dbgsjkgbshkbg");
  }


  if(window == 1){
    do { // Generate segments from file, until the the end of the file.
      // Prepare data segment.
      data_pkt.seq_num = htonl(seq_num++);

      // Load data from file.
      data_len = fread(data_pkt.data, 1, sizeof(data_pkt.data), file);

      
      // Send segment.
      if(ack_rec== 1){
        ack_pkt.seq_num = htonl(seq_ack);
        ssize_t sent_len = sendto(sockfd, &data_pkt, offsetof(data_pkt_t, data) + data_len, 0,
                  (struct sockaddr *)&srv_addr, sizeof(srv_addr));
        printf("Sending segment %d, size %ld.\n", ntohl(data_pkt.seq_num),offsetof(data_pkt_t, data) + data_len);
        if (sent_len != offsetof(data_pkt_t, data) + data_len) {
          fprintf(stderr, "Truncated packet.\n");
          exit(-1);
        }
      }
    
      ssize_t len = recvfrom(sockfd, &ack_pkt, sizeof(ack_pkt), 0,
                  (struct sockaddr *)&srv_addr, &(socklen_t){sizeof(srv_addr)});
      if(len >= 0  && ack_pkt.seq_num == data_pkt.seq_num + htonl(number)){
        printf("Received ACK %d / %d.\n" , ntohl(ack_pkt.seq_num),ntohl(ack_pkt.selective_acks));
        ack_rec = 1;
        cont = 0;
      }else{   
        ack_pkt.seq_num = htonl(seq_ack);
        ssize_t sent_len = sendto(sockfd, &data_pkt, offsetof(data_pkt_t, data) + data_len, 0,
                  (struct sockaddr *)&srv_addr, sizeof(srv_addr));
        printf("Sending segment %d, size %ld.\n", ntohl(data_pkt.seq_num),offsetof(data_pkt_t, data) + data_len);
        if (sent_len != offsetof(data_pkt_t, data) + data_len) {
          fprintf(stderr, "Truncated packet.\n");
          exit(-1);
        }
        ssize_t len = recvfrom(sockfd, &ack_pkt, sizeof(ack_pkt), 0,
                  (struct sockaddr *)&srv_addr, &(socklen_t){sizeof(srv_addr)});
        if(len >= 0  && ack_pkt.seq_num == data_pkt.seq_num + htonl(number)){
          printf("Received ACK %d / %d.\n" , ntohl(ack_pkt.seq_num),ntohl(ack_pkt.selective_acks));
          ack_rec = 1;
          cont = 0;
        }
      }
      seq_ack++;
    } while (!(feof(file) && data_len < sizeof(data_pkt.data)));
  }
  
  int base = 0;
  int seqN = 1;
  int flag = 1;
  int flag1;

  if (window != 1){
    do{
      
      while ((seqN - base) <= window){
        data_pkt.seq_num = htonl(seq_num);

        // Load data from file.
        data_len = fread(data_pkt.data, 1, sizeof(data_pkt.data), file);
        ack_pkt.seq_num = htonl(seq_ack);

        ssize_t sent_len = sendto(sockfd, &data_pkt, offsetof(data_pkt_t, data) + data_len, 0,
                  (struct sockaddr *)&srv_addr, sizeof(srv_addr));
        printf("Sending segment %d, size %ld.\n", ntohl(data_pkt.seq_num),offsetof(data_pkt_t, data) + data_len);
        if (sent_len != offsetof(data_pkt_t, data) + data_len) {
          fprintf(stderr, "Truncated packet.\n");
          exit(-1);
        }
        if (sent_len < 1000)
        {
          flag1 = seq_num;
        }
        
        seq_num++;
        seqN++;
      }
      // recvfrom 
      ssize_t len = recvfrom(sockfd, &ack_pkt, sizeof(ack_pkt), 0,
                  (struct sockaddr *)&srv_addr, &(socklen_t){sizeof(srv_addr)});
      if(len > 0 ){
        if(ntohl(ack_pkt.seq_num) -1 == seqN - window){
          printf("Received ACK %d / %d.\n" , ntohl(ack_pkt.seq_num), ntohl(ack_pkt.selective_acks));
          base = seqN - window;
          flag = 1;
          if (ntohl(ack_pkt.seq_num) -1 == flag1)
          {
            flag = 0;
          }   
        }
        
      }else{
        for (size_t i = base; i < seqN ; i++)
        {
          fseek(file, i*1000, SEEK_SET);
          data_pkt.seq_num = htonl(i);
          data_len = fread(data_pkt.data, 1, sizeof(data_pkt.data), file);
          ack_pkt.seq_num = htonl(i);
          

          ssize_t sent_len = sendto(sockfd, &data_pkt, offsetof(data_pkt_t, data) + data_len, 0,
                    (struct sockaddr *)&srv_addr, sizeof(srv_addr));
          printf("Sending segment %d, size %ld.\n", ntohl(data_pkt.seq_num),offsetof(data_pkt_t, data) + data_len);
          if (sent_len != offsetof(data_pkt_t, data) + data_len) {
            fprintf(stderr, "Truncated packet.\n");
            exit(-1);
          }
        }
        
      } 
      seq_ack++;
    } while (flag);
  }
  // Clean up and exit.
  close(sockfd);
  fclose(file);

  printf("122132432424");
  return 0;
}


*/


