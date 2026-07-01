#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <termios.h>

void get_password(char *password) {
	struct termios oldt, newt;
	
	printf("Enter password: ");
	fflush(stdout);

	tcgetattr(STDIN_FILENO, &oldt);
	newt=oldt;
	newt.c_lflag &= ~ECHO;
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	scanf("%99s", password);

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	printf("\n");
}

void run_gpg(char *mode, char *filename) {
	int pipefd[2];
	char password[16];
	get_password(password);
	pipe(pipefd);
	pid_t pid = fork();

	if(pid==0) {
		dup2(pipefd[0], STDIN_FILENO);
		close(pipefd[1]);
		if(strcmp(mode, "encrypt")==0) {
			execlp("gpg", "gpg", "--batch", "--yes", "--pinentry-mode", "loopback", "--passphrase-fd", "0", "-c", filename, NULL);
		} else {
			execlp("gpg", "gpg", "--batch", "--yes", "--pinentry-mode", "loopback", "--passphrase-fd", "0", "-d", filename, NULL);

		} 
		perror("exec failed");
		exit(1);
	} 

	close(pipefd[0]);
	write(pipefd[1], password, strlen(password));
	write(pipefd[1], "\n", 1);
	close(pipefd[1]);
	wait(NULL);

}


int main(int argc, char *argv[]) {
	if(argc!=3) {
		printf("Usage: %s encrypt|decrypt file \n", argv[0]);
		return 1;
	}
	run_gpg(argv[1], argv[2]);
	printf("Done.\n");
	return 0;
}





	
