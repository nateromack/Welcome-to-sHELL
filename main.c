/*
Basic UNIX Shell. 
Uses RayLib for GUI components, license for RayLib posted below:

-- Copyright (c) 2020-2024 Jeffery Myers
--
--This software is provided "as-is", without any express or implied warranty. In no event 
--will the authors be held liable for any damages arising from the use of this software.

--Permission is granted to anyone to use this software for any purpose, including commercial 
--applications, and to alter it and redistribute it freely, subject to the following restrictions:

--  1. The origin of this software must not be misrepresented; you must not claim that you 
--  wrote the original software. If you use this software in a product, an acknowledgment 
--  in the product documentation would be appreciated but is not required.
--
--  2. Altered source versions must be plainly marked as such, and must not be misrepresented
--  as being the original software.
--
--  3. This notice may not be removed or altered from any source distribution.

*/

#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "resource_dir.h"	//utility header for SearchAndSetResourceDir

#define MAX_INPUT_CHARS		256

typedef struct {

	char* progname;
	int redirect[2];
	char* args[];

}cmd;

typedef struct {

	int commandCount;
	cmd* commands[];

}pipelineStruct;

cmd* parseCommand(char * str);

pipelineStruct* parsePipeline(char * str);
ssize_t promptInput(const char* prompt, char** line, size_t* len);
void freePipeline(int numberOfPipes, int(*pipes)[2]);
int execWrapper(cmd* command, int numberOfPipes, int (*pipes)[2], int write_fd);
pid_t runRedirect(cmd* command, int numberOfPipes, int (*pipes)[2], char* outputBuffer, size_t bufferSize);
char * shell(char *terminalInput, char * output);
char * submitCommand(char *command, char *output);
int sh_cd(char **args);
int sh_help(char **args);
int sh_exit(char **args);
int sh_num_builtins();
extern char *builtin_str[];
extern int (*builtin_func[]) (char **);

// Shell output to be drawn on the screen
	//char shellOutput[1024] = "CrazyShell> \0"; 

int main ()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	const int screenWidth = 1330;
    const int screenHeight = 768;

	// Create the window and OpenGL context
	InitWindow(screenWidth, screenHeight, "CrazyShell");

	// add null terminator at the end of the string
    char  staticShell[12] = "CrazyShell> ";
	char inputCommand[MAX_INPUT_CHARS + 1] = "\0";
    char shellOutput[4096] = "CrazyShell> \0";
	int letterCount = 0;

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	// make a textbox capable of receiving input
	Rectangle textBox = { 50, 700, 1230, 50 };
	bool mouseOnText = false;

	int framesCounter = 0;
	SetTargetFPS(60);

	

	// Load a texture from the resources directory
	//Texture freakyEmoji = LoadTexture("freakyEmoji.png");
	
	// game loop
	while (!WindowShouldClose())		// run the loop untill the user presses ESCAPE or presses the Close button on the window
	{
		// UPDATE SECTION
		//----------------------------------------------------------------------------------------------------------------------
		if (CheckCollisionPointRec(GetMousePosition(), textBox)) mouseOnText = true;
		else mouseOnText = false;

		if (mouseOnText)
        {
            // Set the window's cursor to the I-Beam
            SetMouseCursor(MOUSE_CURSOR_IBEAM);

            // Get char pressed (unicode character) on the queue
            int key = GetCharPressed();

            // Check if more characters have been pressed on the same frame
            while (key > 0)
            {
                // NOTE: Only allow keys in range [32..125]
                if ((key >= 32) && (key <= 125) && (letterCount < MAX_INPUT_CHARS))
                {
                    inputCommand[letterCount] = (char)key;
                    inputCommand[letterCount+1] = '\0'; // Add null terminator at the end of the string.
                    letterCount++;
                }

                key = GetCharPressed();  // Check next character in the queue
            }

            if (IsKeyPressed(KEY_BACKSPACE))
            {
                letterCount--;
                if (letterCount < 0) letterCount = 0;
                inputCommand[letterCount] = '\0';
            }

			if (IsKeyPressed(KEY_ENTER))
			{
				strcpy(shellOutput, submitCommand(inputCommand,shellOutput));
                

				letterCount = 0;
				inputCommand[letterCount] = '\0';
			}
        }
        else SetMouseCursor(MOUSE_CURSOR_DEFAULT);

        if (mouseOnText) framesCounter++;
        else framesCounter = 0;
		//----------------------------------------------------------------------------------------------------------------------

		// DRAWING SECTION
		//----------------------------------------------------------------------------------------------------------------------
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(BLACK);

		// draw some text using the default font
		DrawText("Welcome to CrazyShell", 550,20,20,RED);
        DrawText(TextFormat("%s%s",staticShell, shellOutput), 80,80,20,YELLOW);

		DrawRectangleRec(textBox, BLACK);
            if (mouseOnText) DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, RED);
            else DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, WHITE);

            DrawText(TextFormat("%s",staticShell), (int)textBox.x + 5, (int)textBox.y + 8, 40, RED);

			//DrawText(shellOutput, 550,300,20,RED);

			// draw text displaying characters input, not necessary
            //DrawText(TextFormat("INPUT CHARS: %i/%i", letterCount, MAX_INPUT_CHARS), 315, 250, 20, RED);

            if (mouseOnText)
            {
                if (letterCount < MAX_INPUT_CHARS)
                {
                    // Draw blinking underscore char
                    if (((framesCounter/20)%2) == 0) DrawText("_", (int)textBox.x + 8 + MeasureText(TextFormat("%s",staticShell, inputCommand), 40), (int)textBox.y + 12, 40, RED);
                }
                //else DrawText("Press BACKSPACE to delete chars...", 230, 300, 20, GRAY);
            }

		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
		//----------------------------------------------------------------------------------------------------------------------
	}


	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}

