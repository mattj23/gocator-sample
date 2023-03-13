# Gocator Sample

## Overview

This is an extremely simple example of using the C API on an LMI Gocator 2XXX laser line series scanner.  This code was used to gather sample data from a Gocator 2420.  The code is based on some of LMI's examples in the SDK.

If you're looking to use this as a starting point, edit the constants at the beginning of `main.cpp` to match with your
sensor and the number of total captures that you want to take.  When run, the resulting binary will save a file named `sample-##.data` in the current working directory.  That file will be a fairly un-embellished copy of the raw messages, in which the message header data is saved at the beginning and the points are represented with sixteen bit integer values and an eight bit color.

Serialization is done with `reinterpret_cast<char*>` so it's important to deserialize the files on the same architecture they were created on and convert them into a format that's more useful. 

The `gocator_convert` binary shows how to do that deserialization.  You'll probably want to do something more useful than printing the point x, y, z, color values to stdout.

## The LMI SDK on Linux

On Linux systems, the LMI SDK is a little confusing as to how to install it once compiled.

You will have to get their SDK code in a zip file from their website.  Once you do you can extract it and build it:

```bash
cd GO_SDK/Gocator

# Make sure to use whatever architecture your system is using
make -f GoSdk-Linux_X64.mk  
```

The built libraries will be in `GO_SDK/lib/linux_x64d` or whatever architecture you used.  You will need to copy both `libkApi.so` and `libGoSdk.so` to a library path, and the appropriate header folders to a system include directory.  I usually use `/usr/local`.

```bash
cd GO_SDK

cp lib/linux_x64d/libGoSdk.so /usr/local/lib/
cp lib/linux_x64d/libkApi.so /usr/local/lib/

# I don't know what the appropriate folders are for headers, but copying the source folders 
# seems to work fine.
cp -r Gocator/GoSdk/GoSdk /usr/local/include/
cp -r Platform/kApi/kApi /usr/local/include/
```

At that point you will be able to compile this project in that environment, provided you have `cmake` and all the other necessary build tools.  Usually I make a `.deb` or `.rpm` package to avoid having to do this multiple times for a single version of the SDK.
