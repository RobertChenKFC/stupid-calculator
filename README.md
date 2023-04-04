# Stupid Calculator
I haven't found a good name for this calculator, so it is currently called the
stupid calculator. It uses a modified version of the [nand2tetris](https://www.nand2tetris.org/)
software stack and CPU to implement a basic calculator.
## Overview
The calculator is written in the stupid language (.stu extension; need to come
up with a better name for this as well), compiled to the Jack programming
language of nand2tetris, and then further compiled to the Hack machine code to
run on the Hack CPU of nand2tetris. The following lists the software components
of this project:
* `compiler`: this is the compiler responsible to compile the stupid language
to the Jack programming language.
* `simulator`: the stupid language is designed to be quite similar to
Javascript, meaning that with little modification, stupid code can be executed
using a typical Javascript engine, and can thus allow you to easily debug your
stupid code using debuggers and `console.log`s.
* `emulator`: this is a full nand2tetris software stack that compiles the Jack
programming language to Hack machine code and runs the code on an emulated Hack
CPU. This allows you to test the calculator fully before depolying to real
hardware.
## Build Instructions
### `compiler`:
* Prerequisites: [GNU Make](https://www.gnu.org/software/make/),
[Bison](https://www.gnu.org/software/bison/), [Flex](https://github.com/westes/flex)
* Build: Simply run `make -j`.
### `simulator`:
* Prerequisites: [node](https://nodejs.org/en), [npm](https://www.npmjs.com/),
[Python](https://www.python.org/)
* Build: Install node modules using `npm ci`.
### `emulator`:
* Prerequisites: [emscripten](https://emscripten.org/), [CMake](https://cmake.org/),
[GNU Make](https://www.gnu.org/software/make/)
* Build:
1. Create `build` folder
```
mkdir build
```
2. Navigate to `build` folder
```
cd build
```
3. Run `cmake` using `emcmake`
```
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
```
4. Run `make` using `emmake`
```
emmake make -j
```
## Usage
### `compiler`
Provide a stupid code file named `${filename}.stu` to the binary `compiler`, and
a `${filename}.jack` will be generated containing the compiled Jack code.
### `simulator`
Provide the path of the stupid code to `test.py`, and a console interface of the
simulated calculator will be created.
### `emulator`
Run an HTTP server at the path `emulator/src/`, and use a browser to connect to
the server, then a GUI interface will be provided for both the compilation from
the Jack language to Hack machine code and the emulated calculator.
