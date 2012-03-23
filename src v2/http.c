#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

void *httpServer(void *arg) {
	int server_sockfd, client_sockfd;
	int server_len, client_len;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	char *http_ok = "HTTP/1.1 200 OK\nServer: JB OLED Server (Gumstix)\nConnection: Keep-alive\n\n";
	char *bmp_head = "BM\x46\xC7\0\0\0\0\0\0\x36\0\0\0\x28\0\0\0\x82\0\0\0\x82\0\0\0\x01\0\x18\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
	char *html = "<html>\n<h1>Hello World!</h1>\n</html>";
	char temp[500];
	int bytes_sent, req = 1;
	extern int8 frame_buffer[oled_screenheight][oled_screenwidth][3];
	int bmp_size = oled_screenheight * (oled_screenwidth * 3 + 2);
	int8 bmp_buffer[bmp_size];
	int i, i2, i3, x;
	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(80);
	server_len = sizeof(server_address);
	bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
	listen(server_sockfd, 5);
	while (1) {
		client_len = sizeof(client_address);
		client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
		//send(client_sockfd, html, strlen(html), 0);
		// For debugging :D  (outputs bitmap data http style)
		printf("HTTP request #%u\n", req);
		req++;
		x = 0;
		for (i = 129; i >= 0; i --) {
	        for (i2 = 0; i2 < 130; i2 ++) {
				for (i3 = 2; i3 >= 0; i3 --) {
					if (frame_buffer[i][i2][i3] == 0) {
						bmp_buffer[x] = 0;
					} else {
						bmp_buffer[x] = frame_buffer[i][i2][i3] * 4;
					}
					x++;
				}
	        }
			bmp_buffer[x] = 0;
			x++;
			bmp_buffer[x] = 0;
			x++;
	    }
		send(client_sockfd, http_ok, strlen(http_ok), 0);
		send(client_sockfd, bmp_head, 54, 0);
		write(client_sockfd, &bmp_buffer, 50960);
		usleep(50000);
		close(client_sockfd);
	}
}
