#define CURL_STATICLIB
#include <curl/curl.h>
#include <curl/easy.h>
#if defined(__APPLE__) || defined (__linux__)
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <cerrno>
#elif defined(__CYGWIN__) || defined (_WIN32)
	#include <windows.h>
#endif

/*
 * This function get character of ruby version and check if it is legal
 * accept only number, dot and x
 */
bool ruby_version_string_legal_char(char c)
{
	if (isdigit(c)) return true;
	if (c == '.') return true;
	if (c == 'x') return true;
	return false;
}

/*
 * This function get ruby version from our filename
 */
string get_ruby_version(char* argv)
{
	//get the current process filename
	string process_filename = argv;
	
	//Get the last 4 characters of the filename, to compare if it is .exe or not
	//for example, if it is ruby-oneclick-1.9.3.exe, we will extract from - to last .
	//else it is ruby-oneclick-1.9.3 (linux), we will extract from - to end of string.
	
	string process_filename_last4char = process_filename.substr(process_filename.length() - 4,4);
	std::transform(process_filename_last4char.begin(), process_filename_last4char.end(),process_filename_last4char.begin(), ::toupper);
	
	int process_filename_version_end = (process_filename_last4char == ".EXE") ? process_filename.length() - 4 : process_filename.length();
	int process_filename_version_start = process_filename.find_last_of("-");
	
	string ruby_version;
	if (process_filename_version_start != process_filename.npos)
	{
		ruby_version = process_filename.substr(process_filename_version_start + 1,process_filename_version_end - process_filename_version_start);
		//found, we have a legal file name, but there are still more, if it's actually a version ?
		//or it's just ruby-oneclick.exe ???
		//we will check the whole string, and will only accept number, dot, and 'x'
		for (int i = 0;i < ruby_version.length();++i)
		{
			if (!ruby_version_string_legal_char(ruby_version[i]))
			{
				ruby_version = DEFAULT_RUBY_VERSION;
				break;
			}
		}
		//now our string is good. But there is 1 problem left
		//The funny thing is, on mingw32, our last name will be like 1.9.3. (with a trail dot) because of substr. We will have the remove that trait dot
		if (ruby_version[ruby_version.length() - 1] == '.') ruby_version.erase(ruby_version.length() - 1);
	}
	else
	{
		//not found '-' ? it will be default version
		ruby_version = DEFAULT_RUBY_VERSION;
	}
	#if defined(DEBUG_INFO)
		cout << "Ruby version: " << ruby_version << endl;
	#endif
	return ruby_version;
}

/*
 * This function get installer url, and then extract the filename
 * we will find the last character of "/", and ".exe", between that will be our filename
 */
string extract_installer_name(const string &installer_url)
{
	int installername_start = installer_url.find_last_of("/") + 1;
	string rubyinstaller_name = installer_url.substr(installername_start,installer_url.find_last_of(".") + 4 - installername_start);
	return rubyinstaller_name;
}

/*
 * This function test if file is existing
 */
bool is_existing(const string &name)
{
    if (FILE *file = fopen(name.c_str(), "r"))
	{
        fclose(file);
		#if defined(DEBUG_INFO)
			cout << "Already downloaded file. Skipping " << endl;
		#endif
        return true;
    }
	else
	{
        return false;
    }
}

/*
 * This function test if ruby is installed
 */
bool check_installed_ruby()
{
	const char *command = "ruby -v";
	#if defined(__CYGWIN__) || defined (_WIN32)
		bool result;
		DWORD exitCode; 
		STARTUPINFO si; //[in]
		PROCESS_INFORMATION pi; //[out]
		ZeroMemory( &pi, sizeof(pi) );
		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
		
		// Start the child process. 
		result = CreateProcess( NULL,(char*)command,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi );

		// Wait until child process exits.
		WaitForSingleObject( pi.hProcess, INFINITE );
		
		// Close process and thread handles. 
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
		
		if (result)
		{
			MessageBox(0, "Ruby is already installed, uninstall first.", "Error", MB_OK | MB_ICONERROR | MB_SETFOREGROUND );
			return true;
		}
	#endif
	return false;
}

