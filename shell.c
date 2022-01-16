#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>

/* CREDITS:
 *	The some part of the code is inspired by the GNU libc manual which
 *	describes the implementation of the shell:
 *	https://www.gnu.org/software/libc/manual/html_node/Initializing-the-Shell.html
 */

/*TODO: handle the escaped delim while tokenizing
 *		NOTE: need to write my own strtok
 *TODO: handle & symbol which again breaks the job
 */

#define PS1_DFL ("\\W >") 
#define PS1_MAX (PATH_MAX + 1 + 100) //100 bytes buffer for extra chars such as username
#define JOB_DELIM (";\n")
#define PROC_DELIM ("|")
#define WRD_DELIM (" \t\v\r")
#define MAX_ARG_SIZE (200)

typedef struct word {
	struct word *next_word;
	char *w;
	char *assignment;
	char dollar;
} word;

typedef struct command {
	struct command *next_command;
	word *first_word;
	char *input_redirection;
	char *output_redirection;
	pid_t pid;
} command;

typedef struct job {
	struct job *next_job;
	command *first_command;
	char foreground;
	char *command;
	pid_t pgid;
	int status;
} job;

typedef char path[PATH_MAX + 1];
char prompt_string[PS1_MAX] = ""; 


void init_word(word *w) {
	if(w == NULL)
		return;
	w->next_word = NULL;
	w->w = NULL;
	w->assignment = NULL;
	w->dollar = 0;
	return;
}

void init_command(command *c) {
	if(c == NULL)
		return;
	c->next_command = NULL;
	c->first_word = NULL;
	c->input_redirection = NULL;
	c->output_redirection = NULL;
	c->pid = -1;
	return;
}

void init_job(job *j) {
	if(j == NULL)
		return;
	j->next_job = NULL;
	j->first_command = NULL;
	j->foreground = 0;
	j->command = NULL;
	j->pgid = -1;
	j->status = -1;
	return;
}

/*TODO: handle the return values in main code */
int cd(const path dir_path) {
	int status = chdir(dir_path);
	if(!status)
		return status;
	status = setenv("PWD", dir_path, 1);
	return status;
}

int fg(pid_t pid) {
}

int bg(pid_t pid) {
}

void export() {
}

__attribute__((noreturn)) void exit_shell(int status) {
	exit(status);
}

int set_prompt_string() {
	int prompt_string_size;
	char *string, *PS1;

	prompt_string[0] = '\0';
	prompt_string_size = 0;
	PS1 = getenv("PS1");

	string = strtok(PS1, "\\");
	strncat(prompt_string, string, PS1_MAX - prompt_string_size);
	prompt_string_size += strlen(string);

	while(string) {
		string = strtok(NULL, "\\");
		switch(string[0]) {
			case 'W':				//expand to current working dir
				path current_working_dir;
				char *status;
				status = getcwd(current_working_dir, PATH_MAX);
				if(status != NULL) {
					strncat(prompt_string, current_working_dir, PS1_MAX - prompt_string_size);
					prompt_string_size += 1;
				}

			default:				//ignore that character
				string++; 
				break;
		}
		strncat(prompt_string, string, PS1_MAX - prompt_string_size);
		prompt_string_size += strlen(string);
	}
	return prompt_string_size;
}


word *parse_word(const char *const input_word) {
	word *w;

	if(input_word == NULL || input_word[0] == '\0')
		return NULL;

	w = (word *) malloc(sizeof(word));
	init_word(w);

	if(input_word[0] == '$') {
		w->dollar = 1;
		w->w = strdup(input_word + 1);
	}
	else {
		/* Assignment is invalid when there is dollar*/
		w->assignment = strstr(input_word, "=");
		w->w = strdup(input_word);
	}

	return w;
}

command *parse_command(const char *const input_command) {
	char *word_string, *input_command_copy, *saveptr;
	word *tmp;
	command *c;

	if(input_command == NULL || input_command[0] == '\0')
		return NULL;

	c = (command *) malloc(sizeof(command));
	init_command(c);

	input_command_copy = strdup(input_command);
	word_string = strtok_r(input_command_copy, WRD_DELIM, &saveptr);
	c->first_word = parse_word(word_string);
	tmp = c->first_word;

	while(tmp) {
		word_string = strtok_r(NULL, WRD_DELIM, &saveptr);
		tmp->next_word = parse_word(word_string);
		tmp = tmp->next_word;
	}
	free(input_command_copy);

	return c;
}

