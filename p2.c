/*
	Name: Nimish Saxena
	Professor: Jon Carroll
	Class: CS570
	Due Date: 4/25/2018
	Summary: Writing various parts of a shell such as MV, cd, redirect, piping, etc. and making a parse method that parses the flags.
			 Integration of vertical multipiping algorithm for pipes as well as pipeand descriptors. 
			 Making use of printing information about child and parent id's as well making use of forks. 
*/
#include "p2.h"
#include <stdio.h>

char s[1000];
char *argv[MAXITEM];
int counter = 0;
char *in_string, *out_string;
int pipeflag = 0; int andflag = 0; int poundflag = 0; int leftcaretflag = 0; int rightcaretflag = 0; int pipeandflag = 0;
int parseInt;
int pipearray[20];
int pipeandarray[20];
int k = 0;
int flags = 0;
int input_file = 0; int output_file = 0;
int main(int argc, char** cmdarray){
	signal(SIGTERM,myhandler);
	pid_t pid, child_pid;
	int c,p = 0;
	int i = 0;
	int tmp = 0;
	int scriptcounter = 0;
	char *first_file = "";
	char *second_file = "";
	/*Checking if there is more than necessary a number of arguments in the command line.
	  If so. then send a message to stderr and exit.*/
	if(argc >= 3){
		fprintf(stderr,"Too many arguments.\n");
		exit(0);
	}
	/*If the last check is ok, then check if the target file exists, without the redirection. 
	  If it does not exist, then throw an error and exit, if it does exist then pretend as if there is an input flag symbol
	  between the files and perform the redirection code. 
	*/
	else{
		if(access(cmdarray[1],F_OK) != 0){
			fprintf(stderr,"The target file does not exist!\n");
			exit(0);
		}
		int file = open(cmdarray[1],O_RDONLY);
		dup2(file,STDIN_FILENO);
		close(file);
	}
	for(;;){
		/*Checking if there is a redirection symbol in the command line argument, then output the prompt. 
		  My Argc only recognizes commands before the redirection symbol if one is placed. So if there is a redirection symbol, then only 1 will show for argc. 
		 */
		if(argc < 2){
			printf("p2: ");
		}
		parseInt = parse(); //The number that returns the word count shown from parse
		if(parseInt == 0){//Reissue prompt if nothing is entered
			continue;
		}
		if(parseInt == -1){//If reaches any told statements for breaking, then break out of loop
			break;
		}
		/*Code that handles the error checking for CD code. 
		  IF there are more than 2 string including the cd code, then throw error for too many arguments. 
		  If the HOME directory could not be found, then throw error saying could not find directory. 
		  If the argument after cd is not a directory, then throw errow saying that there is no such directory. 
		*/
		if(strcmp(argv[0],"cd") == 0){
			if(parseInt > 2){
				fprintf(stderr,"cd: Too many arguments.\n");

			}
			else if(parseInt == 1){	//cd was entered with no arguments 
				if(chdir(getenv("HOME")) != 0)
					fprintf(stderr,"cd: could not find home directory.\n");
			}
			else{	//argv is two so you want to cd to the argument
				if(chdir(argv[1]) != 0)
					fprintf(stderr,"%s: No such file or directory.\n",argv[1]);
			}
			continue;
		}
		/* Code that handles the MV statement. 
		   If only one argument or two arguments are present including MV< then throw not enough arguments error.
		*/
		if(strcmp(argv[0],"MV") == 0){
			int i = 0;
			if(parseInt == 1 || parseInt == 2){
				fprintf(stderr,"MV: Not enough arguments.\n");
			}
			/* Iterate through the argv array and check for the first input that is not a flag, and place it into a tmp variable
			   Iterate after the first file index through the array and find the second input that is not a flag, and place into another tmp variable.  
			*/
			else{
				for(i = 1; i < parseInt; i++){
					if(strcmp(argv[i],"-f") != 0 && strcmp(argv[i],"-n") != 0){
						first_file = argv[i];
						tmp = i+1;
						break;
					}
				}
				for(i = tmp; i < parseInt; i++){
					if(strcmp(argv[i],"-f") != 0 && strcmp(argv[i],"-n") != 0){
						second_file = argv[i];
						tmp = i;
						break;
					}
				}
			}
			//If either file is NULL or doesn't exist, then add an error saying there is not enough arguments.
			if(first_file[0] == '\0' || second_file[0] == '\0'){
				fprintf(stderr,"MV: Not enough arguments.\n");
				continue;
			}
			//If there is any file not a flag after the second file, then show that there is too many arguments. 
			if(argv[tmp+1] != NULL){
				fprintf(stderr,"MV: Too many arguments.\n");
				continue;
			}
			/* If first file exists, then see if the flag before the second file is a -f. 
			   If it is a -f, and the first file has a -n before it, then unlink the second file and move the contents from first file to second file without deleting first file. 
			   If no -n before first file, then delete the first file as well as overwrite second file. 
			*/ 
			if(access(first_file,F_OK) == 0){
				if((strcmp(argv[tmp-1],"-f") == 0) && strcmp(argv[1],"-n") == 0){
					unlink(second_file);
					link(first_file,second_file);
				}
				else if(strcmp(argv[tmp-1],"-f") == 0){
						unlink(second_file);
						link(first_file,second_file);
						unlink(first_file);							
						
				}
				/* If a flag is anything else other than -f, and file exists, then throw an error. 
				   If the file doesn't exist, then just move contents of the first file to the second file. 
				*/
				else{
					if(access(second_file,F_OK) == 0){
						fprintf(stderr,"%s: File Already Exists.\n",second_file);
					}
					else{
						link(first_file,second_file);
						unlink(first_file);
					}
				}
			}
			//If file doesn't exist, then throw error saying file or directory doesn't exist. 
			else{
				fprintf(stderr, "%s: No such file or directory.\n",first_file);
			}	
			continue;
		}
		//> Redirect Code for output flag error checking. 
		if(rightcaretflag != 0){
			flags = 0;
			flags = O_CREAT | O_EXCL | O_RDWR;
			//if there are more than one output flag, then throw error for too many flags
			if(rightcaretflag > 2){
				fprintf(stderr,"Ambiguous input redirect.\n");
				continue;
			}
			//if there are no files or string after the flag, then throw error for no file to be redirected too. 
			if(out_string == NULL){
				fprintf(stderr,"Missing name for redirect.\n");
				continue;
			} 
			//If the target file already exists, then throw error saying that file already exists. 
			if((output_file = open(out_string,flags,S_IRWXU)) < 0){ 
				fprintf(stderr,"%s: Already Exists.\n",out_string);
				continue;
			}
			
		}
		//< Redirect Code for input flag error checking
		if(leftcaretflag != 0){
			flags = 0;
			flags = O_RDONLY;
			//if there are more than one input flag, then throw error for too many flags
			if(leftcaretflag > 2){
				fprintf(stderr,"Ambiguous input redirect.\n");
				continue;
			}
			//if there are no files or string after the flag, then throw error for no file to be redirected too. 
			if(in_string == NULL){
				fprintf(stderr,"Missing name for redirect.\n");
				continue;
			}
			//If the target file already exists, then throw error saying that file already exists. 
			if((input_file = open(in_string,flags)) < 0){
				fprintf(stderr,"Failed to open: %s\n",in_string);
				continue;
			}
		}
		/* Handling of pipeand algorithm
		   If there is more than ten pipeand flags, then throw error for max pipeands.
		   Else, call the pipeandpipes() function which is similar to regular piping but with the stderr being redirected from write side of the pipe as well. 
		*/
		if(pipeandflag != 0){
			if(pipeandflag > 10){
				fprintf(stderr,"Error '|&': Too many pipe-ands.\n");
				exit(0);
			}
			else{

				pipeandpipes();
				continue;
			}
		}
		/*Handling of pipe algorithm
		  If there s more than then pipe flags, then throw error for max pipes. 
		  Else, call the pipes() function which handles the multiple vertical piping. 
		*/
		if(pipeflag != 0){
			if(pipeflag > 10){
				fprintf(stderr,"Error '|': Too many pipes.\n");
				exit(0);
			}
			else{
				pipes();
				continue;
			}
		}
		/* Handling of the Pound Code
		   Iterating through the argv array, and setting the index of the "#" into a temporary variable.
		*/
		if(poundflag > 0){
			int poundcount = 0;
			int poundtmp = 0;
			for(poundcount = 0; poundcount < parseInt; poundcount++){
				if(strcmp(argv[poundcount],"#") == 0){
					poundtmp = poundcount;
					break;
				}
			}
			/* If there is not a redirection flag in the command line argument, and a print command is shown, 
			   Null Terminate the pound that returns a -1, when it is by itself in an input. Don't shown anything after it.
			*/
			if((argc >= 2) && (strcmp(argv[0],"echo") == 0)){
				for(poundcount = poundtmp; poundcount < parseInt; poundcount++){
					argv[poundcount] = '\0';
				}
			}
			/* If there is no echo shown in the beginning, then null terminate every input after and including the pound symbol 
			*/
			else if((strcmp(argv[poundcount],"#") == 0) && strcmp(argv[0],"echo") != 0){
				for(poundcount = poundtmp; poundcount < parseInt; poundcount++){
					argv[poundcount] = '\0';
				}			
			}
			
		}
		//flushing and forking statements. 
		fflush(stdout);
		fflush(stderr);
		child_pid = fork();//forking the child
		//If child pid is not forkable, then throw error and exit. 
		if(child_pid < 0){
			printf("Terminating! Can't Fork!");
			exit(1);
		}
		else if(child_pid == 0){//if child pid fork is a success
			//dev/null/ code
			if(andflag != 0 && leftcaretflag == 0){		//This will send STDIN to /dev/null/
				int devnull;
				flags = 0;
				flags = O_RDONLY;
				if((devnull = open("/dev/null",flags)) < 0){
					fprintf(stderr,"Failed to open /dev/null.\n");
					exit(9);
				}
				dup2(devnull,STDIN_FILENO);
				close(devnull);
			}
			/*If child id is 0 but there are some right carets shown : >
			  Duplicate whatever is in standard output to the target file, then close the output file. 
			*/
			if(rightcaretflag != 0){
				dup2(output_file,STDOUT_FILENO);
				close(output_file);
			}
			/*If child id is 0 but there are some left carets shown : <
			  Duplicate the stdin to the target file, then close the input file. 
			*/
			if(leftcaretflag != 0){
				dup2(input_file,STDIN_FILENO);
				close(input_file);
			}
			//Executes the command shown in current argv if the command is found. 
			if((execvp(*argv, argv)) < 0){	//execute the command
				fprintf(stderr,"%s: Command not found.\n",argv[0]);
						exit(9);
			}
							
		}
		
		/* If there are and symbols in the input. 
		   Then print the argv with the child id. Used for background processes outputs. 
		*/ 
		if(andflag != 0){
			printf("%s [%d]\n",*argv,child_pid);
			andflag = 0;
			//continue;
		}

		else{
			for(;;){
				pid = wait(NULL);
				if(pid == child_pid){
					break;
				}
			}
		}

	}
	killpg(getpid(),SIGTERM);
	printf("p2 terminated.\n");
	exit(0);
}
//Parse handles the flags and every character being passed from getword and puts all valid words into a char counter.
int parse(){
	int charamt = 0;
	int flagCounter = 0;
	int c = 0;
	in_string = '\0';
	out_string = '\0';
	int p = 0; //pointer used to start at the beginning of the word
	pipeflag = andflag = poundflag = leftcaretflag = rightcaretflag = pipeandflag = 0;
	while((counter = getword(s + p)) > -100){
		/*If there is a newline, then break out of the loop*/
		if(counter == -10){
			break;
		}
		//If there is no more words, then return a -1 and break out of the loop.
		if(counter == 0){
			return -1; 
		}
		/*Handling of the flags for the andflag. If statement checking if the counter is not -2 due to a '&' in the pipeand metacharacter
		  If either source files or target files are NULL with a redirection flag, then break out of the loop.
		*/
		if(*(s + p) == '&' && counter != -2){
			if((in_string == NULL && out_string == NULL) && (leftcaretflag == 1 || rightcaretflag == 1)){ //looks after >
				break;
			}
			andflag++;
			flagCounter++;
		}
		/* Handling of the flags for the pipes. Checking if the counter is not -2 due to a '|' in the pipeand metacharacter. 
		   Putting back '|' into the argv, and setting that value equal to null. 
		   Populate pipearray with every number of pipeflags.
		*/
		else if(*(s+p) == '|' && pipeflag < 10 && counter != -2){
			argv[c++] = '\0';
			pipearray[pipeflag++] = c;
		}
		/* Handling of the input flag
		   Handing the file after the flag into a temporary variable. 
		*/
		else if(*(s+p) == '<' || leftcaretflag == 1){
			leftcaretflag++; flagCounter++;
			in_string = s + p;
		}
		/* Handling of the outputflag
		   Holding the target file into a temporary variable
		*/
		else if(*(s+p) == '>' || rightcaretflag == 1){
			rightcaretflag++; flagCounter++;
			out_string = s+p;
		}
		/* Handling for pipeand metacharacter. Pipeand will always return -2. 
		   Similar to regular pipes only with a different array used. 
		*/
		else if(counter == -2){
			argv[c++] = "|&";
			pipeandarray[pipeandflag++] = c;
		}
		/* Handling of the '#' flag when it is by itself, aka returning of the -1. 
		   Putting the '#' back into the argv. 
		*/
		else if(*(s+p) == '#' && counter == -1){
			argv[c++] = "#";
			poundflag++; 
		}
		//putting all non metacharacters into the argv array. 
		else{
			argv[c++] = s + p;
		}
		s[p + counter] = '\0';
		p += counter + 1;
		charamt++;

	}

	argv[c] = '\0';
	return charamt - flagCounter;
}
/* Algorithm used for calculating multiple pipes in an input
   Method used is vertical piping which has every child from its parent fork processes. 
   Uses three process id's, parent, child, and grandchild, a file descriptor array, and a counter that tracks the amounts of children. 
*/
int pipes(){
	pid_t parent,child,grandchild;
	int fd[pipeflag * 2];
	int childcounter = 0;

	fflush(stdout);
	fflush(stderr);
	child = fork();
	if(child == 0){
		while(childcounter < pipeflag){
			pipe(fd + (childcounter*2));
			fflush(stdout);
			fflush(stderr);
			//grandchild is made when the child of the parent is forked, causing the vertical piping behavior. 
			grandchild = fork();
			if(grandchild == 0){
				childcounter++;
				//If only one pipe or last process, then redirect the input
				if(childcounter == pipeflag){
					if(leftcaretflag != 0){
						dup2(input_file, STDIN_FILENO);
						close(input_file);
					}
				}
				/* If the amount of children is less then amount of flags, then 
				   redirect the standard out to the proper position in the file descriptor array. If 2 children, then redirect to the next process. 
				*/
				if(childcounter <= pipeflag){
					dup2(fd[2 * childcounter -1],STDOUT_FILENO);

					close(fd[2 * childcounter - 1]);
				}
				continue;
			}
			/*If not the last process
			  Redirect the stdin to the proper position of the file descriptor which is the position after the index where stdout was redirected to.
			*/
			if(childcounter != pipeflag){
				dup2(fd[childcounter * 2], STDIN_FILENO);
				close(fd[(childcounter * 2) + 1]);
			}
			/* If this is the only process left
			   Then redirect the stdout from the last process into an output file. 
			*/
			if(childcounter == 0){
				if(rightcaretflag != 0){
					dup2(output_file,STDOUT_FILENO);
					close(output_file);
				}
			}
			break;
		}
		/* if equal amount of children as well as pipeflags, then execute the command in the first index at argv */
		if(childcounter == pipeflag){
			if((execvp(argv[0],argv)) < 0){
				fprintf(stderr, "%s: Command not found.\n", argv[0]);
				exit(9);
			}
		}
		else{
			if((execvp(argv[pipearray[(pipeflag-1) - childcounter]],&argv[pipearray[(pipeflag-1) - childcounter]])) < 0){
				fprintf(stderr, "%s: Command not found\n",argv[pipearray[1]]);
				exit(9);
			}
		}
		for(;;){
			parent = wait(NULL);
			if(parent == grandchild)
				break;
		}
	}
	/* Next methods are similar to main() in where the dev/null, andflag, and the parent id checking is made. */
	if(andflag != 0 && leftcaretflag == 0){
		int devnull;
		flags = O_RDONLY;
		if((devnull = open("/dev/null",flags)) < 0){
			fprintf(stderr, "Failed to open /dev/null.\n");
			exit(9);
		}
		dup2(devnull,STDIN_FILENO);
		close(devnull);
	}
	if(andflag != 0){
		printf("%s [%d]\n",*argv,child);
		andflag = 0;
	}
	else{
		for(;;){
			parent = wait(NULL);
			if(parent == child)
				break;
		}
	}
}
/* Called when pipeands are used instead of pipes. 
   Very similar to regular multipiping algorithm, but only difference is that stderr is also redirected from the write end of the pipes. 
*/
int pipeandpipes(){
	pid_t parent,child,grandchild;
	int fd[pipeflag * 2];
	int childcounter = 0;

	fflush(stdout);
	fflush(stderr);
	child = fork();
	if(child == 0){
		while(childcounter < pipeflag){
			pipe(fd + (childcounter*2));
			fflush(stdout);
			fflush(stderr);
			//grandchild is made when the child of the parent is forked, causing the vertical piping behavior. 
			grandchild = fork();
			if(grandchild == 0){
				childcounter++;
				//If only one pipe or last process, then redirect the input
				if(childcounter == pipeflag){
					if(leftcaretflag != 0){
						dup2(input_file, STDIN_FILENO);
						close(input_file);
					}
				}
				/* If the amount of children is less then amount of flags, then 
				   redirect the standard out to the proper position in the file descriptor array. If 2 children, then redirect to the next process. 
				*/
				if(childcounter <= pipeflag){
					dup2(fd[2 * childcounter -1],STDOUT_FILENO);
					dup2(fd[2 * childcounter -1],STDERR_FILENO);
					close(fd[2 * childcounter - 1]);
				}
				continue;
			}
			/*If not the last process
			  Redirect the stdin to the proper position of the file descriptor which is the position after the index where stdout was redirected to.
			*/
			if(childcounter != pipeflag){
				dup2(fd[childcounter * 2], STDIN_FILENO);
				close(fd[(childcounter * 2) + 1]);
			}
			/* If this is the only process left
			   Then redirect the stdout from the last process into an output file. 
			*/
			if(childcounter == 0){
				if(rightcaretflag != 0){
					dup2(output_file,STDOUT_FILENO);
					dup2(output_file,STDERR_FILENO);
					close(output_file);
				}
			}
			break;
		}
		/* if equal amount of children as well as pipeflags, then execute the command in the first index at argv */
		if(childcounter == pipeflag){
			if((execvp(argv[0],argv)) < 0){
				fprintf(stderr, "%s: Command not found.\n", argv[0]);
				exit(9);
			}
		}
		else{
			if((execvp(argv[pipeandarray[(pipeflag-1) - childcounter]],&argv[pipeandarray[(pipeflag-1) - childcounter]])) < 0){
				fprintf(stderr, "%s: Command not found\n",argv[pipeandarray[1]]);
				exit(9);
			}
		}
		for(;;){
			parent = wait(NULL);
			if(parent == grandchild)
				break;
		}
	}
	/* Next methods are similar to main() in where the dev/null, andflag, and the parent id checking is made. */
	if(andflag != 0 && leftcaretflag == 0){
		int devnull;
		flags = O_RDONLY;
		if((devnull = open("/dev/null",flags)) < 0){
			fprintf(stderr, "Failed to open /dev/null.\n");
			exit(9);
		}
		dup2(devnull,STDIN_FILENO);
		close(devnull);
	}
	if(andflag != 0){
		printf("%s [%d]\n",*argv,child);
		andflag = 0;
	}
	else{
		for(;;){
			parent = wait(NULL);
			if(parent == child)
				break;
		}
	}
}
void myhandler(int sig){};
