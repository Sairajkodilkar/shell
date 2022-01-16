#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

/* CREDITS:
 *	The some part of the code is inspired by the GNU libc manual which
 *	describes the implementation of the shell:
 *	https://www.gnu.org/software/libc/manual/html_node/Initializing-the-Shell.html
 */

/*TODO: handle the escaped delim while tokenizing
 *		NOTE: need to write my own strtok
 */

#define PS1_DFL ("\W >") 
#define PS1_MAX (PATH_MAX + 1 + 100) //100 bytes buffer for extra chars such as username
#define JOB_DELIM (";\n")
#define CMD_DELIM ("|")
#define WRD_DELIM (" \t\v\r")

typedef struct word {
	struct word *next_word;
	char dollar;
	char assignment;
} word;

typedef struct process {
	struct process *next_process;
	word *first_word;
	char *input_redirection;
	char *output_redirection;
	pid_t pid;
	int status;
} process;

typedef struct job {
	struct job *next_job;
	process *first_process;
	char foreground;
	char *command;
	pid_t pgid;
} job;

typedef char path[PATH_MAX + 1];
char prompt_string[PS1_MAX] = ""; 


void 

void init_job(job *j) {
	if(j == NULL)
		return;
	j->next_process = NULL;
	j->first_word = NULL;
	j->input_redirection = NULL;
	j->output_redirection = NULL;
	j->pid = -1;
	j->status = -1;
}

/*TODO: handle the return values in main code */
int cd(const path dir_path) {
	int status = chdir(path);
	if(!status)
		return status;
	status = setenv("PWD", path);
	return status;
}

int fg(pid_t pid) {
}

int bg(pid_t pid) {
}

__attribute__((noreturn)) void exit_shell(int status) {

}

void export() {
}

int set_prompt_string() {
	int prompt_string_size;
	char *string;

	prompt_string[0] = '\0';
	prompt_string_size = 0;
	const char *const PS1 = getenv("PS1");

	string = strtok(PS1, "\\");
	strcat(prompt_string, string, PS1_MAX - prompt_string_size);
	prompt_string_size += strlen(string);

	while(string) {
		string = strtok(NULL, '\\');
		switch(string[0]) {
			case 'W':				//expand the to current working dir
				path current_working_dir;
				char *status;
				status = getcwd(current_working_dir, PATH_MAX);
				if(status != NULL)
					strcat(prompt_string, current_working_dir);
				//If status is null simply ignore the character
			default:				//ignore that character
				string++; 
				break
		}
		strcat(current_working_dir, PS1_MAX - prompt_string_size);
		prompt_string_size += strlen(string);
	}
	return prompt_string_size;
}

void init_shell() {

	setenv("PS1", PS1_DFL, 1);

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
}

command *parse_command(const char *const command) {
	//word is tokenize to the '\s \t \v'
	return command;
}

job *parse_job(const char *const input_job) {
	if(input_job == NULL) {
		return NULL
	}
	job *j = (job *)malloc(sizeof(job));
	init_job(j);
	input_job_copy = strdup(input_job);
	command_string = strtok(input_job_copy, CMD_DELIM);
	commands = 
	return single_job
}

job *parse_input(const char *const input) {
	job *jobs, *tmp;

	if(input == NULL) 
		return NULL;

	input_copy = strdup(input);
	job_string = strtok(input_copy, JOB_DELIM);
	jobs = parse_job(job_string);
	tmp = jobs;
	while(tmp) {
		job_string = strtok(NULL, JOB_DELIM);
		tmp->next = parse_job(job_string);
		tmp = tmp->next
	}
	free(input_copy);

	return jobs;
}

void execute_job(job) {

}


int main(int argc, char *argv[]) {

	init_shell();
	while(1) {
		ps = getenv("PS")
		input = readline(ps);
		job = parse_input();
		execute_job(job);
	}
	
	return 0;
}