#if defined(__CYGWIN__) || defined (_WIN32)
/**
 * This function read Windows registry
* @param location The location of the registry key. For example "Software\\Bethesda Softworks\\Morrowind"
* @param name the name of the registry key, for example "Installed Path"
* @return the value of the key or an empty string if an error occured.
*/
std::string getRegKey(const std::string& location, const std::string& name){
    HKEY key;
    TCHAR value[1024]; 
    DWORD bufLen = 1024*sizeof(TCHAR);
    long ret;
    ret = RegOpenKeyExA(HKEY_LOCAL_MACHINE, location.c_str(), 0, KEY_QUERY_VALUE, &key);
    if( ret != ERROR_SUCCESS ){
        return std::string();
    }
    ret = RegQueryValueExA(key, name.c_str(), 0, 0, (LPBYTE) value, &bufLen);
    RegCloseKey(key);
    if ( (ret != ERROR_SUCCESS) || (bufLen > 1024*sizeof(TCHAR)) ){
        return std::string();
    }
    std::string stringValue = std::string(value, (size_t)bufLen - 1);
    size_t i = stringValue.length();
    while( i > 0 && stringValue[i-1] == '\0' ){
        --i;
    }
    return stringValue.substr(0,i); 
}
#endif

/*
 * This function install ruby
 */
bool install_ruby(const string &path)
{
	#if defined(__CYGWIN__) || defined (_WIN32)
		string fullpath;
		fullpath = path + " " + RUBY_INSTALLER_SWITCHES;
		
		bool result;
		DWORD exitCode; 
		STARTUPINFO si; //[in]
		PROCESS_INFORMATION pi; //[out]
		ZeroMemory( &pi, sizeof(pi) );
		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
		
		// Start the child process. 
		if( !CreateProcess( NULL,(char*)fullpath.c_str(),NULL,NULL,FALSE,0,NULL,NULL,&si,&pi ))
		{
			#if defined(DEBUG_INFO)
				cout << "Failed to install DevKit." << endl;
			#endif
			return false;
		}
		
		// Wait until child process exits.
		WaitForSingleObject( pi.hProcess, INFINITE );
		
		result = GetExitCodeProcess(pi.hProcess, &exitCode);
		
		// Close process and thread handles. 
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
		
		if (!result)
		{
			#if defined(DEBUG_INFO)
				cout << "Failed to get result of installing DevKit." << endl;
			#endif
			return false;
		}
		
		if (exitCode != 0)
		{
			#if defined(DEBUG_INFO)
				cout << "There were errors in installing DevKit." << endl;
			#endif
			return false;
		}
		
		string newpath = getRegKey("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment","path");
		newpath = "PATH=" + newpath;
		putenv((char*)newpath.c_str());
		
		#if defined(DEBUG_INFO)
			cout << "Updated:" << newpath << endl;
			cout << "Check:" << getenv("PATH") << endl;
		#endif
	#endif
	return true;
}

/*
 * This function assist curl to write content to file
 */
size_t write_data_to_file(void *contents, size_t size, size_t nmemb, FILE *stream) {
    size_t written;
    written = fwrite(contents, size, nmemb, stream);
    return written;
}

/*
 * This function assist curl to write content to stream (memory)
 */
