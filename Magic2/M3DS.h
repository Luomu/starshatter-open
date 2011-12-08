/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         M3DS.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Window class
*/

// parts copyright (c) 2001-2002 Lev Povalahev

#ifndef M3DS_H
#define M3DS_H

#include "List.h"

// +--------------------------------------------------------------------+

struct Chunk
{
   WORD  id;
   DWORD len;
   BYTE* start;

   List<Chunk> chunks;
};

// +--------------------------------------------------------------------+

struct LTriangle
{
   WORD a,b,c;
};

struct LTriangle2
{
   Point    vertices[3];
   Vec3     vertexNormals[3];
   LVector2 textureCoords[3];
   Vec3     faceNormal;
   DWORD     materialId;
};

struct TextureMap
{
   char     fname[256];
   float    strength;
   float    u_scale;
   float    v_scale;
   float    u_offset;
   float    v_offset;
   float    angle;
};

// +--------------------------------------------------------------------+

class LObject
{
public:
   LObject();
   virtual ~LObject();

   virtual const char*  GetName();
   virtual bool         IsObject(const const char* name);

protected:
    Text    name;
};

// +--------------------------------------------------------------------+

class LMaterial : public LObject
{
public:
   LMaterial();
   virtual ~LMaterial();

   DWORD       GetID();
   TextureMap& GetTextureMap1();
   TextureMap& GetTextureMap2();
   TextureMap& GetOpacityMap();
   TextureMap& GetSpecularMap();
   TextureMap& GetBumpMap();
   TextureMap& GetReflectionMap();
   ColorValue  GetAmbientColor();
   ColorValue  GetDiffuseColor();
   ColorValue  GetSpecularColor();
   float       GetShininess();
   float       GetTransparency();
   DWORD       GetShadingType();

   // this methods should not be used by the "user", they're used internally to fill the class
   // with valid data when reading from file. If you're about to add an importer for another format you'LL
   // have to use these methods

   void        SetID(DWORD value);
   void        SetAmbientColor(const ColorValue &color);
   void        SetDiffuseColor(const ColorValue &color);
   void        SetSpecularColor(const ColorValue &color);
   void        SetShininess(float value);
   void        SetTransparency(float value);
   void        SetShadingType(DWORD shading);

protected:
   int         id;
   TextureMap  texMap1;
   TextureMap  texMap2;
   TextureMap  opacMap;
   TextureMap  refTextureMap;
   TextureMap  bumpMap;
   TextureMap  specMap;
   ColorValue  ambient;
   ColorValue  diffuse;
   ColorValue  specular;
   float       shininess;
   float       transparency;
   DWORD       shading;
};

// +--------------------------------------------------------------------+

class LMesh : public LObject
{
public:
    LMesh();
    virtual ~LMesh();

    void Clear();

    DWORD GetVertexCount();
    void SetVertexArraySize(DWORD value);
    DWORD GetTriangleCount();
    void SetTriangleArraySize(DWORD value);

    // returns given vertex    
    const LVector4& GetVertex(DWORD index);
    // returns the given normal
    const LVector3& GetNormal(DWORD index);
    // returns the given texture coordinates vector
    const LVector2& GetUV(DWORD index);
    // returns the pointer to the array of tangents
    const LVector3& GetTangent(DWORD index);
    // returns the pointer to the array of binormals
    const LVector3& GetBinormal(DWORD index);
    // sets the vertex at a given index to "vec" - for internal use    
    void SetVertex(const LVector4 &vec, DWORD index);
    // sets the normal at a given index to "vec" - for internal use    
    void SetNormal(const LVector3 &vec, DWORD index);
    // sets the texture coordinates vector at a given index to "vec" - for internal use    
    void SetUV(const LVector2 &vec, DWORD index);
    // sets the tangent at a given index to "vec" - for internal use    
    void SetTangent(const LVector3 &vec, DWORD index);
    // sets the binormal at a given index to "vec" - for internal use    
    void SetBinormal(const LVector3 &vec, DWORD index);
    // returns the triangle with a given index
    const LTriangle& GetTriangle(DWORD index);
    // returns the triangle with a given index, see LTriangle2 structure description
    LTriangle2 GetTriangle2(DWORD index);
    // returns the mesh matrix, should be identity matrix after loading    
    LMatrix4 GetMatrix();
    // sets the mesh matrix to a given matrix - for internal use
    void SetMatrix(LMatrix4 m);
    // optimizises the mesh using a given optimization level
    void Optimize(LOptimizationLevel value);
    // sets an internal triangle structure with index "index" - for internal use only
    void SetTri(const LTri &tri, DWORD index);
    // returns the pointer to the internal triangle structure - for internal use only
    LTri& GetTri(DWORD index);
    // returns the material id with a given index for the mesh
    DWORD GetMaterial(DWORD index);
    // adds a material to the mesh and returns its index - for internal use
    DWORD AddMaterial(DWORD id);
    // returns the number of materials used in the mesh
    DWORD GetMaterialCount();
protected:
    // the vertices, normals, etc.
    List<LVector4> vertices;
    List<LVector3> normals;
    List<LVector3> binormals;
    List<LVector3> tangents;
    List<LVector2> uv;
    
