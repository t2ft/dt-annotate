# dt-annotate

A very simplistic re-annotation tool for re-compiled device tree source files.
## General usage
Main benefit is to replace numeric phandle values by meaningful symbols and add lables. For that it is required that the device tree binary was re-compiles with the following command line:

`dtc -I dtb -O dts -o <output (*.dts) file name> -s -H epapr -@ -A <input (*.dtb) file name>`

The program is invoked like this:

`dt-annotate <input> [<output>]`

if no \<output\> file is given, the input file name with added extention ".annotated" is used.

## Usage with Windows:
Since this is a Qt5 program the correct environment has to be set-up first. A batch file might be helpful for invocation (assumes dt-annotation is in the %PATH%):
```
call C:\Qt\5.15.2\msvc2015_64\bin\qtenv2.bat
dt-annotate.exe %1
```
If you save this batch-file to a directory that is included in the %PATH% and has the name `dt-annotate_explorer.bat` an integration with the Windows Explorer right-click menu is possible by importing a file with this content to the Windows Registry:
```
Windows Registry Editor Version 5.00

[HKEY_CLASSES_ROOT\*\shell\dt-annotate]
@="dt-&annotate"

[HKEY_CLASSES_ROOT\*\shell\dt-annotate\command]
@="\"dt-annotate_explorer.bat\" \"%1\""
```
