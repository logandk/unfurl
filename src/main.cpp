#include <sstream>
#include <string>
#include <pugixml.hpp>
#include "windows_helpers.h"
#include "zip_helpers.h"
#include "splash_screen.h"

using namespace std;
using namespace pugi;

HINSTANCE inst_ = NULL;
HWND bkg_win_   = NULL;
HWND txt_win_   = NULL;

void show_splash(const string &app_name)
{
  RegisterWindowClass(inst_);

  HBITMAP bkg_img = LoadSplashImage();
  bkg_win_ = CreateSplashWindow(inst_);
  SetSplashImage(bkg_win_, bkg_img);

  string splash_text = "Upgrading " + app_name + ", please wait...";
  HFONT font = CreateFont(16, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, _T("Arial"));
  HBITMAP txt_img = CreateAlphaTextBitmap(splash_text.c_str(), font, RGB(255, 255, 255));
  txt_win_ = CreateSplashWindow(inst_);
  SetSplashImage(txt_win_, txt_img);
}

void hide_splash()
{
  if (bkg_win_ && txt_win_)
  {
    DestroyWindow(bkg_win_);
    DestroyWindow(txt_win_);
  }
}

void show_help(const string &name)
{
  stringstream s;

  s << "Usage: unfurl manifest [options]..." << endl << endl
    << "Deploy and launch the latest version of the application " << endl
    << "described in the supplied manifest file." << endl
    << "Any additional options will be forwarded to the " << endl
    << "target application." << endl << endl
    << "Example manifest file:" << endl << endl
    << "---" << endl << endl
    << "  <?xml version=\"1.0\" ?>" << endl
    << "  <unfurl>" << endl
    << "    <binary>myapp.exe</binary>" << endl
    << "    <repository>P:\\deploy</repository>" << endl
    << "    <identifier>myapp</identifier>" << endl
    << "    <current-version>1.0</current-version>" << endl
    << "  </unfurl>" << endl << endl
    << "---" << endl << endl
    << "http://github.com/logandk/unfurl" << endl;

  show_info("unfurl: usage", s.str());
}

string sanitize_argument(const string &arg)
{
  string s = arg;
  bool need_quotes = false;

  if (s.find(' ') != string::npos)
  {
    need_quotes = true;
  }

  if (s.find('"') != string::npos)
  {
    size_t pos = 0;
    while((pos = s.find("\"", pos)) != string::npos)
    {
       s.replace(pos, 1, "\\\"");
       pos += 2;
    }
    need_quotes = true;
  }

  if (need_quotes)
  {
    s = string("\"") + s + string("\"");
  }

  return s;
}

int main(int argc, char *argv[])
{
  try
  {
    if (argc < 2 || string(argv[1]) == "-h" || string(argv[1]) == "--help")
    {
      show_help(argv[0]);

      return 1;
    }

    string manifest_path = argv[1];

    // Case 1: Path is absolute
    if (!file_exists(manifest_path))
    {
      if (file_exists(join_path(current_path(), manifest_path)))
      {
        // Case 2: Path is relative to working directory with extension
        manifest_path = join_path(current_path(), manifest_path);
      }
      else if (file_exists(join_path(current_path(), manifest_path + ".xml")))
      {
        // Case 3: Path is relative to working directory without extension
        manifest_path = join_path(current_path(), manifest_path + ".xml");
      }
      else if (file_exists(join_path(application_path(), manifest_path)))
      {
        // Case 4: Path is relative to unfurl directory with extension
        manifest_path = join_path(application_path(), manifest_path);
      }
      else if (file_exists(join_path(application_path(), manifest_path + ".xml")))
      {
        // Case 5: Path is relative to unfurl directory without extension
        manifest_path = join_path(application_path(), manifest_path + ".xml");
      }
    }

    // Check if a manifest file was found
    if (!file_exists(manifest_path))
    {
      throw runtime_error("manifest file not found");
    }

    // Parse manifest file
    xml_document doc;
    xml_parse_result result = doc.load_file(manifest_path.c_str());

    if (!result)
    {
      throw runtime_error(result.description());
    }

    xml_node root = doc.child("unfurl");

    string binary          = root.child_value("binary");
    string repository      = root.child_value("repository");
    string identifier      = root.child_value("identifier");
    string current_version = root.child_value("current-version");

    // Check if installation is required
    bool install_required = true;

    string data_path    = join_path(appdata_path(), "unfurl");
    string app_path     = join_path(data_path, identifier);
    string version_path = join_path(app_path, current_version);
    string binary_path  = join_path(version_path, binary);

    if (file_exists(binary_path))
    {
      install_required = false;
    }

    // Unzip package to destination
    if (install_required)
    {
      show_splash(identifier);

      string package_name = identifier + "-" + current_version + ".zip";
      string package_path = join_path(repository, package_name);

      create_path(version_path);

      try
      {
        unzip(package_path, version_path);
      }
      catch (...)
      {
        delete_path(version_path);
        throw;
      }
    }

    // Launch application
    string command = binary_path;

    for (int i = 2; i < argc; i++)
    {
      command += " ";
      command += sanitize_argument(argv[i]);
    }

    return create_process(command, hide_splash);
  }
  catch (exception &e)
  {
    show_error("unfurl: error", e.what());
  }

  return 1;
}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE, LPSTR, INT)
{
  CoInitialize(NULL);

  inst_ = inst;
  int r = main(__argc, __argv);

  CoUninitialize();

  return r;
}

