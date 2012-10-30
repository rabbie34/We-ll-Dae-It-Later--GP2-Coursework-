//Are we on the windows platform?
#ifdef WIN32
//if so include the header for windows applications
#include <Windows.h>
#endif

#include "Win32Window.h"
#include "GameApplication.h"

//Check to see if we are on windows
#ifdef WIN32
int WINAPI WinMain(HINSTANCE hInstance, 
					HINSTANCE hPrevInstance, 
					LPSTR lpCmdLine,
					int nCmdShow)
#else
//we are on some other platform
int main(int argc, char **argv)
#endif
{
	//entry point of the application, allocates a pointer to the game application class and checks to see if it can be initialized.
	//If it can then the tun function is callled. If not then the pointer is deleted. Pointed also deleted at end of game loop.
	CGameApplication*pApp=new CGameApplication();

	if(!pApp->init())
	{
		if(pApp)
		{
			delete pApp;
			pApp=NULL;
			return 1;
		}
	}

	pApp->run();

	if(pApp)
	{
		delete pApp;
		pApp=NULL;
	}
	return 0;
}