job *parse_job(const char *const input_job) {
	char *input_job_copy, *command_string, *saveptr;
	command *tmp;
	job *j; 

	if(input_job == NULL || input_job[0] == '\0') 
		return NULL;

	j = (job *)malloc(sizeof(job));
	init_job(j);

	input_job_copy = strdup(input_job);
	command_string = strtok_r(input_job_copy, PROC_DELIM, &saveptr);
	j->first_command = parse_command(command_string);
	tmp = j->first_command;

	while(tmp) {
		command_string = strtok_r(NULL, PROC_DELIM, &saveptr);
		tmp->next_command = parse_command(command_string);
		tmp = tmp->next_command;
	}

	free(input_job_copy);
	return j;
}

job *parse_input(const char *const input) {
	/* TODO: handle the & here */
	char *input_copy, *job_string, *saveptr;
	job *jobs, *tmp;

	if(input == NULL || input[0] == '\0') 
		return NULL;

	input_copy = strdup(input);
	job_string = strtok_r(input_copy, JOB_DELIM, &saveptr);
	jobs = parse_job(job_string);
	tmp = jobs;
	while(tmp) {
		tmp->command = job_string;
		job_string = strtok_r(NULL, JOB_DELIM, &saveptr);
		tmp->next_job = parse_job(job_string);
		tmp = tmp->next_job;
	}

	return jobs;
}

void init_shell() {

	setenv("PS1", PS1_DFL, 1);
	/* TODO init history */
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
}

void free_word(word **w) {
	word *tmp;
	tmp = *w;
	*w = *w->next_word;
	free(tmp->w);
	free(tmp);
	return;
}

void free_command(command **c) {
	word *w = *c->first_word;
	command *tmp
	while(w) {
		free_word(&w);
	}
	tmp = *c;
	*c = *c->next_command;
	free(tmp->input_redirection);
	free(tmp->output_redirection);
	free(tmp);
	return;
}

free_job(job **j) {
	command *c;
	job *tmp;
	c = *j->first_command;
	while(c) {
		free_command(&c);
	}
	tmp = *j;
	*j = *j->next_job;
	free(tmp->command);
	free(tmp);
}

void preprocess_command(command *c) {
	if(c == NULL)
		return;
	char *name, *value;
	if(!c->first_word->dollar && c->first_word->assignment != NULL) {

		c->first_word->assignment = '\0';
		name = c->first_word->w;
		value = c->first_word->assignment + 1;

		setenv(name, value, 1);

		free_command(&c);
	}
	word *w = c->first_word;
	while(w) {
		if(w->dollar) {
			value = getenv(w->w);
			free(w->w);
			w->w = strdup(value);
		}
		w = w->next;
	}
	return;
}

__attribute__ ((noreturn)) 
void execute_command( command *c, int input_fd, int output_fd) {
	char *w;
	char **argv[MAX_ARG_SIZE + 1];
	int i;

	preprocess_command(c);

	/* restore signals for the child */
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	signal(SIGTSTP, SIG_DFL);
	signal(SIGTTIN, SIG_DFL);
	signal(SIGTTOU, SIG_DFL);
	signal(SIGCHLD, SIG_DFL);

	i = 0;
	w = c->first_word;
	while(w && i < MAX_ARG_SIZE) {
		argv[i++] = w->w;
		w = w->next_word;
	}
	argv[i] = (char *)NULL;

	execvp(c->first_word, argv);
	perror("execvp");
}

void execute_job(job *j) {

	command *c = j->first_command;
	while(c) {
		/* TODO: handle pipeline and redirection here */
		/* TODO: handle group ids */
		pid_t child_pid = fork();
		c->pid = child_pid;
		switch(child_pid) {
			case -1:
				perror("fork");
				break;
			case 0:
				execute_command(c);
				break;
			default:
				int wstatus;
				if(j->foreground){
					waitpid(child_pid, &wstatus, WUNTRACED);
					j->status = wstatus;
				}
				else {
					/* TODO: add job on the running queue */
				}
		}
	}
	return;
}

int main(int argc, char *argv[]) {

	job *jobs;

	init_shell();

	while(1) {

		set_prompt_string();

		input = readline(prompt_string);
		add_history(input);

		if(input == NULL) {
			exit_shell();
		}

		jobs = parse_input(input);

		while(jobs) {
			execute_job(jobs);
			jobs = jobs->next_job;
			/* TODO: check for the job completion status */
		}

	}

	return 0;
}