size_t write_data_to_memstring(void *contents, size_t size, size_t nmemb, void *stream) {
    ((std::string*)stream)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

/*
 * This function get the webpage of ruby and extract download urls
 */
bool curl_get_installer_url(const string &ruby_version,string &rubyinstaller_url,string &rubydevkit_url)
{
	CURL *curl;
	CURLcode res;
	string readBuffer;

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, DOWNLOAD_URL);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_to_memstring);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}
	
	int rubyinstaller_url_detect = readBuffer.find("rubyinstaller-" + ruby_version);
	if (rubyinstaller_url_detect == readBuffer.npos)
	{
		#if defined(DEBUG_INFO)
			cout << "Not found ruby version " + ruby_version << endl;
		#endif
		return false;
	}
	int rubyinstaller_url_last = readBuffer.find_first_of("\"",rubyinstaller_url_detect); //a href="http://abc.com/.../rubyinstaller-a.b.c-pxyz-exe?direct" <-- last character is "
	int rubyinstaller_url_first = readBuffer.find_last_of("\"",rubyinstaller_url_detect) + 1; //it can be http or https, we don't know, better get the " character also
	rubyinstaller_url = readBuffer.substr(rubyinstaller_url_first,rubyinstaller_url_last - rubyinstaller_url_first);	
	
	#if defined(DEBUG_INFO)
		cout << "Detected Installer url: " << rubyinstaller_url << endl;
	#endif
	
	string devkit_prefix = (ruby_version[0] == '1') ? DEVKIT_PREFIX_VERSION1 : DEVKIT_PREFIX_VERSION2;
	
	int rubydevkit_url_detect = readBuffer.find(devkit_prefix);
	if (rubydevkit_url_detect == readBuffer.npos)
	{
		#if defined(DEBUG_INFO)
			cout << "Not found devkit for ruby version " + ruby_version + " with prefix " + devkit_prefix << endl;
		#endif
		return false;
	}
	int rubydevkit_url_last = readBuffer.find_first_of("\"",rubydevkit_url_detect);
	int rubydevkit_url_first = readBuffer.find_last_of("\"",rubydevkit_url_detect) + 1;
	rubydevkit_url = readBuffer.substr(rubydevkit_url_first,rubydevkit_url_last - rubydevkit_url_first);	
	
	#if defined(DEBUG_INFO)
		cout << "Detected DevKit url: " << rubydevkit_url << endl;
	#endif
	return true;
}

/*
 * This function download the file
 */
void curl_get_rubyinstaller_exe(const string &url,const string &fullpath)
{
	CURL *curl;
	FILE *fp;
	CURLcode res;
	
	#if defined(DEBUG_INFO)
		cout << "Downloading to file: " << fullpath << endl;
	#endif
	
	curl = curl_easy_init();
	if (curl) {
		fp = fopen(fullpath.c_str(),"wb");
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_to_file);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		fclose(fp);
	}
	
	#if defined(DEBUG_INFO)
		cout << "Done..." << endl;
	#endif
}

/*
 * This function creates temp folder, and only one, assuming that all of the decendent folder is existing
 */
