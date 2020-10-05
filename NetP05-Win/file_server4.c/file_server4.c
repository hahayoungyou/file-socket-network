/*
파일명 : file_server3.c
기  능 : file 을 수신해서 저장하는 서버 file_server2에서 filename 과 filesize를 먼저 전송 받는다.
컴파일 : cc -o file_server3 file_server3.c
사용법 : file_server3 [port]
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

#define BUF_LEN 128
#define file_SERVER "0.0.0.0"
#define file_PORT "30000"

int main(int argc, char* argv[]) {
	struct sockaddr_in server_addr, client_addr;
	int server_fd, client_fd;			/* 소켓번호 */
	int len, msg_size;
	char buf[BUF_LEN + 1];
	char buf2[BUF_LEN + 1];
	char buf3[BUF_LEN + 1];
	char buf4[BUF_LEN + 1];
	char buf5[BUF_LEN + 1];
	char menu[BUF_LEN + 1];
	char filename[BUF_LEN + 1];
	unsigned int set = 1;
	char* ip_addr = file_SERVER, * port_no = file_PORT;

	if (argc == 2) {
		port_no = argv[1];
	}
#ifdef WIN32
	printf("Windows : ");
	init_winsock();
#else
	printf("Linux : ");
#endif
	/* 소켓 생성 */
	if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Server: Can't open stream socket.");
		exit(0);
	}
#ifdef WIN32
	main_socket = server_fd;
#endif

	printf("file_server1 waiting connection..\n");
	printf("server_fd = %d\n", server_fd);
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&set, sizeof(set));

	/* server_addr을 '\0'으로 초기화 */
	memset((char*)&server_addr, 0, sizeof(server_addr));
	/* server_addr 세팅 */
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(port_no));

	/* bind() 호출 */
	if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		printf("Server: Can't bind local address.\n");
		exit(0);
	}

	/* 소켓을 수동 대기모드로 세팅 */
	listen(server_fd, 5);

	/* iterative  file 서비스 수행 */
	printf("Server : waiting connection request.\n");
	len = sizeof(client_addr);

	while (1) {
		/* 연결요청을 기다림 */
		client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len);
		if (client_fd < 0) {
			printf("Server: accept failed.\n");
			exit(0);
		}

		printf("Client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		printf("client_fd = %d\n", client_fd);

		while (1) {
			printf("\nwaiting client command\n");

			int s = 0, n;
			memset(buf, 0, BUF_LEN);
			msg_size = recv(client_fd, buf, BUF_LEN, 0);
			if (msg_size <= 0) {
				printf("recv error\n");
				break;
			}
			memset(buf2, '\0', BUF_LEN);

			//buf[msg_size] = '\0'; // 문자열 끝에 NULL 추가


			memset(menu, '\0', BUF_LEN);
			memset(filename, '\0', BUF_LEN);
			sscanf(buf, "%s %s", menu, filename);



			if (strcmp(menu, "put") == 0) {
				char filename[BUF_LEN];
				FILE* fp;
				int filesize, readsum = 0, nread = 0, n;
				if (recv(client_fd, buf2, BUF_LEN, 0) <= 0) {
					printf("filename recv error\n");
					exit(0);
				}//파일이름 받기

				sscanf(buf2, "%s %d", filename, &filesize);
				printf("Received %d %s %d\n", msg_size, buf, filesize);
				//printf("Received file name:%s size=%d\n", filename, filesize);
				if ((fp = fopen(filename, "wb")) == NULL) {
					printf("file open error\n");
					exit(0);
				}
				readsum = 0;

				if (filesize < BUF_LEN)
					nread = filesize;
				else
					nread = BUF_LEN;

				memset(buf2, 0, BUF_LEN + 1);
				printf("Receiving %s %d  bytes\n", filename, filesize);
				while (readsum < filesize) {

					n = recv(client_fd, buf2, nread, 0);
					if (n <= 0) {
						printf("\n end of file\n");
						break;
					}//비정상적 종료
					//printf("read data=%d bytes : %s\n", n, buf2);
					if (fwrite(buf2, n, 1, fp) <= 0) {
						printf("fwrite error\n");
						break;
					}
					readsum += n;
					if ((nread = (filesize - readsum)) > BUF_LEN)
						nread = BUF_LEN;
				}
				fclose(fp);

				printf("\n filedata %s %d bytes received. \n", filename, filesize);
				char* s3 = "finish\n";
				s3 = buf5;
				if (send(client_fd, buf5, BUF_LEN, 0) <= 0) {
					printf("filename send error\n");
					exit(0);
				}

			}
			else if (strcmp(menu, "get") == 0) {
				/*if (recv(client_fd, filename, BUF_LEN, 0) <= 0) {
					printf("filename recv error\n");
					exit(0);
				}//파일이름 받기*/
				//printf("%s\n", filename);

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

				memset(buf3, '\0', BUF_LEN);
				sprintf(buf3, "%s %d", filename, filesize);

				printf("Receiving %d  %s %d bytes\n", msg_size,buf3, filesize);
				if (send(client_fd, buf3, BUF_LEN, 0) <= 0) {
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
					memset(buf3, 0, BUF_LEN + 1);
					n = fread(buf3, 1, BUF_LEN, fp);
					if (n <= 0)
						break;
					
					if (send(client_fd, buf3, n, 0) <= 0) {
						printf("send error\n");
						break;
					}
					readsum += n;
					if ((nread = (filesize - readsum)) > BUF_LEN)
						nread = BUF_LEN;
				}
				printf("sending file %s %d bytes.\n", filename, filesize);

				if (recv(client_fd, buf4, BUF_LEN, 0) <= 0) {
					printf("finish check error\n");
					exit(0);
				}
				if (strcmp(buf4, "finish\n"))
					printf("File %s %d sent.\n", filename, filesize);

				fclose(fp);

			}

			else if (strcmp(menu, "dir") == 0) {
			FILE* fp;
			
				

#ifdef _WIN32
				fp = _popen(menu, "rb");
			

#else
				if (strcmp(menu, "dir") == 0)
					strcpy(menu, "ls -l");
				//fp = popen("ls -l", "rb");
				if ((fp = popen("ls -l", "r"))== NULL) { //rb하면 cant open file 뜬다.
					printf("Can't open file \n");
					exit(0);
				}

#endif

				printf("Received %d dir \n", msg_size);
				while (1) {
					memset(buf3, '\0', BUF_LEN);

					n = fread(buf3, 1, BUF_LEN, fp);
					//printf("\n남은 바이트 %d\n", n);
					if (n == 0) {
						break;
					}

					send(client_fd, buf3, BUF_LEN, 0);



					//printf(buf3);

				}
				memset(buf4, '\0', BUF_LEN);
				strcpy(buf4, "-EOF-");
				//printf(buf4);
				if (send(client_fd, buf4, BUF_LEN, 0) <= 0) {
					printf("2send error\n");
					break;
				}

				printf("Sending directory listing\n");

#ifdef _WIN32
				_pclose(fp);
#else
				pclose(fp);
#endif


			}
			else if (strcmp(menu, "quit") == 0) {
				break;
			}

		}
#ifdef _WIN32
		closesocket(client_fd);
#else
		close(client_fd);
#endif

	}
#ifdef _WIN32
	closesocket(server_fd);
#else
	close(server_fd);
#endif	
	return(0);
}

