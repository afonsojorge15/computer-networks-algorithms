#include "packet-format.h"
#include <arpa/inet.h>
#include <limits.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
   char *file_name = argv[1];
   int port = atoi(argv[2]);

   FILE *file = fopen(file_name, "w");
   if (!file) {
      perror("fopen");
      exit(-1);
   }

   // Prepare server socket.
   int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
   if (sockfd == -1) {
      perror("socket");
      exit(-1);
   }

   // Allow address reuse so we can rebind to the same port,
   // after restarting the server.
   if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) <
      0) {
      perror("setsockopt");
      exit(-1);
   }

   struct sockaddr_in srv_addr = {
      .sin_family = AF_INET,
      .sin_addr.s_addr = htonl(INADDR_ANY),
      .sin_port = htons(port),
   };
   if (bind(sockfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr))) {
      perror("bind");
      exit(-1);
   }
   fprintf(stderr, "Receiving on port: %d\n", port);

   ssize_t len;
   uint32_t number = 1;
   uint32_t seq_num = 0;
   data_pkt_t data_pkt;
   ack_pkt_t ack_pkt;
   int a = 1;
   int c = 1;
   do { // Iterate over segments, until last the segment is detected.
      // Receive segment.
      struct sockaddr_in src_addr;
      
      len = recvfrom(sockfd, &data_pkt, sizeof(data_pkt), 0,
                  (struct sockaddr *)&src_addr, &(socklen_t){sizeof(src_addr)});
      if(len > 0 && data_pkt.seq_num == htonl(seq_num)) {
         printf("Received segment %d, size %ld.\n", ntohl(data_pkt.seq_num), len);
         data_pkt.seq_num = htonl(seq_num);
         ack_pkt.seq_num = data_pkt.seq_num + htonl(number);
         ack_pkt.selective_acks = htonl(0b00);
         fwrite(data_pkt.data, 1, len - offsetof(data_pkt_t, data), file);
         
         seq_num++;

         if (len != sizeof(data_pkt_t))
         {
            c =0;
         }
         
      }
      ssize_t recv_len = sendto(sockfd, &ack_pkt, sizeof(ack_pkt), 0,
               (struct sockaddr *)&src_addr, sizeof(src_addr));
      printf("Sending ACK %d / %d.\n" , ntohl(ack_pkt.seq_num), ntohl(ack_pkt.selective_acks));

      if (c ==0 )
      {
         a = 0;
      }
      
   } while (a);



   // Clean up and exit.
   close(sockfd);
   fclose(file);

   return 0;
}


