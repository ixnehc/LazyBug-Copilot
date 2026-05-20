#include "stdh.h"
#include "spatialtester.h"

#include <assert.h>


BOOL SpacialTester::Equals(SpacialTester &other)
{
	if (type!=other.type)
		return FALSE;
	switch(type)
	{
		case None:
			return TRUE;
		case Box:
			return aabb==other.aabb;
		case Rect:
			return rc==other.rc;
		case Line:
			return line==other.line;
		case Sphere:
			return sph==other.sph;
		case Frustum:
			return vol==other.vol;
	}
	return FALSE;
}


SpacialTester::Result SpacialTester::Test(i_math::aabbox3df &aabb0)
{
	switch(type)
	{
		case Box:
		{
			if (!aabb.intersectsWithBox(aabb0))
				return NoTouch;
			if (aabb.containsBox(aabb0))
				return Contain;
			return Intersect;
		}
		case Rect:
		{
			if (!(rc.Left()<=aabb0.MaxEdge.x)&&(rc.Top()<=aabb0.MaxEdge.z)&&
				(rc.Right()>=aabb0.MinEdge.x)&&(rc.Bottom()>=aabb0.MinEdge.z))
				return NoTouch;

			if (!(rc.Left()<=aabb0.MinEdge.x)&&(rc.Top()<=aabb0.MinEdge.z)&&
				(rc.Right()>=aabb0.MaxEdge.x)&&(rc.Bottom()>=aabb0.MaxEdge.z))
				return Contain;

			return Intersect;
		}
		case Line:
		{
			if (aabb0.intersectsWithLine(line))
				return Intersect;
			return NoTouch;
		}
		case Frustum:
		{
			i_math::EIntersectionRelation3D r=vol.classifyAABB(aabb0);
			switch(r)
			{
				case ISREL3D_FRONT:
					return NoTouch;
				case ISREL3D_BACK:
					return Contain;
				default: //ISREL3D_CLIPPED:
					return Intersect;
			}
		}
		case Sphere:
		{
			i_math::vector3df center=aabb0.getCenter();
			i_math::vector3df dir=center-sph.center;
			if (sph.radius*sph.radius>dir.getLengthSQ())
				return Intersect;
			dir.normalize();
			i_math::vector3df v=sph.center+dir*sph.radius;
			if (aabb0.isPointInside(v))
				return Intersect;
			return NoTouch;
		}
		default:
			assert(FALSE);
	}

	return NoTouch;
}

SpacialTester::Result SpacialTester::Test(i_math::vector3df &v)
{
	switch(type)
	{
		case Box:
		{
			if (aabb.isPointInside(v))
				return Contain;
			return NoTouch;
		}
	case Rect:
		{
			if ((rc.Left()<=v.x)&&(rc.Top()<=v.z)&&
				(rc.Right()>=v.x)&&(rc.Bottom()>=v.z))
				return Contain;
			return NoTouch;
		}
	case Line:
		{
			assert(FALSE);//Not implemented yet
			return NoTouch;
		}
	case Frustum:
		{
			assert(FALSE);//Not implemented yet
			return NoTouch;
		}
	case Sphere:
		{
			if ((v-sph.center).getLengthSQ()>sph.radius*sph.radius)
				return NoTouch;
			return Contain;
		}
	}

	return NoTouch;
}

SpacialTester::Result SpacialTester::Test(SpacialTester &other)
{
	if (other.type==SpacialTester::Box)
		return Test(other.aabb);
	assert(FALSE);
	return NoTouch;
}

BOOL SpacialTester::Translate(i_math::vector3df &pos)
{
	switch(type)
	{
		case Box:
		{
			aabb.MinEdge+=pos;
			aabb.MaxEdge+=pos;
			return TRUE;
		}
		case Rect:
		{
			rc+=i_math::pos2df(pos.x,pos.z);
			return TRUE;
		}
		case Line:
		{
			line.start+=pos;
			line.end+=pos;
			return TRUE;
		}
		case Frustum:
		{
			vol+=pos;
			return TRUE;
		}
		case Sphere:
		{
			sph.center+=pos;
			return TRUE;
		}
	}

	return FALSE;
}


BOOL SpacialTester::GetAABB(i_math::aabbox3df &aabbRet)
{
	switch(type)
	{
	case Box:
		{
			aabbRet=aabb;
			return TRUE;
		}
	case Rect:
		{
			return FALSE;
		}
	case Line:
		{
			aabbRet.reset(line.start);
			aabbRet.addInternalPoint(line.end);
			return TRUE;
		}
	case Frustum:
			return FALSE;//this should be a little tricky
	case Sphere:
		{
			sph.toAABB(aabbRet);
			return TRUE;
		}
	}

	return FALSE;
}
//star
SpacialTester::Result SpacialTester::Test(i_math::vector3df &pos,float radius)
{
	switch(type)
	{
	case Line:
		{
			i_math::vector3df projpos = line.getClosestPoint(pos);
			float dist = (float)projpos.getDistanceFrom(pos);
			if(dist<=radius)
				return Intersect;
			else
				return NoTouch;
		}
	default:
		assert(FALSE);
	}
	return NoTouch;
}
SpacialTester::Result SpacialTester::Test(i_math::matrix43f &mat,i_math::aabbox3df &aabb)
{
	i_math::matrix43f matInverse;
	matInverse = mat;
	matInverse.makeInverse();

	switch(type)
	{
	case Line:
		{
			i_math::line3df lineInverse;
			matInverse.transformVect(line.start,lineInverse.start);
			matInverse.transformVect(line.end,lineInverse.end);			
			
			bool ret = aabb.intersectsWithLine(lineInverse);

			if(ret)
				return Intersect;
			else
				return NoTouch;
		}
	default:
		assert(FALSE);
	}
	return NoTouch;
}

SpacialTester::Result SpacialTester::Test(i_math::triangle3df &tri)
{
	switch(type)
	{
	case Line:
		{
			i_math::vector3df intersectPos;
			bool bIntersect = tri.getIntersectionWithLimitedLine(line,intersectPos);
			if(bIntersect) 
				return Intersect;
			else
				return NoTouch;
		}
	default:
		assert(FALSE);
	}

	return NoTouch;
}


