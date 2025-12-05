# NifMopp
Command line tool using the NifMopp library. Should be built for 32-bit Windows, and requires NifMopp.dll, which is included with Hexabit's NifSkope releases.

## Usage
    NifMopp.exe <FILENAME>

Reads shape data from FILENAME, generates MOPP code, and saves it overwriting FILENAME.

### Input file format
    uint32_t    0x4853454D; // "MESH"
    uint32_t    shapeCount; // number of sub-shapes, can be zero
    uint32_t    shapeVerts[shapeCount];
    uint32_t    vertCount;
    float       verts[vertCount * 3];
    uint32_t    triangleCount;
    uint16_t    triangles[triangleCount * 3];

### Output file format
    uint32_t    0x50504F4D; // "MOPP"
    float       origin[3]; // X, Y, Z
    float       scale;
    uint32_t    moppCodeLen;
    uint8_t     moppCode[moppCodeLen];
