# Expr-IR
An Expression Tree to LLVM-IR Example.

## Demo
![](https://raw.githubusercontent.com/farzonl/expr-ir/master/demo.svg?sanitize=true)

## Why does this exist?
A tutorial should be simple. If your like me the last time a tutorial on how to use llvm was simple was the ![2.6 tutorial](lhttp://releases.llvm.org/2.6/docs/tutorial/JITTutorial1.html) So this is an attempt to get back to something simple while updating for llvm 8.0.1.

## Other modern resources (2019)
- https://satyendrabanjare.com/plt/2019/03/10/llvm/

## Build instructions Mac
```
brew install llvm
echo 'export PATH="/usr/local/opt/llvm/bin:$PATH"' >> ~/.bash_profile
./build.sh
```

Further reading on setting up your build:
- https://embeddedartistry.com/blog/2017/2/20/installing-clangllvm-on-osx

## Run instructions
```
./expr-ir ab+cde+**
```
