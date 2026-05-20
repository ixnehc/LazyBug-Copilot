/********************************************************************
	created:	2023/4/30
*********************************************************************/
#include "stdh.h"

#include "animbase.h" 

#include "math/circle.h"

//////////////////////////////////////////////////////////////////////////
//AnimEventZone

AnimTick AnimEventZone::GetDur()
{
	if (keysFan.size()>0)
		return keysFan[keysFan.size()-1].t-keysFan[0].t;

	return 0;
}

// ?????????????
// ?? keysFan ???,???????????;???? 0
AnimTick AnimEventZone::GetStart()
{
	if (keysFan.size()>0)
		return keysFan[0].t;

	return 0;
}

AnimTick AnimEventZone::GetEnd()
{
	if (keysFan.size()>0)
		return keysFan[keysFan.size()-1].t;

	return 0;
}




BOOL AnimEventZone::IsIn(AnimTick t)
{
	if (tp==Fan)
	{
		if (keysFan.size()>0)
		{
			if ((t>=keysFan[0].t)&&(t<=keysFan[keysFan.size()-1].t))
				return TRUE;
		}
	}
	return FALSE;
}

BOOL AnimEventZone::CalcKeyFan(AnimTick t,KeyFan &k)
{
	if (tp==Fan)
	{
		if (keysFan.size()>0)
		{
			if (t<=keysFan[0].t)
			{
				k=keysFan[0];
				return TRUE;
			}
			if (t>=keysFan[keysFan.size()-1].t)
			{
				k=keysFan[keysFan.size()-1];
				return TRUE;
			}

			for (int i=0;i<keysFan.size()-1;i++)
			{
				KeyFan &kCur=keysFan[i];
				KeyFan &kNext=keysFan[i+1];

				if ((t>=kCur.t)&&(t<=kNext.t))
				{
					float ratio;
					if (kNext.t<=kCur.t)
						ratio=0.0f;
					else
						ratio=((float)(t-kCur.t))/((float)(kNext.t-kCur.t));

					k.t=t;
					k.xfmCenter=kNext.xfmCenter.getInterpolated(kCur.xfmCenter,ratio);
					k.radiusInner=i_math::lerp(kCur.radiusInner,kNext.radiusInner,ratio);
					k.radiusOutter=i_math::lerp(kCur.radiusOutter,kNext.radiusOutter,ratio);
					k.radianFrom=i_math::lerp_angle(kCur.radianFrom,kNext.radianFrom,ratio);
					k.radianTo=i_math::lerp_angle(kCur.radianTo,kNext.radianTo,ratio);
					k.ht=i_math::lerp(kCur.ht,kNext.ht,ratio);

					return TRUE;
				}

			}
		}
	}
	return FALSE;
}

BOOL AnimEventZone::CalcXform(AnimTick t,i_math::xformf &xfm)
{
	if (tp==Fan)
	{
		KeyFan k;
		if (CalcKeyFan(t,k))
		{
			k.CalcXfm(xfm);
			return TRUE;
		}
	}
	return FALSE;
}



//////////////////////////////////////////////////////////////////////////
//AnimEventZone::KeyFan
BOOL AnimEventZone::KeyFan::CheckIn(i_math::vector2df &posTarget0,float radiusTarget)
{
	i_math::vector2df posTarget;
	if (TRUE)//转换到局部空间
	{
		i_math::vector3df pos;
		pos.setXZ(posTarget0);
		pos.y=xfmCenter.pos.y;//拉到于xfmCenter同一高度

		xfmCenter.transformVectInv(pos,pos);

		if (TRUE)
		{
			i_math::vector3df pos;
			pos.setXZ(posTarget0);
			pos.y=xfmCenter.pos.y;//拉到于xfmCenter同一高度

			i_math::matrix43f mat=xfmCenter.getMatrix();
			mat.makeInverse();
			mat.transformVect(pos,pos);

		}


		posTarget=pos.getXZ();
	}

	float dist=posTarget.getLength();
	if (dist>radiusOutter+radiusTarget)
		return FALSE;
	if (dist<radiusInner-radiusTarget)
		return FALSE;

	float radianAddOn=radiusTarget/dist;
	float radianTarget=atan2f(posTarget.y,posTarget.x);

	while(radianTarget<radianFrom-radianAddOn)
		radianTarget+=i_math::Pi*2.0f;

	if (radianTarget-(radianFrom-radianAddOn)>radianTo+radianAddOn-(radianFrom-radianAddOn))
		return FALSE;

	return TRUE;
}

i_math::circlef AnimEventZone::KeyFan::CalcBoundingCircle()
{
	i_math::circlef circle;
	if (fabsf(radianTo-radianFrom)>=i_math::Pi)
	{
		circle.setCenter(xfmCenter.pos.getXZ());
		circle.setRadius(radiusOutter);
	}
	else 
	{
		i_math::vector2df corners[4];
		i_math::vector2df center=xfmCenter.pos.getXZ();

		corners[0].x=cosf(radianFrom)*radiusInner;
		corners[0].y=sinf(radianFrom)*radiusInner;

		corners[1].x=cosf(radianTo)*radiusInner;
		corners[1].y=sinf(radianTo)*radiusInner;

		corners[2].x=cosf(radianFrom)*radiusOutter;
		corners[2].y=sinf(radianFrom)*radiusOutter;

		corners[3].x=cosf(radianTo)*radiusOutter;
		corners[3].y=sinf(radianTo)*radiusOutter;
		for (int i=0;i<4;i++)
		{
			i_math::vector3df pos(corners[i].x,0.0f,corners[i].y);
			xfmCenter.transformVect(pos,pos);
			corners[i]=pos.getXZ();
		}

		circle.fromPoints(corners,4);
	}

	return circle;
}

BOOL AnimEventZone::KeyFan::CalcInfo(i_math::vector3df &pos,i_math::vector3df &dir,float &fov)
{
	pos=xfmCenter.pos;

	float rad=i_math::lerp_angle(radianFrom,radianTo,0.5f);
	dir.set(cosf(rad),0.0f,sinf(rad));
	dir=xfmCenter.rot*dir;
	dir.normalize();

	fov=i_math::get_radian_dist(radianFrom,radianTo);
	return TRUE;
}

void AnimEventZone::KeyFan::CalcXfm(i_math::xformf &xfm)
{
	xfm=xfmCenter;

	float rad=i_math::lerp_angle(radianFrom,radianTo,0.5f);
	i_math::vector3df dir;
	dir.set(cosf(rad),0.0f,sinf(rad));
	dir=xfmCenter.rot*dir;
	dir.normalize();

	i_math::vector3df euler;
	euler.x=i_math::Pi/2.0f-atan2f(dir.y,dir.x);
	xfm.rot.fromEuler(euler);
}
