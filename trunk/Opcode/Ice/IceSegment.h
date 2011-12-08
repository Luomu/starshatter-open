///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for segments.
 *	\file		IceSegment.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICESEGMENT_H__
#define __ICESEGMENT_H__

	class ICEMATHS_API Segment
	{
		public:
		//! Constructor
		inline_					Segment()															{}
		//! Constructor
		inline_					Segment(const IcePoint& p0, const IcePoint& p1) : mP0(p0), mP1(p1)		{}
		//! Copy constructor
		inline_					Segment(const Segment& seg) : mP0(seg.mP0), mP1(seg.mP1)			{}
		//! Destructor
		inline_					~Segment()															{}

		inline_	const	IcePoint&	GetOrigin()						const	{ return mP0;						}
		inline_			IcePoint	ComputeDirection()				const	{ return mP1 - mP0;					}
		inline_			void	ComputeDirection(IcePoint& dir)	const	{ dir = mP1 - mP0;					}
		inline_			float	ComputeLength()					const	{ return mP1.Distance(mP0);			}
		inline_			float	ComputeSquareLength()			const	{ return mP1.SquareDistance(mP0);	}

		inline_			void	SetOriginDirection(const IcePoint& origin, const IcePoint& direction)
								{
									mP0 = mP1 = origin;
									mP1 += direction;
								}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Computes a IcePoint on the segment
		 *	\param		pt	[out] IcePoint on segment
		 *	\param		t	[in] IcePoint's parameter [t=0 => pt = mP0, t=1 => pt = mP1]
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_			void	ComputePoint(IcePoint& pt, float t)	const	{	pt = mP0 + t * (mP1 - mP0);		}

						float	SquareDistance(const IcePoint& IcePoint, float* t=null)	const;
		inline_			float	Distance(const IcePoint& IcePoint, float* t=null)			const			{ return sqrtf(SquareDistance(IcePoint, t));	}

						IcePoint	mP0;		//!< Start of segment
						IcePoint	mP1;		//!< End of segment
	};

#endif // __ICESEGMENT_H__