bool create_single_folder(const string &folder_path)
{
	#if defined(__APPLE__) || defined (__linux__)
		if ((mkdir(folder_path.c_str(),0777) < 0) && (EEXIST != errno))
	#elif defined(__CYGWIN__) || defined (_WIN32)
		if (!(CreateDirectory(folder_path.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()))
	#endif
			{
				#if defined(DEBUG_INFO)
					cout << "Failed to create directory: " << folder_path << endl;
				#endif
				return false;
			}
		return true;
}

/*
 * This function creates temp folder recursively from the directory tree.
 */
bool create_folder(const string &folder_path)
{
	#if defined(__APPLE__) || defined (__linux__)
		//we convert ~ to $HOME, because only the shell knows ~ is $HOME.
		string folder_path_process = folder_path;
		if (folder_path_process[0] == '~') folder_path_process.replace(0,1,getenv("HOME"));
		char sep_char = '/';
	#elif defined(__CYGWIN__) || defined (_WIN32)
		string folder_path_process = folder_path;
		char sep_char = '\\';
	#endif
		size_t ifound = 0;
		string temp_folder;
		while (ifound != folder_path_process.npos)
		{
			ifound = folder_path_process.find(sep_char,ifound + 1);
			temp_folder = folder_path_process.substr(0,ifound);
			#if defined(__CYGWIN__) || defined (_WIN32)
				if ((ifound == 2) && (folder_path_process[1] == ':')) continue;
			#endif
			if (!create_single_folder(temp_folder)) return false;
		}
	return true;
}

/*
 * This function create fullpath of the installer
 */
string get_installer_fullpath(const string &installer_name)
{
	string fullpath;
	#if defined(__APPLE__) || defined (__linux__)
		fullpath = UNIX_TEMP_FOLDER + ("/" + installer_name);
		if (fullpath[0] == '~') fullpath.replace(0,1,getenv("HOME")); //we convert ~ to $HOME, because only the shell knows ~ is $HOME.
	#elif defined(__CYGWIN__) || defined (_WIN32)
		fullpath = WINDOWS_TEMP_FOLDER + ("\\" + installer_name);
	#endif
	return fullpath;
}

/*
 * This function extract DevKit
 */
bool extract_DevKit(const string &path)
{
	#if defined(__CYGWIN__) || defined (_WIN32)
	string finalpath;
	finalpath = path + " -y -o\"" + GEM_FOLDER + "\"";
	
	bool result;
	DWORD exitCode; 
	STARTUPINFO si; //[in]
    PROCESS_INFORMATION pi; //[out]
	ZeroMemory( &pi, sizeof(pi) );
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW; //this one will force Windows use the following flag
	si.wShowWindow = SW_HIDE; //this one act as "ShowWindow(hWnd,SW_HIDE)". So we are hiding it happily.
	
    // Start the child process. 
    if( !CreateProcess( NULL,(char*)finalpath.c_str(),NULL,NULL,FALSE,0,NULL,NULL,&si,&pi ))
    {
		#if defined(DEBUG_INFO)
			cout << "Failed to extract DevKit." << endl;
		#endif
		return false;
    }
	
	// Wait until child process exits.
    WaitForSingleObject( pi.hProcess, INFINITE );
	
	result = GetExitCodeProcess(pi.hProcess, &exitCode);
	
    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
	
	if (!result)
	{
		#if defined(DEBUG_INFO)
			cout << "Failed to get result of extracting DevKit." << endl;
		#endif
		return false;
	}
	
	if (exitCode != 0)
	{
		#if defined(DEBUG_INFO)
			cout << "There were errors in extracting DevKit." << endl;
		#endif
		return false;
	}
	
	#endif
	return true;
}

/*
 * This function activate DevKit
 */
bool activate_DevKit()
{
	#if defined(__CYGWIN__) || defined (_WIN32)
		string temp = "ruby dk.rb init";
		bool result;
		DWORD exitCode; 
		STARTUPINFO si; //[in]
		PROCESS_INFORMATION pi; //[out]
		ZeroMemory( &pi, sizeof(pi) );
		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
		
		// Start the child process. 
		if( !CreateProcess( NULL,(char*)temp.c_str(),NULL,NULL,FALSE,0,NULL,GEM_FOLDER,&si,&pi ))
		{
			#if defined(DEBUG_INFO)
				cout << "Failed to init ruby DevKit." << endl;
			#endif
			return false;
		}
		
		// Wait until child process exits.
		WaitForSingleObject( pi.hProcess, INFINITE );
		
		result = GetExitCodeProcess(pi.hProcess, &exitCode);
		
		// Close process and thread handles. 
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
		
		if (!result)
		{
			#if defined(DEBUG_INFO)
				cout << "Failed to get result of initiation ruby DevKit." << endl;
			#endif
			return false;
		}
		
		if (exitCode != 0)
		{
			#if defined(DEBUG_INFO)
				cout << "There were errors in initiation ruby DevKit." << endl;
			#endif
			return false;
		}
		
		temp = "ruby dk.rb install";
		ZeroMemory( &pi, sizeof(pi) );
		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
		
		// Start the child process. 
		if( !CreateProcess( NULL,(char*)temp.c_str(),NULL,NULL,FALSE,0,NULL,GEM_FOLDER,&si,&pi ))
		{
			#if defined(DEBUG_INFO)
				cout << "Failed to install ruby DevKit." << endl;
			#endif
			return false;
		}
		
		// Wait until child process exits.
		WaitForSingleObject( pi.hProcess, INFINITE );
		
		result = GetExitCodeProcess(pi.hProcess, &exitCode);
		
		// Close process and thread handles. 
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
		
		if (!result)
		{
			#if defined(DEBUG_INFO)
				cout << "Failed to get result of install ruby DevKit." << endl;
			#endif
			return false;
		}
		
		if (exitCode != 0)
		{
			#if defined(DEBUG_INFO)
				cout << "There were errors in install ruby DevKit." << endl;
			#endif
			return false;
		}
	#endif
	return true;
}
