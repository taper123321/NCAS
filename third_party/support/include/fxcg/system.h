#ifdef __cplusplus
extern "C" {
#endif

// This adds in syscalls for interfacing with the OS

void SetAutoPowerOffTime(int duration); // in minutes
void SetBacklightDuration(char duration); // in half-minutes

int GetAutoPowerOffTime();
char GetBacklightDuration();
void PowerOff(int displayLogo);
void Restart();
void SpecialMatrixcodeProcessing(int*col, int*row);
void TestMode(int);
void*GetStackPtr(void);

void SetSetupSetting(unsigned int SystemParameterNo, unsigned char SystemParameterValue);
unsigned char GetSetupSetting(unsigned int SystemParameterNo);

int Timer_Install(int InternalTimerID, void (*hander)(void), int elapse);
int Timer_Deinstall(int InternalTimerID);
int Timer_Start(int InternalTimerID);
int Timer_Stop(int InternalTimerID);

void TakeScreenshot(void);
void TakeScreenshot2(void); //seems to be the same as the one above

void DisplayMainMenu(void);

//Hold program execution:
void OS_InnerWait_ms(int);

void CMT_Delay_100micros(int); //does CMT stand for Composable Memory Transactions? Couldn't find documentation on this
void CMT_Delay_micros(int); //   nor on this (gbl08ma)

void SetQuitHandler(void (*)()); // sets callback to be run when user exits through the main menu from one app to another. eActivity uses this in the "Save file?" dialog

void Alpha_SetData( char VarName, void* Src );
void Alpha_GetData( char VarName, void* Dest );


int GetMainBatteryVoltage(int one);
int CLIP_Store( unsigned char*buffer, int length ); // stores buffer of length length in the system clipboard.
int MB_ElementCount(char* buf); // like strlen but for the graphical length of multibyte strings
#ifdef __cplusplus
}
#endif
