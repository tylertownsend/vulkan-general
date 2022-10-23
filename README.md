# vulkan-general

# Dependencies
## STB Image
```
sudo apt install libstb-dev
```

# SPD Log
```
sudo apt install libspdlog-dev
```

## GLFW
```
sudo apt install libglfw3
sudo apt install libglfw3-dev
```

## GLEW
```
sudo apt-get install libglew-dev
```
## Debug:
valgrind --tool=memcheck --leak-check=full --track-origins=yes ./build/sandbox/sandbox

The code was producing an error:
```
sysmalloc: Assertion `(old_top == initial_top (av) && old_size == 0) unique_ptr
```
This came from adding unique_ptrs for the Window class and WindowController as members.

So we were getting this:

```
Invalid write of size 8 ==353438== at 0x4A90D3C: std::unique_ptr
```
on the new members

Using valgrind we got the error stack:

But what fixed was a clean build:
https://stackoverflow.com/a/65109373

We rm -rf build  and rebuilt from clean state.