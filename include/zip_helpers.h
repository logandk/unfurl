#ifndef UNFURL_ZIP_HELPERS_H
#define UNFURL_ZIP_HELPERS_H

#include <unzip.h>
#include <iowin32.h>
#include <string>
#include <stdexcept>
#include "windows_helpers.h"

#define UNZIP_BUFFER_SIZE 8192

void unzip(const std::string &archive, const std::string &destination)
{
  // Open the zip file
  unzFile zipfile = unzOpen64(archive.c_str());

  if (zipfile == NULL)
  {
    throw std::runtime_error("unzOpen64: package not found: " + archive);
  }

  // Get info about the zip file
  unz_global_info64 global_info;

  if (unzGetGlobalInfo64(zipfile, &global_info) != UNZ_OK)
  {
    unzClose(zipfile);
    throw std::runtime_error("unzGetGlobalInfo64: package corrupt: " + archive);
  }

  // Buffer to hold data read from the zip file.
  char read_buffer[UNZIP_BUFFER_SIZE];

  // Loop to extract all files
  uLong i;

  for (i = 0; i < global_info.number_entry; ++i)
  {
    // Get info about current file.
    unz_file_info64 file_info;
    char filename[MAX_PATH];

    if (unzGetCurrentFileInfo64(zipfile, &file_info, filename, MAX_PATH, NULL, 0, NULL, 0) != UNZ_OK)
    {
      unzClose(zipfile);
      throw std::runtime_error("unzGetCurrentFileInfo64: package corrupt: " + archive);
    }

    // Check if this entry is a directory or file.
    const size_t filename_length = strlen(filename);

    if (filename[filename_length - 1] == '\\' || filename[filename_length - 1] == '/')
    {
      // Entry is a directory, so create it.
      create_path(join_path(destination, filename));
    }
    else
    {
      // Entry is a file, so extract it.
      if (unzOpenCurrentFile(zipfile) != UNZ_OK)
      {
        unzClose(zipfile);
        throw std::runtime_error("unzOpenCurrentFile: cannot open file: " +
          std::string(filename));
      }

      // Open a file to write out the data.
      FILE *out = fopen(join_path(destination, filename).c_str(), "wb");

      if (out == NULL)
      {
        unzCloseCurrentFile(zipfile);
        unzClose(zipfile);
        throw std::runtime_error("fopen: cannot open output file: " +
          join_path(destination, filename));
      }

      int error = UNZ_OK;

      do
      {
        error = unzReadCurrentFile(zipfile, read_buffer, UNZIP_BUFFER_SIZE);

        if (error < 0)
        {
          fclose(out);
          unzCloseCurrentFile(zipfile);
          unzClose(zipfile);
          throw std::runtime_error("unzReadCurrentFile: error while reading file: " +
            std::string(filename));
        }

        // Write data to file.
        if (error > 0)
        {
          if (fwrite(read_buffer, error, 1, out) != 1)
          {
            fclose(out);
            unzCloseCurrentFile(zipfile);
            unzClose(zipfile);
            throw std::runtime_error("fwrite: error while writing to file: " +
              join_path(destination, filename));
          }
        }
      }
      while (error > 0);

      fclose(out);
    }

    unzCloseCurrentFile(zipfile);

    // Go the the next entry listed in the zip file.
    if ((i + 1) < global_info.number_entry)
    {
      if (unzGoToNextFile(zipfile) != UNZ_OK)
      {
        unzClose(zipfile);
        throw std::runtime_error("unzGoToNextFile: error while reading next file");
      }
    }
  }

  unzClose(zipfile);
}

#endif

