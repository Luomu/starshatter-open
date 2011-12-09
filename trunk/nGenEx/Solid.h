/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Solid.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Classes for rendering solid meshes of polygons
*/

#ifndef Solid_h
#define Solid_h

#include "Polygon.h"
#include "Graphic.h"
#include "Video.h"
#include "List.h"

// +--------------------------------------------------------------------+

class Solid;
class Model;
class ModelFile;
class Surface;
class Segment;
class Shadow;
class Light;

class OPCODE_data;   // for collision detection

// +--------------------------------------------------------------------+

class Solid : public Graphic
{
public:
	static const char* TYPENAME() { return "Solid"; }

	enum { NAMELEN = 64 };

	static bool    IsCollisionEnabled();
	static void    EnableCollision(bool enable);

	Solid();
	virtual ~Solid();

	// operations
	virtual void   Render(Video* video, DWORD flags);
	virtual void   SelectDetail(Projector* p);
	virtual void   ProjectScreenRect(Projector* p);
	virtual void   Update();

	// accessors / mutators
	Model*         GetModel()     const { return model;         }
	void           GetAllTextures(List<Bitmap>& textures);

	virtual bool   IsDynamic()    const;
	virtual void   SetDynamic(bool d);
	virtual void   SetLuminous(bool l);
	virtual void   SetOrientation(const Matrix& o);
	virtual void   SetOrientation(const Solid& match);
	const Matrix&  Orientation()  const { return orientation;   }
	float          Roll()         const { return roll;          }
	float          Pitch()        const { return pitch;         }
	float          Yaw()          const { return yaw;           }
	virtual bool   IsSolid()      const { return true;          }

	// stencil shadows
	virtual void   CreateShadows(int nlights=1);
	virtual void   UpdateShadows(List<Light>& lights);
	List<Shadow>&  GetShadows()         { return shadows;       }

	bool           Load(const char* mag_file, double scale=1.0);
	bool           Load(ModelFile*  loader,   double scale=1.0);
	void           UseModel(Model* model);
	void           ClearModel();
	bool           Rescale(double scale);

	// collision detection
	virtual int    CollidesWith(Graphic& o);
	virtual int    CheckRayIntersection(Point pt, Point vpn, double len, Point& ipt,
	bool treat_translucent_polys_as_solid=true);
	virtual Poly*  GetIntersectionPoly() const { return intersection_poly; }

	// buffer management
	virtual void   DeletePrivateData();
	virtual void   InvalidateSurfaceData();
	virtual void   InvalidateSegmentData();

protected:
	Model*         model;
	bool           own_model;

	float          roll, pitch, yaw;
	Matrix         orientation;
	Poly*          intersection_poly;

	List<Shadow>   shadows;
};

// +--------------------------------------------------------------------+

class Model
{
	friend class Solid;
	friend class ModelFile;

public:
	static const char* TYPENAME() { return "Model"; }

	enum  {  MAX_VERTS = 64000, MAX_POLYS = 16000 };

	Model();
	Model(const Model& m);
	~Model();

	Model& operator = (const Model& m);
	int operator == (const Model& that) const { return this == &that; }

	bool           Load(const char* mag_file, double scale=1.0);
	bool           Load(ModelFile*  loader,   double scale=1.0);

	const char*    Name()         const { return name;             }
	int            NumVerts()     const { return nverts;           }
	int            NumSurfaces()  const { return surfaces.size();  }
	int            NumMaterials() const { return materials.size(); }
	int            NumPolys()     const { return npolys;           }
	int            NumSegments()  const;
	double         Radius()       const { return radius;           }
	bool           IsDynamic()    const { return dynamic;          }
	void           SetDynamic(bool d)   { dynamic = d;             }
	bool           IsLuminous()   const { return luminous;         }
	void           SetLuminous(bool l)  { luminous = l;            }

	List<Surface>&    GetSurfaces()     { return surfaces;         }
	List<Material>&   GetMaterials()    { return materials;        }
	const Material*   FindMaterial(const char* mtl_name) const;
	const Material*   ReplaceMaterial(const Material* mtl);
	void              GetAllTextures(List<Bitmap>& textures);

	Poly*          AddPolys(int nsurf, int npolys, int nverts);
	void           ExplodeMesh();
	void           OptimizeMesh();
	void           OptimizeMaterials();
	void           ScaleBy(double factor);

	void           Normalize();
	void           SelectPolys(List<Poly>&, Material* mtl);
	void           SelectPolys(List<Poly>&, Vec3 loc);

	void           AddSurface(Surface* s);
	void           ComputeTangents();

