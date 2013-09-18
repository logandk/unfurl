# unfurl #

Simple filesystem-based desktop application deployment for Windows

# Introduction #

**unfurl** is a simple desktop application deployment tool for Microsoft
Windows.

The main objective is to create a simple and robust alternative to systems such
as [ClickOnce][1]. Inspired by [nlaunch][2], the objective is to create an
application/environment/language-agnostic deployment tool that does not require
sophisticated infrastructure.

# How it works #

1. **unfurl** is started with the manifest file as the first argument. Any
   additional arguments will be passed along to the target application.
   Examples:
   * `unfurl.exe myapp`
   * `unfurl.exe myapp.xml`
   * `unfurl.exe apps\myapp`
   * `unfurl.exe P:\deploy\myapp.xml`
2. It finds the manifest file based on the first argument:
  * If the argument is an absolute or relative path, it is used
  * Search the working directory for the manifest file, adding `.xml` if needed
  * Search the application directory for the manifest file, adding `.xml` if
    needed
3. The manifest file is parsed.
4. It checks the local `APPDATA` folder for existence of the latest version of
   the target application binary.
5. If a local binary was not found, the latest application package is found in
   `repository`. The package naming convention is
   `[identifier]-[current-version].zip`. E.g. `myapp-1.0.zip`. The package is
   extracted to the local `APPDATA` folder.
6. The application specified by `binary` is launched, passing along any
   additional command line arguments.

If an error occurs during unpackaging, the local copy is removed from
`APPDATA`.

# Manifest file #

```xml
<?xml version="1.0" ?>
<unfurl>
  <binary>myapp.exe</binary>
  <repository>P:\deploy</repository>
  <identifier>myapp</identifier>
  <current-version>1.0</current-version>
</unfurl>
```
# TODO #

* Tests
* Package dependencies (e.g. a Qt package containing all DLL's)

# License #

This code is copyright 2013 Logan Raarup, and is released under the revised BSD License.

For more information, see [`LICENSE`](http://github.com/logandk/unfurl/blob/master/LICENSE).


[1]: http://en.wikipedia.org/wiki/ClickOnce
[2]: https://code.google.com/p/nlaunch
