# dt-annotate

A very simple auto-annotation tool for re-compiled device tree source files

Main benefit is to replace numeric phandle values by meaningful symbols and add lables. For that it is required that the device tree binary was re-compiles with the following command line:

`dtc -I dtb -O dts -o <output (*.dts) file name> -s -H epapr -@ -A <input (*.dtb) file name>`