	// buffer management
	void           DeletePrivateData();

private:
	bool           LoadMag5(BYTE* block, int len, double scale);
	bool           LoadMag6(BYTE* block, int len, double scale);

	char           name[Solid::NAMELEN];
	List<Surface>  surfaces;
	List<Material> materials;
	int            nverts;
	int            npolys;
	float          radius;
	float          extents[6];
	bool           luminous;
	bool           dynamic;
};

// +--------------------------------------------------------------------+

class Surface
{
	friend class Solid;
	friend class Model;

public:
	static const char* TYPENAME() { return "Surface"; }

	enum { HIDDEN=1, LOCKED=2, SIMPLE=4, MAX_VERTS=64000, MAX_POLYS=16000 };

	Surface();
	~Surface();

	int operator == (const Surface& s) const { return this == &s; }

	const char*    Name()            const { return name;                                }
	int            NumVerts()        const { return vertex_set ? vertex_set->nverts : 0; }
	int            NumSegments()     const { return segments.size();                     }
	int            NumPolys()        const { return npolys;                              }
	int            NumIndices()      const { return nindices;                            }
	bool           IsHidden()        const { return state & HIDDEN ? true : false;       }
	bool           IsLocked()        const { return state & LOCKED ? true : false;       }
	bool           IsSimplified()    const { return state & SIMPLE ? true : false;       }

	Model*         GetModel()        const { return model;         }
	List<Segment>& GetSegments()           { return segments;      }
	const Point&   GetOffset()       const { return offset;        }
	const Matrix&  GetOrientation()  const { return orientation;   }
	double         Radius()          const { return radius;        }
	VertexSet*     GetVertexSet()    const { return vertex_set;    }
	Vec3*          GetVLoc()         const { return vloc;          }
	Poly*          GetPolys()        const { return polys;         }

	void           SetName(const char* n);
	void           SetHidden(bool b);
	void           SetLocked(bool b);
	void           SetSimplified(bool b);

	void           CreateVerts(int nverts);
	void           CreatePolys(int npolys);
	void           AddIndices(int n)       { nindices += n;        }
	Poly*          AddPolys(int npolys, int nverts);

	VideoPrivateData* GetVideoPrivateData()   const { return video_data;    }
	void              SetVideoPrivateData(VideoPrivateData* vpd)
	{ video_data = vpd; }

	void           ScaleBy(double factor);

	void           BuildHull();
	void           Normalize();
	void           SelectPolys(List<Poly>&, Material* mtl);
	void           SelectPolys(List<Poly>&, Vec3 loc);

	void           InitializeCollisionHull();
	void           ComputeTangents();
	void           CalcGradients(Poly& p, Vec3& tangent, Vec3& binormal);

	void           Copy(Surface& s, Model* m);
	void           OptimizeMesh();
	void           ExplodeMesh();

private:
	char           name[Solid::NAMELEN];
	Model*         model;
	VertexSet*     vertex_set; // for rendering
	Vec3*          vloc;       // for shadow hull
	float          radius;
	int            nhull;
	int            npolys;
	int            nindices;
	int            state;
	Poly*          polys;
	List<Segment>  segments;

	Point          offset;
	Matrix         orientation;

public:
	OPCODE_data*   opcode;

private:
	VideoPrivateData* video_data;
};

// +--------------------------------------------------------------------+

class Segment
{
public:
	static const char* TYPENAME() { return "Segment"; }

	Segment();
	Segment(int n, Poly* p, Material* mtl, Model* mod=0);
	~Segment();

	bool        IsSolid()         const { return material ? material->IsSolid()      : true;  }
	bool        IsTranslucent()   const { return material ? material->IsTranslucent(): false; }
	bool        IsGlowing()       const { return material ? material->IsGlowing()    : false; }

	VideoPrivateData* GetVideoPrivateData()   const { return video_data;    }
	void              SetVideoPrivateData(VideoPrivateData* vpd)
	{ video_data = vpd; }

	int         npolys;
	Poly*       polys;
	Material*   material;
	Model*      model;
	VideoPrivateData* video_data;
};

// +--------------------------------------------------------------------+

class ModelFile
{
public:
	ModelFile(const char* fname);
	virtual ~ModelFile();

	virtual bool   Load(Model* m, double scale=1.0);
	virtual bool   Save(Model* m);

protected:
	char        filename[256];
	Model*      model;

	// internal accessors:
	char*       pname;
	int*        pnverts;
	int*        pnpolys;
	float*      pradius;
};

// +--------------------------------------------------------------------+

#endif Solid_h

