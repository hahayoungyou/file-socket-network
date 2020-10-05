/*
 파일명 : file_client4.c
 기  능 : ftp 와 비슷하게 만들기. get, put, dir, quit 구현
 컴파일 : cc -o file_client4 file_client4.c
 사용법 : file_client4 [host IP] [port]
*/
/*
 파일명 : file_client3.c
 기  능 : file_clien2에서 filename + filesize 전송
 컴파일 : cc -o file_client3 file_client3.c
 사용법 : file_client3 [host IP] [port]
*/

#ifdef _WIN32
#include <winsock.h>
#include <signal.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#else
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#ifdef _WIN32
WSADATA wsadata;
int	main_socket;

void exit_callback(int sig)
{
	closesocket(main_socket);
	WSACleanup();
	exit(0);
}

void init_winsock()
{
	WORD sversion;
	u_long iMode = 1;

	// winsock 사용을 위해 필수적임
	signal(SIGINT, exit_callback);
	sversion = MAKEWORD(1, 1);
	WSAStartup(sversion, &wsadata);
}
#endif

#define ECHO_SERVER "127.0.0.1"
#define ECHO_PORT "30000"
#define BUF_LEN 128

int main(int argc, char* argv[]) {
	int s, n, len_in, len_out, msg_size;
	struct sockaddr_in server_addr;
	char* ip_addr = ECHO_SERVER, * port_no = ECHO_PORT;
	char buf[BUF_LEN + 1] = { 0 };

	if (argc == 3) {
		ip_addr = argv[1];
		port_no = argv[2];
	}
#ifdef _WIN32
	printf("Windows : ");
	init_winsock();
#else // Linux
	printf("Linux : ");
#endif 

	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("can't create socket\n");
		exit(0);
	}
#ifdef _WIN32
	main_socket = s;
