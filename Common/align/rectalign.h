
#pragma once

// align style 
enum RectAlign
{
	ALIGN_LEFTUP,
	ALIGN_UP,
	ALIGN_RIGHTUP,
	ALIGN_LEFT,
	ALIGN_CENTER,
	ALIGN_RIGHT,
	ALIGN_LEFTDOWN,
	ALIGN_DOWN,
	ALIGN_RIGHTDOWN,
};

template<typename T>
i_math::rect<T> RelativeAreaToLocalRect(i_math::rect<T> & scrParent,i_math::rect<T> &rcArea,RectAlign alignType)
{

	i_math::rect<T> rcLocal;
	i_math::pos2d<T> p0(0,0);

	int w = rcArea.getWidth();
	int h = rcArea.getHeight();

	int sw = scrParent.getWidth();
	int sh = scrParent.getHeight();

	switch(alignType)
	{
	case ALIGN_LEFT:
		{
			p0.x = rcArea.UpperLeftCorner.x;
			p0.y = (sh - h)/2 + rcArea.UpperLeftCorner.y;
			break;
		}
	case ALIGN_RIGHT:
		{
			p0.x = sw - rcArea.UpperLeftCorner.x - w;
			p0.y = (sh - h)/2 + rcArea.UpperLeftCorner.y;
			break;
		}
	case ALIGN_UP:
		{
			p0.x = (sw - w)/2 + rcArea.UpperLeftCorner.x; 
			p0.y = rcArea.UpperLeftCorner.y;
			break;
		}
	case ALIGN_DOWN:
		{
			p0.x = (sw - w)/2 + rcArea.UpperLeftCorner.x;
			p0.y = sh - rcArea.UpperLeftCorner.y - h;
			break;
		}
	case ALIGN_LEFTUP:
		{
			p0 = rcArea.UpperLeftCorner;
			break;
		}
	case ALIGN_LEFTDOWN:
		{
			p0.x = rcArea.UpperLeftCorner.x;
			p0.y = sh - rcArea.UpperLeftCorner.y - h;
			break;
		}
	case ALIGN_RIGHTUP:
		{
			p0.x = sw - rcArea.UpperLeftCorner.x - w;
			p0.y = rcArea.UpperLeftCorner.y;
			break;
		}
	case ALIGN_RIGHTDOWN:
		{
			p0.x = sw - rcArea.UpperLeftCorner.x - w;
			p0.y = sh - rcArea.UpperLeftCorner.y - h;
			break;
		}
	case ALIGN_CENTER:
		{
			p0.x = (sw - w)/2 + rcArea.UpperLeftCorner.x;
			p0.y = (sh - h)/2 + rcArea.UpperLeftCorner.y;
			break;
		}
	default: break;
	}

	rcLocal.set(p0,i_math::size2d<T>(w,h));

	return rcLocal;
}

template<typename T>
i_math::rect<T> RelativeAreaToLocalRect(i_math::rect<T> & scrParent,i_math::vector4d<T>&rcArea0,RectAlign alignType)
{
	return RelativeAreaToLocalRect(scrParent,FORCE_TYPE(i_math::rect<T>,rcArea0),alignType);
}

