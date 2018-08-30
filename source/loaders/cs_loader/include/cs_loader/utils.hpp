/*
*	Loader Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A plugin for loading net code at run-time into a process.
*
*/

#include <cstdlib>
#include <set>
#include <string>
#include <cstring>

/* TODO: Remove this limitation */

#if !defined(__GNUC__) || __GNUC__ < 5 || ( __GNUC__ == 5 && __GNUC_MINOR__ < 3)
  #error THIS SOFTWARE CURRENTLY BUILDS ONLY ON GCC 5.3 OR NEWER!
#endif

#include <experimental/filesystem>
namespace SCCH_fs = std::experimental::filesystem;

void AddFilesFromDirectoryToTpaList( std::string directory, std::string& tpaList ) {

  for ( auto& dirent : SCCH_fs::directory_iterator(directory) ) {
    std::string path = dirent.path();

    if ( ! path.compare(path.length() - 4, 4, ".dll") ) {
      tpaList.append(path + ":");
    }
  }

}