#endif 



	/* echo 서버의 소켓주소 구조체 작성 */
	memset((char*)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip_addr);
	server_addr.sin_port = htons(atoi(port_no));



	/* 연결요청 */
	printf("Connecting %s %s\n", ip_addr, port_no);

	if (connect(s, (struct sockaddr*)&server_addr,
		sizeof(server_addr)) < 0) {
		printf("can't connect.\n");
		exit(0);
	}

	while (1) {
		char buf[BUF_LEN + 1] = { 0 };
		char buf2[BUF_LEN + 1] = { 0 };
		char buf3[BUF_LEN + 1] = { 0 };
		char buf4[BUF_LEN + 1] = { 0 };
		char menu[BUF_LEN + 1], filename[BUF_LEN + 1] = { 0 };

		memset(buf, '\0', BUF_LEN);
		printf("file client4 > ");
		if (fgets(buf, BUF_LEN, stdin)) { // gets(buf);
			len_out = strlen(buf);
			//buf[BUF_LEN] = '\0';
		}
		else {
			printf("fgets error\n");
			exit(0);
		}
		//printf("%s", buf);
		//memset(buf2, '\0', BUF_LEN);
		//printf("%s", buf2);

		if (send(s, buf, BUF_LEN, 0) <= 0) {
			printf("send error\n");
			exit(0);
		}

		memset(menu, '\0', BUF_LEN);
		memset(filename, '\0', BUF_LEN);
		sscanf(buf, "%s %s", menu, filename);


		if (strcmp(menu, "put") == 0) {
			FILE* fp;
			/*char filename[BUF_LEN] = "data.txt"; // data file 예
			printf("Enter file name : ");
			scanf("%s", filename);
			getchar(); // Enter key 처리.*/

			if ((fp = fopen(filename, "rb")) == NULL) {
				printf("Can't open file %s\n", filename);
				exit(0);
			}

			int filesize;
			int readsum = 0, nread;
			fseek(fp, 0, 2);
			filesize = ftell(fp);
			rewind(fp);


			sprintf(buf2, "%s %d", filename, filesize);
			printf("sending %s %d bytes\n", filename, filesize);
			if (send(s, buf2, BUF_LEN, 0) <= 0) {
				printf("filename send error\n");
				exit(0);
			}
			readsum = 0;
			if (filesize < BUF_LEN)
				nread = filesize;
			else
				nread = BUF_LEN;
			while (readsum < filesize) {
				int n;
				memset(buf2, 0, BUF_LEN + 1);
				n = fread(buf2, 1, BUF_LEN, fp);
				if (n <= 0)
					break;
				//printf("Sending %d bytes: %s\n", n, buf2);
				if (send(s, buf2, n, 0) <= 0) {
					printf("send error\n");
					break;
				}
				readsum += n;
				if ((nread = (filesize - readsum)) > BUF_LEN)
					nread = BUF_LEN;
			}
			if (recv(s, buf3, BUF_LEN, 0) <= 0) {
				printf("filename recv error\n");
				exit(0);
			}//파일이름 받기
			if (strcmp(buf, "finish\n"))
				printf("File %s %d transffered\n", filename, filesize);

			fclose(fp);

		}


		else if (strcmp(menu, "get") == 0) {
			/*printf("%s\n", filename);
			if (send(s, filename, BUF_LEN, 0) <= 0) {
				printf("filename send error\n");
				exit(0);
			}// 1 파일 이름 먼저 보내기*/

			FILE* fp;
			int filesize, readsum = 0, nread = 0, n;
			if (recv(s, buf3, BUF_LEN, 0) <= 0) {
				printf("filesize recv error\n");
				exit(0);
			}//파일사이즈 받기

			sscanf(buf3, "%s %d", filename, &filesize);
			//msg_size = recv(s, buf3, BUF_LEN, 0);

			if ((fp = fopen(filename, "wb")) == NULL) {
				printf("file open error\n");
				exit(0);
			}
			readsum = 0;

			if (filesize < BUF_LEN)
				nread = filesize;
			else
				nread = BUF_LEN;

			memset(buf3, 0, BUF_LEN + 1);
			printf("Receiving %s %d  bytes\n", filename, filesize);
			while (readsum < filesize) {

				n = recv(s, buf3, nread, 0);
				if (n <= 0) {
					printf("\n end of file\n");
					break;
				}//비정상적 종료
				//printf("read data=%d bytes : %s\n", n, buf2);
				if (fwrite(buf3, n, 1, fp) <= 0) {
					printf("fwrite error\n");
					break;
				}
				readsum += n;
				if ((nread = (filesize - readsum)) > BUF_LEN)
					nread = BUF_LEN;
			}


			printf("\n filedata %s %d received \n", filename, filesize);

			char* s4 = "finish\n";
			s4 = buf4;
			if (send(s, buf4, BUF_LEN, 0) <= 0) {
				printf("filename send error\n");
				exit(0);
			}

			fclose(fp);

		}

		else if (strcmp(menu, "dir") == 0) {


			int len_out = 0;
			while (1) {
				memset(buf3, '\0', BUF_LEN);

				msg_size = recv(s, buf3, BUF_LEN, 0);
				len_out = strlen(buf3);
				//printf("\n%d\n", len_out);
				if (len_out < BUF_LEN) {
					printf(buf3);
					break;
				}

				printf(buf3);
			}

			memset(buf4, '\0', BUF_LEN);
			

			if (recv(s, buf4, BUF_LEN, 0) <= 0) {
				printf("2recv error\n");
				exit(0);
			}//파일사이즈 받기
			printf(buf4);
			
			if (strncmp(buf4, "-EOF-", 5) == 0) {
				printf("\n");
			}

		}
		else if (strcmp(menu, "quit") == 0) {
			printf("client end.\n");
			exit(0);
		}
		else if (strcmp(menu, "ldir") == 0) {
			system("dir");
		}
		else if (menu[0] == ('!')) {
			system(menu + 1);
		}
		}
#ifdef _WIN32
	closesocket(s);
#else
	close(s);
#endif
	return(0);
	}