// Check if any key is pressed
// NOTE: We limit keys check to keys between 32 (KEY_SPACE) and 126
bool IsAnyKeyPressed()
{
    bool keyPressed = false;
    int key = GetKeyPressed();

    if ((key >= 32) && (key <= 126)) keyPressed = true;

    return keyPressed;
}

// Submit command to shell
char* submitCommand(char *command, char * output)
{
    return strcpy(output, shell(command, output));
	//shell(command);
    
}

char * shell(char *terminalInput, char * output){

    char * line = NULL;
    size_t len = 0;
    char outputBuffer[4096];

    // /*while(*/promptInput("Shell> ", &line, &len) > 0) //{
        pipelineStruct*  pipeline = parsePipeline(terminalInput);
        int numberOfPipes = pipeline->commandCount;
        int(*pipes)[2] = calloc (sizeof(int[2]), numberOfPipes);
        for(int i = 1; i < numberOfPipes; ++i) {
            pipe(pipes[i-1]);
            pipeline->commands[i]->redirect[STDIN_FILENO] = pipes[i-1][0];
            pipeline->commands[i]->redirect[STDOUT_FILENO] = pipes[i-1][1];
        }
        for(int i = 0; i < numberOfPipes; ++i) {
            runRedirect(pipeline -> commands[i], numberOfPipes, pipes, outputBuffer, sizeof(outputBuffer));
        }
        freePipeline(numberOfPipes, pipes);

        for(int i = 0; i < numberOfPipes; ++i) {
            wait(NULL);
        }
        fputs("\n",stderr);
        return strcpy(output, outputBuffer);
    //}
}

ssize_t promptInput(const char* prompt, char** line, size_t* len) {
        fputs(prompt, stderr);
        return getline(line, len, stdin);
    }

    void freePipeline(int numberOfPipes, int(*pipes)[2]){
        for (int i; i < numberOfPipes; ++i) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
    }
int execWrapper(cmd* command, int numberOfPipes, int (*pipes)[2], int write_fd) {
    int fd = -1;
    if (command->redirect[0] != -1) {
        dup2(command->redirect[0],STDIN_FILENO);
    }
    if (command->redirect[1] != -1) {
        dup2(command->redirect[1], STDOUT_FILENO);
    }
    else if (write_fd != -1) {
        dup2(write_fd, STDOUT_FILENO); // Redirect stdout to the write end of the new pipe }
    }
    freePipeline(numberOfPipes, pipes);
    return execvp(command->progname, command->args);
    
}
pid_t runRedirect(cmd* command, int numberOfPipes, int (*pipes)[2], char* outputBuffer, size_t bufferSize) {
    for (int i = 0; i < sh_num_builtins(); i++) {
       if (strcmp(command->progname, builtin_str[i]) == 0) {
         return (*builtin_func[i])(command->args);
         return 0;
       }
    }
    int capturePipe[2];
    if (pipe(capturePipe) == -1) {
        perror("pipe");
        exit(1);
    }

    pid_t child = fork();
    if(child){
        switch(child){
            case -1:
                fprintf(stderr,"Child");
                return -1;
            default:
                //Parent process: Read from the pip
                close(capturePipe[1]); //Close the write end in the parent
                ssize_t bytesRead = read(capturePipe[0], outputBuffer,bufferSize - 1);
                if (bytesRead >= 0) {
                    outputBuffer[bytesRead] = '\0'; //Null-terminate the string
                } else {
                    perror("read");
                }
                close(capturePipe[0]);
                return child;
        }
        }else{
            close(capturePipe[0]); //close the read end in the child
            execWrapper(command, numberOfPipes, pipes, capturePipe[1]);
            perror("Problem with command");
            return 0;
        }

    
}

char* nextToken(char**line){
    char * token;
    while((token=strsep(line," \t\n\r"))&&!*token);
    return token;
}

cmd* parseCommand(char * str){
    char * copy = strndup(str,1024);
    char* token;
    int i=0;
    cmd* ret = calloc(sizeof(cmd) + 1024 * sizeof(char), 1);

    while((token = nextToken(&copy))){
        ret->args[i++] = token;
    }
    ret->progname = ret->args[0];
    ret->redirect[0] = -1;
    ret->redirect[1] = -1;
    return ret;
}
pipelineStruct* parsePipeline(char * str){
    char * copy = strndup(str,1024);
    char* token;
    int numberCommands = 0;
    int i = 0;
    pipelineStruct* ret;
    for(char*current = copy; *current; current++){
        if(*current == '|'){
            numberCommands++;
        }
    }
    numberCommands++;
    ret = calloc(sizeof(pipelineStruct) + numberCommands * sizeof(cmd), 1);
    ret->commandCount = numberCommands;

    while((token = strsep(&copy, "|"))){
        ret->commands[i++] = parseCommand(token);
    }
    return ret;
}

char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &sh_cd,
  &sh_help,
  &sh_exit
};


int sh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}


int sh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "sh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

int sh_help(char **args)
{
  char list[4096] = "";
  //strcat(list, "Type program names and arguments, and hit enter.\nThe following are built in:\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (int i = 0; i < sh_num_builtins(); i++) {
    //strcat(list, builtin_str[i]);
    printf("  %s\n", builtin_str[i]);
  }

  return 1;
}

int sh_exit(char **args)
{
  exit(0);
  return 0;
}