    // triangles
    List<LTriangle> triangles;

    //used internally
    List<LTri> tris;

    // the transformation matrix.
    Matrix matrix;

    // the material ID array
    List<DWORD> materials;

    // calculates the normals, either using the smoothing groups information or not
    void CalcNormals(bool useSmoothingGroups);
    // calculates the texture(tangent) space for each vertex
    void CalcTextureSpace();
    // transforms the vertices by the mesh matrix
    void TransformVertices();
};

//------------------------------------------------

class LImporter
{
public:
    // the default constructor
    LImporter();
    // the destructor
    virtual ~LImporter();
    // reads the model from a file, must be overriden by the child classes
    virtual bool LoadFile(const char *filename) = 0;
    // returns the number of meshes in the scene
    DWORD GetMeshCount();
    // returns the number of lights in the scene
    DWORD GetLightCount();
    // returns the number of materials in the scene
    DWORD GetMaterialCount();
    // returns a pointer to a mesh
    LMesh& GetMesh(DWORD index);
    // returns a pointer to a light at a given index
    LLight& GetLight(DWORD index);
    // returns the pointer to the material
    LMaterial& GetMaterial(DWORD index);
    // returns the pointer to the material with a given name, or NULL if the material was not found
    LMaterial* FindMaterial(const Text &name);
    // returns the pointer to the mesh with a given name, or NULL if the mesh with such name 
    // is not present in the scene
    LMesh* FindMesh(const Text &name);
    // returns the pointer to the light with a given name, or NULL if not found
    LLight* FindLight(const Text &name);
    // sets the optimization level to a given value
    void SetOptimizationLevel(LOptimizationLevel value);
    // returns the current optimization level
    LOptimizationLevel GetOptimizationLevel();
protected:
    // the lights found in the scene
    List<LLight> lights;
    // triangular meshes
    List<LMesh> meshes;
    // the materials in the scene
    List<LMaterial> materials;
    // level of optimization to perform on the meshes
    LOptimizationLevel optLevel;
    // clears all data.
    virtual void Clear();
};
//------------------------------------------------

class L3DS : public LImporter
{
public:
    // the default contructor
    L3DS();
    // constructs the object and loads the file
    L3DS(const char *filename);
    // destructor
    virtual ~L3DS();
    // load 3ds file 
    virtual bool LoadFile(const char *filename);
protected:
    // used internally for reading
    char objName[100];
    // true if end of file is reached
    bool eof;
    // buffer for loading, used for speedup
    unsigned char *buffer;
    // the size of the buffer
    DWORD bufferSize;
    // the current cursor position in the buffer
    DWORD pos;

    // reads a short value from the buffer
    short ReadShort();
    // reads an int value from the buffer
    int ReadInt();
    // reads a char from the buffer
    char ReadChar();
    //reada a floatvalue from the buffer
    float ReadFloat();
    //reads an unsigned byte from the buffer
    byte ReadByte();
    //reads an asciiz string 
    int ReadASCIIZ(char *buf, int max_count);
    // seek wihtin the buffer
    void Seek(int offset, int origin);
    // returns the position of the cursor
    DWORD Pos();

    // read the chunk and return it.
    LChunk ReadChunk();
    // read until given chunk is found
    bool FindChunk(LChunk &target, const LChunk &parent);
    // skip to the end of chunk "chunk"
    void SkipChunk(const LChunk &chunk);
    // goes to the beginning of the data in teh given chunk
    void GotoChunk(const LChunk &chunk);

    // the function read the color chunk (any of the color chunks)
    ColorValue ReadColor(const LChunk &chunk);
    // the function that read the percentage chunk and returns a float from 0 to 1
    float ReadPercentage(const LChunk &chunk);
    // this is where 3ds file is being read
    bool Read3DS();
    // read a light chunk 
    void ReadLight(const LChunk &parent);
    // read a trimesh chunk
    void ReadMesh(const LChunk &parent);
    // reads the face list, face materials, smoothing groups... and fill rthe information into the mesh
    void ReadFaceList(const LChunk &chunk, LMesh &mesh);
    // reads the material
    void ReadMaterial(const LChunk &parent);
    // reads the map info and fills the given map with this information
    void ReadMap(const LChunk &chunk, TextureMap& map);
    // reads keyframer data of the OBJECT_NODE_TAG chunk
    void ReadKeyframeData(const LChunk &parent);
    // reads the keyheader structure from the current offset and returns the frame number
    long ReadKeyheader();
};

//---------------------------------------------------------

#endif