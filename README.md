# Gocator Sample

This is an extremely simple example of using the C API on an LMI Gocator 2XXX laser line series scanner.  This code was used to gather sample data from a Gocator 2420.  The code is based on some of LMI's examples in the SDK.

If you're looking to use this as a starting point, edit the constants at the beginning of `main.cpp` to match with your
sensor and the number of total captures that you want to take.  When run, the resulting binary will save a file named `sample-##.data` in the current working directory.  That file will be a fairly un-embellished copy of the raw messages, in which the message header data is saved at the beginning and the points are represented with sixteen bit integer values and an eight bit color.

Serialization is done with `reinterpret_cast<char*>` so it's important to deserialize the files on the same architecture they were created on and convert them into a format that's more useful. 
