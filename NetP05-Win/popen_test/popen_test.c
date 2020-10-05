// popen_test.c 
// 명령어를 실행하고 결과를 파일로 읽는 예제
#include <stdio.h>
#include <stdlib.h>
void main()
{
	char str[129] = { 0 };
	FILE* fp;
	char buf[129] = { 0 };
	int n;

	while (1) {
		printf("\nEnter command : ");
		gets(str);
		if (strcmp(str, "quit") == 0)
			break;
		//system(str);
		// file_server4는 gets(str) 대신
		// recv(client_fd, buf, 128, 0); 을 사용하고
		// if (strncmp("dir", buf, 3)==0) 이면
		//  popen() 처리를 하고 결과를 보내면 된다.

#ifdef _WIN32
		fp = _popen(str, "rb");
#else // Linux 는 dir 대신 ls -l 사용
		if (strcmp("dir", str) == 0)
			strcpy(str, "ls -l");
		fp = popen(str, "rb");
#endif
		while (1) {
			memset(buf, 0, 128);
			n = fread(buf, 1, 128, fp);
			if (n <= 0)
				break;
			//printf("n=%d\n", n);
			printf(buf); // 화면에 출력
			// file_server4는
			// 만약 ftp_client4에서 dir 명령어를 요청한다면
			// printf() 대신 send(client_fd, buf, n, 0); 쓰면 된다.
			// 
		}
#ifdef _WIN32
		_pclose(fp);
#else
		pclose(fp);
#endif
	}
}