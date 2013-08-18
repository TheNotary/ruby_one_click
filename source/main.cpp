#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <algorithm>
#include "const.h"
using namespace std;
#include "sharelib.cpp"

#if defined(__APPLE__) || defined (__linux__)
	#define RSI_UNIX
	#pragma message("Compiling under Unix system")
#elif defined(__CYGWIN__)
	#define RSI_WIN
	#pragma message("Compiling under Cygwin system")
#elif defined (_WIN32) //ON Windows 64, _WIN32 is also defined so we don't have to check _WIN64.
	#define RSI_WIN
	#pragma message("Compiling under Windows system")
#else
	#error "Unknown platform."
#endif

int main(int argc,char** argv)
{
	#if defined(DEBUG_INFO)
		freopen ("debuginfo.txt","a",stdout);
		cout << "==================================" << endl;
	#endif
	
	//STEP 0: CHECK TO SEE IF RUBY IS INSTALLED
	#if defined(RSI_WIN)
		if (check_installed_ruby()) return 0;
	#endif
	
	//STEP 1: READ APP FILE NAME TO GET RUBY VERSION
	string ruby_version = get_ruby_version(argv[0]);
	
	//STEP 2: GET TARGET INSTALLER URL & DEVKIT URL
	string rubyinstaller_url, rubydevkit_url;
	
	curl_get_installer_url(ruby_version,rubyinstaller_url,rubydevkit_url);
	string rubyinstaller_name = extract_installer_name(rubyinstaller_url);
	string rubyinstaller_fullpath = get_installer_fullpath(rubyinstaller_name);
	string rubydevkit_name = extract_installer_name(rubydevkit_url);
	string rubydevkit_fullpath = get_installer_fullpath(rubydevkit_name);
	
	//STEP 3 & 4: CHECK CACHE FOLDER & DOWNLOAD RUBY INSTALLER
	if (!create_folder(TEMP_FOLDER)) return 0;
	if (!is_existing(rubyinstaller_fullpath)) curl_get_rubyinstaller_exe(rubyinstaller_url,rubyinstaller_fullpath);
	
	//STEP 5: RUN THE RUBY INSTALLER
	#if defined(RSI_WIN)
		if (!install_ruby(rubyinstaller_fullpath)) return 0;
	#endif
	
	//STEP 6: CHECK CACHE & DOWNLOAD THE DEVKIT INSTALLER
	if (!is_existing(rubydevkit_fullpath)) curl_get_rubyinstaller_exe(rubydevkit_url,rubydevkit_fullpath);
	
	//STEP 7: EXTRACT & ACTIVATE DEVKIT
	#if defined(RSI_WIN)
		if (!create_folder(GEM_FOLDER)) return 0;
		if (!extract_DevKit(rubydevkit_fullpath)) return 0;
		if (!activate_DevKit()) return 0;
	#endif
	
	//IF IT'S MAC OR LINUX, JUST DISPLAY ERROR
	#if defined(RSI_UNIX)
		cout << "Files downloaded but cannot be run due to platform incompatabilities." << endl;
	#endif
	
	#if defined(DEBUG_INFO)
		fclose (stdout);
	#endif
	return 0;
}
