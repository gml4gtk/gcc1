# gcc1
This is a re-compilation of GNU GCC compiler version 1.21 from 1988

From the ChangeLog:  
~~~  
Sun May  1 07:20:34 1988  Richard Stallman  (rms at frosted-flakes.ai.mit.edu)  
  
	* Version 1.21 released.  
  
	* expr.c (expand_call): Merge duplicate code for structure_value_addr.  
  
	* emit-rtl.c (restore_reg_data): Delete error check; it's legit for  
	a pseudo reg to appear nowhere in the rtl.  
  
~~~  
  
In the directory orig is the original gcc-1.21 version  
In the directory src1 is the initial re-compiled version to be improved  
In the directory misc is the C grammar as html page of gcc-1.21 and other gcc versions  
In the directory bison is the oldest known GNU Bison parser generator from 1988  
This GNU Bison version from 1988 can be re-compiled without issues in 2021  
  
Re-compiling can be fun and this can be extended using GCC docs as a guide about gcc basics  
  
See also this blog article: "Building and using GCC 0.9 aka the first public version"  
https://virtuallyfun.com/wordpress/2016/12/01/building-using-gcc-0-90-aka-first-public-version/  
  
This also can be used as a protoype for another cpu or system or even generate wasm assembly  
And it is more usable to re-compile older GCC version adding new features  
then to write the yet-another-crappy-compiler as almost other programmers do  
  
Todo:  
To improve, fix more issues and bugs, and a autoconf version  
Make example how to add some cpu and / or generate wasm assembly  
Generate compiler graph data to be used with gml4gtk graph viewer  
For debug generate documented assembly and / wasm assembly  
Create a front-end usable for debug  
For more features simply look at gcc-1.42 and / or gcc-2.95 sources  
  
![screenshot](gnufsf.png)  
  
