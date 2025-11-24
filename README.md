# SaC Image Formats

The project uses `cmake-common`.
Upon cloning this repo it is important initialize submodules:

```
git submodule update --init --recursive
```

Then build the library using `make`:

```
mkdir build
cd build
cmake -DTARGETS="<targets>" ..
make
```
