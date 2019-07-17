# Silva

## Requirements

Silva is using instead of the arduino editor. For me the benefits is that I can use external editors, like clion, have
autocompletion etc and being able to use a command line tool for compiling and uploading the code.

How to install the PlatformIO tools are described in their documentation [here](https://docs.platformio.org/en/latest/installation.html), but for a Mac Homebrew user it might be easiest with:

`brew install platformio`

After that is done, ensure that you have installed the libraries that this project demands, like FASTLed etc:

`make install`

Now you can try to compile the source code with 

`make` 

to ensure everything is working. If this is the first time running platformio it will also download required tools to 
work with arduino boards etc. 