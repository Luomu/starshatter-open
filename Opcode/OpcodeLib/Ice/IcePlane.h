///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for planes.
 *	\file		IcePlane.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICEPLANE_H__
#define __ICEPLANE_H__

	#define PLANE_EPSILON		(1.0e-7f)

	class ICEMATHS_API IcePlane
	{
		public:
		//! Constructor
		inline_			IcePlane()															{												}
		//! Constructor from a normal and a distance
		inline_			IcePlane(float nx, float ny, float nz, float d)					{ Set(nx, ny, nz, d);							}
		//! Constructor from a point on the plane and a normal
		inline_			IcePlane(const IcePoint& p, const IcePoint& n)							{ Set(p, n);									}
		//! Constructor from three points
		inline_			IcePlane(const IcePoint& p0, const IcePoint& p1, const IcePoint& p2)		{ Set(p0, p1, p2);								}
		//! Constructor from a normal and a distance
		inline_			IcePlane(const IcePoint& _n, float _d)								{ n = _n; d = _d;								}
		//! Copy constructor
		inline_			IcePlane(const IcePlane& plane) : n(plane.n), d(plane.d)				{												}
		//! Destructor
		inline_			~IcePlane()														{												}

		inline_	IcePlane&	Zero()															{ n.Zero(); d = 0.0f;			return *this;	}
		inline_	IcePlane&	Set(float nx, float ny, float nz, float _d)						{ n.Set(nx, ny, nz); d = _d;	return *this;	}
		inline_	IcePlane&	Set(const IcePoint& p, const IcePoint& _n)							{ n = _n; d = - p | _n;			return *this;	}
				IcePlane&	Set(const IcePoint& p0, const IcePoint& p1, const IcePoint& p2);

		inline_	float	Distance(const IcePoint& p)			const						{ return (p | n) + d;							}
		inline_	bool	Belongs(const IcePoint& p)				const						{ return fabsf(Distance(p)) < PLANE_EPSILON;	}

		inline_	void	Normalize()
						{
							float Denom = 1.0f / n.Magnitude();
							n.x	*= Denom;
							n.y	*= Denom;
							n.z	*= Denom;
							d	*= Denom;
						}
		public:
		// Members
				IcePoint	n;		//!< The normal to the plane
				float	d;		//!< The distance from the origin

		// Cast operators
		inline_			operator IcePoint()					const						{ return n;										}
		inline_			operator HPoint()					const						{ return HPoint(n, d);							}

		// Arithmetic operators
		inline_	IcePlane	operator*(const Matrix4x4& m)		const
						{
							// Old code from Irion. Kept for reference.
							IcePlane Ret(*this);
							return Ret *= m;
						}

		inline_	IcePlane&	operator*=(const Matrix4x4& m)
						{
							// Old code from Irion. Kept for reference.
							IcePoint n2 = HPoint(n, 0.0f) * m;
							d = -((IcePoint) (HPoint( -d*n, 1.0f ) * m) | n2);
							n = n2;
							return *this;
						}
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Transforms a plane by a 4x4 matrix. Same as IcePlane * Matrix4x4 operator, but faster.
	 *	\param		transformed	[out] transformed plane
	 *	\param		plane		[in] source plane
	 *	\param		transform	[in] transform matrix
	 *	\warning	the plane normal must be unit-length
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline_	void TransformPlane(IcePlane& transformed, const IcePlane& plane, const Matrix4x4& transform)
	{
		// Rotate the normal using the rotation part of the 4x4 matrix
		transformed.n = plane.n * Matrix3x3(transform);

		// Compute new d
		transformed.d = plane.d - (IcePoint(transform.GetTrans())|transformed.n);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Transforms a plane by a 4x4 matrix. Same as IcePlane * Matrix4x4 operator, but faster.
	 *	\param		plane		[in/out] source plane (transformed on return)
	 *	\param		transform	[in] transform matrix
	 *	\warning	the plane normal must be unit-length
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline_	void TransformPlane(IcePlane& plane, const Matrix4x4& transform)
	{
		// Rotate the normal using the rotation part of the 4x4 matrix
		plane.n *= Matrix3x3(transform);

		// Compute new d
		plane.d -= IcePoint(transform.GetTrans())|plane.n;
	}

#endif // __ICEPLANE_H__