template<typename T>
i_math::rect<T> LocalRectToRelativeArea(i_math::rect<T> & scrParent,i_math::rect<T>& rcLocal,RectAlign alignType)
{
	i_math::rect<T> rcArea;

	int w = rcLocal.getWidth();
	int h = rcLocal.getHeight();

	int sw = scrParent.getWidth();
	int sh = scrParent.getHeight();

	i_math::pos2d<T> p0(0,0);

	switch(alignType)
	{
	case ALIGN_LEFT:
		{
			p0.x = rcLocal.UpperLeftCorner.x;
			p0.y = rcLocal.UpperLeftCorner.y +(h - sh)/2;
			break;
		}
	case ALIGN_RIGHT:
		{
			p0.x = sw - (rcLocal.UpperLeftCorner.x + w);
			p0.y = rcLocal.UpperLeftCorner.y + (h - sh)/2;
			break;
		}
	case ALIGN_UP:
		{
			p0.x = rcLocal.UpperLeftCorner.x + (w - sw)/2;
			p0.y = rcLocal.UpperLeftCorner.y;
			break;
		}
	case ALIGN_DOWN:
		{
			p0.x = rcLocal.UpperLeftCorner.x - (sw - w)/2;
			p0.y = sh - (rcLocal.UpperLeftCorner.y + h); 	
			break;
		}
	case ALIGN_LEFTUP:
		{
			p0 = rcLocal.UpperLeftCorner;
			break;
		}
	case ALIGN_LEFTDOWN:
		{
			p0.x = rcLocal.UpperLeftCorner.x;
			p0.y = sh - (rcLocal.UpperLeftCorner.y + h);
			break;
		}
	case ALIGN_RIGHTUP:
		{
			p0.x = sw - (rcLocal.UpperLeftCorner.x + w);
			p0.y = rcLocal.UpperLeftCorner.y;
			break;
		}
	case ALIGN_RIGHTDOWN:
		{
			p0.x = sw - (rcLocal.UpperLeftCorner.x + w);
			p0.y = sh - (rcLocal.UpperLeftCorner.y + h);
			break;
		}
	case ALIGN_CENTER:
		{
			p0.x = rcLocal.UpperLeftCorner.x - (sw - w)/2;
			p0.y = rcLocal.UpperLeftCorner.y - (sh - h)/2;
			break;
		}
	default: break;
	}	

	rcArea.set(p0.x,p0.y,p0.x+w,p0.y+h);

	return rcArea;
}



template<typename T>
i_math::rect<T> DockRect(i_math::rect<T> &rcTarget,i_math::size2d<T> &szSrc,i_math::pos2d<T> &ptRef,RectAlign alignType,i_math::rect<T> &rcLimit)
{
	i_math::rect<T> rc;
	if (alignType==ALIGN_LEFT)
	{
		rc.Left()=rcTarget.Left()-szSrc.w;
		if (rc.Left()<rcLimit.Left())
			rc.Left()=rcTarget.Right();
		rc.Right()=rc.Left()+szSrc.w;

		rc.Bottom()=ptRef.y+szSrc.h;
		if (rc.Bottom()>rcLimit.Bottom())
			rc.Bottom()=rcLimit.Bottom();
		rc.Top()=rc.Bottom()-szSrc.h;
	}
	if (alignType==ALIGN_RIGHT)
	{
		rc.Right()=rcTarget.Right()+szSrc.w;
		if (rc.Right()>rcLimit.Right())
			rc.Right()=rcTarget.Left();
		rc.Left()=rc.Right()-szSrc.w;

		rc.Bottom()=ptRef.y+szSrc.h;
		if (rc.Bottom()>rcLimit.Bottom())
			rc.Bottom()=rcLimit.Bottom();
		rc.Top()=rc.Bottom()-szSrc.h;
	}

	if (alignType==ALIGN_UP)
	{
		rc.Top()=rcTarget.Top()-szSrc.h;
		if (rc.Top()<rcLimit.Top())
			rc.Top()=rcTarget.Bottom();
		rc.Bottom()=rc.Top()+szSrc.h;

		rc.Right()=ptRef.x+szSrc.w;
		if (rc.Right()>rcLimit.Right())
			rc.Right()=rcLimit.Right();
		rc.Left()=rc.Right()-szSrc.w;
	}

	if (alignType==ALIGN_DOWN)
	{
		rc.Bottom()=rcTarget.Bottom()+szSrc.h;
		if (rc.Bottom()>rcLimit.Bottom())
			rc.Bottom()=rcTarget.Top();
		rc.Top()=rc.Bottom()-szSrc.h;

		rc.Right()=ptRef.x+szSrc.w;
		if (rc.Right()>rcLimit.Right())
			rc.Right()=rcLimit.Right();
		rc.Left()=rc.Right()-szSrc.w;
	}

	return rc;

}



