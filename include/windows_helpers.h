#ifndef UNFURL_WINDOWS_HELPERS_H
#define UNFURL_WINDOWS_HELPERS_H

#include <windows.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <stdexcept>

bool file_exists(const std::string &path)
{
  DWORD attr = GetFileAttributes(path.c_str());

  return (attr != INVALID_FILE_ATTRIBUTES &&
          !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

bool dir_exists(const std::string &path)
{
  DWORD attr = GetFileAttributes(path.c_str());

  return (attr != INVALID_FILE_ATTRIBUTES &&
          (attr & FILE_ATTRIBUTE_DIRECTORY));
}

std::string current_path()
{
  char path[MAX_PATH] = {0};
  GetCurrentDirectory(MAX_PATH, path);
  return path;
}

std::string application_path()
{
  char path[MAX_PATH] = {0};
  GetModuleFileName(NULL, path, MAX_PATH);
  PathRemoveFileSpec(path);
  return path;
}

std::string appdata_path()
{
  char path[MAX_PATH] = {0};
  SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path);
  return path;
}

std::string join_path(const std::string &a, const std::string &b)
{
  char path[MAX_PATH] = {0};
  strcpy(path, a.c_str());
  PathAppend(path, b.c_str());
  if (dir_exists(path)) PathAddBackslash(path);
  return path;
}

std::string parent_path(const std::string &child_path)
{
  char path[MAX_PATH] = {0};
  strcpy(path, child_path.c_str());
  PathRemoveBackslash(path);
  PathRemoveFileSpec(path);
  return path;
}

void create_path(const std::string &path)
{
  if (!dir_exists(parent_path(path)))
  {
    create_path(parent_path(path));
  }

  if (!dir_exists(path))
  {
    CreateDirectory(path.c_str(), NULL);
  }
}

void delete_file(const std::string &path)
{
  DeleteFile(path.c_str());
}

void delete_dir(const std::string &path)
{
  RemoveDirectory(path.c_str());
}

void delete_path(const std::string &path)
{
  if (file_exists(path))
  {
    delete_file(path);
  }
  else
  {
    WIN32_FIND_DATA fd;
    HANDLE h;

    h = FindFirstFile(join_path(path, "*").c_str(), &fd);

    if (h == INVALID_HANDLE_VALUE)
    {
      throw std::runtime_error("delete_path: invalid path: " + path);
    }

    do
    {
      if (std::string(fd.cFileName) == "." || std::string(fd.cFileName) == "..")
      {
        continue;
      }

      if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        delete_path(join_path(path, fd.cFileName));
      }
      else
      {
        delete_file(join_path(path, fd.cFileName));
      }
    }
    while (FindNextFile(h, &fd) != 0);

    FindClose(h);

    delete_dir(path);
  }
}

unsigned long create_process(const std::string &command)
{
  DWORD exit_code;
  char path[MAX_PATH] = {0};

  strcpy(path, command.c_str());

  // Set up members of the PROCESS_INFORMATION structure.
  PROCESS_INFORMATION proc_info;
  ZeroMemory(&proc_info, sizeof(PROCESS_INFORMATION));

  // Set up members of the STARTUPINFO structure.
  // This structure specifies the STDIN and STDOUT handles for redirection.
  STARTUPINFO start_info;
  ZeroMemory(&start_info, sizeof(STARTUPINFO));
  start_info.cb          = sizeof(STARTUPINFO);
  start_info.hStdInput   = GetStdHandle(STD_INPUT_HANDLE);
  start_info.hStdOutput  = GetStdHandle(STD_OUTPUT_HANDLE);
  start_info.hStdError   = GetStdHandle(STD_ERROR_HANDLE);
  start_info.dwFlags    |= STARTF_USESTDHANDLES;

  // Create the child process.
  bool success = CreateProcess(NULL,
    path,        // command line
    NULL,        // process security attributes
    NULL,        // primary thread security attributes
    true,        // handles are inherited
    0,           // creation flags
    NULL,        // use parent's environment
    NULL,        // use parent's current directory
    &start_info, // STARTUPINFO pointer
    &proc_info); // receives PROCESS_INFORMATION

  // If an error occurs, exit the application.
  if (!success)
  {
    throw std::runtime_error("create_process: unable to create process: " + command);
  }

  // Successfully created the process.  Wait for it to finish.
  WaitForSingleObject(proc_info.hProcess, INFINITE);

  // Get the exit code.
  GetExitCodeProcess(proc_info.hProcess, &exit_code);

  // Close handles to the child process and its primary thread.
  CloseHandle(proc_info.hProcess);
  CloseHandle(proc_info.hThread);

  return exit_code;
}

void show_info(const std::string &title, const std::string &message)
{
  MessageBox(
    NULL,
    message.c_str(),
    title.c_str(),
    MB_ICONINFORMATION | MB_OK
  );
}

void show_error(const std::string &title, const std::string &message)
{
  MessageBox(
    NULL,
    message.c_str(),
    title.c_str(),
    MB_ICONERROR | MB_OK
  );
}

#endif

