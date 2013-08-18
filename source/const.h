#define DEFAULT_RUBY_VERSION "2.0.0"

#define WINDOWS_TEMP_FOLDER "C:\\temp\\ruby-installer"
#define UNIX_TEMP_FOLDER "~/.ruby_installer"
#define WINDOWS_GEM_FOLDER "C:\\devkit"

#define RUBY_INSTALLER_SWITCHES "/verysilent /lang=en /tasks=\"addtk,modpath\""
#define DOWNLOAD_URL "http://rubyinstaller.org/downloads/"

#define DEVKIT_PREFIX_VERSION1 "DevKit-tdm"
#define DEVKIT_PREFIX_VERSION2 "DevKit-mingw"

#if defined(__APPLE__) || defined (__linux__)
	#define TEMP_FOLDER UNIX_TEMP_FOLDER
	#define GEM_FOLDER ""
#elif defined(__CYGWIN__) || defined (_WIN32)
	#define TEMP_FOLDER WINDOWS_TEMP_FOLDER
	#define GEM_FOLDER WINDOWS_GEM_FOLDER
#endif
