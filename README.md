Credits
====================
* Chau Nguyen


About
====================
Do you ever find yourself on a windows machine and thinking, "Jeesh... I wish I could just really quickly have ruby installed on this damned thing with out having to go through a bunch of installers..." Well, now you can!  Just double click the .exe generated from this project and it will run in the background, performing the following...  

*  Downloads Ruby_installer from rubyinstaller.org (thanks for the great work guys) to C:/temp/ruby_installer
*  Downloads Devkit from rubyinstaller.org to C:/temp/ruby_installer
*  Installs Ruby_installer silently.  
*  Installs the devkit for the ruby installer silently
*  Exits

All of this happens completely silently, so just open up the task manager (ctrl+shift+esc) and switch to the processes tab and look for ruby_installer*, and when that process goes away, it's finished and you can test ruby on the cmd prompt.  

    c:\> ruby -v
    ruby 2.0.0p247 (2013-06-27) [i386-mingw32]

If you'd like to install a different version, you just need to rename the file to be the version of ruby you'd like to install.  

    c:\> rename ruby-oneclick-2.0.0.exe ruby-oneclick-1.9.3.exe

That's some baller meta action, am I right?  



Contributing & Developing
===================
This app was written in C++ and compiles via MingW32 on windows XP.  It has a make file so once you've set up your development environment accordingly, you only need to run one simple command to compile your own .exe file.  

    c:\> cd project_folder
    c:\> mingw32-make
    g++ -static-libgcc -static-libstdc++ -mwindows main.cpp -static -lcurl -lz -lwldap32 -lws2_32 -o ruby-oneclick-2.0.0.exe
    main.cpp:18:50: note: #pragma message: Compiling under Windows system

From there you will have a new program called ruby-oneclick-2.0.0.exe which you can double click to install ruby on your system with, and it will include any custom hacks that you patched into the source code (FUN!).  

I told you it's written in C++, but it was done very, very well by a student named Chau Nguyen, and so you will find the code in source/main.cpp extremely easy to follow.  Most of the heavy lifting is conducted by functions residing in source/sharelib.cpp which is also pretty easy to follow.   

There are no unit test...  If you would like to contribute some, feel free (hopefully they will fit right into the makefile?).  Otherwise, we're just being careful with what we code.  


Setting Up the Dev OS (Win32)
=============================
So install virtual box on your computer, and install windows 32 so it can run concurrently with your current OS.  You can usually find a backup windows XP installation disk at thepiratebay if you meet the licensing requirements.  


Setting Up MinGW32 on Windows XP
================================
Get to http://www.mingw.org and click the "Download Installer" button (it's quite hidden at the moment, in the upper right corner with out any color to attract attention).  

Install the app, and make sure you check off the C and C++ compilers as well as the MinGW development tool options when you are prompeted by the installer.  (check the report in documentation/ for screen shots).  

Then add 'C:\MinGW\bin' to your system path so you can type `mingw32-make` at the command prompt without needing to specify it's absolute path (see documentation if you don't recall how to do that exactly).  


Install/ Drag & Drop Dependencies
=================================

Once MinGW32 is installed on your system (probably to 'C:\MinGW') you get to drop in some dependencies into it's include and lib folders (CURL is the only dependencie, it's used for DLing files).  For emergencies, we've included the curl zip files which contain the includes and libs needed to compile this project.  
It's recomended that you download CURL without SSL from http://curl.haxx.se/latest.cgi?curl=win32-devel for win32.  
Once you have a zip file named something like...  "libcurl-7.17.1-win32-nossl.zip" and one like "curl-7.31.0-devel-mingw32.zip" you should be good to go.  These zip files should have lib and include files.  Copy their contents over to c:\MinGW32\lib and c:\MinGW32\include and the project will compile.  


From here, you'll be able to change directory into the source folder and run the below command to output a working .exe file.  

    C:\project_folder\source> mingw32-make


