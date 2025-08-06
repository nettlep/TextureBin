// ---------------------------------------------------------------------------------------------------------------------------------
//   _____ _ _                            
//  / ____| (_)                           
// | |    | |_ _ __       ___ _ __  _ __  
// | |    | | | '_ \     / __| '_ \| '_ \ 
// | |____| | | |_) | _ | (__| |_) | |_) |
//  \_____|_|_| .__/ (_) \___| .__/| .__/ 
//            | |            | |   | |    
//            |_|            |_|   |_|    
//
// 3D Homogenous clipper
//
// ---------------------------------------------------------------------------------------------------------------------------------
//
// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep
//
// ---------------------------------------------------------------------------------------------------------------------------------
// Originally created on 12/06/2000 by Paul Nettle
//
// Copyright 2000, Fluid Studios, Inc., all rights reserved.
// ---------------------------------------------------------------------------------------------------------------------------------

#include "texturebin.h"
#include "clip.h"

// ---------------------------------------------------------------------------------------------------------------------------------

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ---------------------------------------------------------------------------------------------------------------------------------

static	inline	void	nClip(Point4 &wdst, const Point4 &won, const Point4 &woff, Point4 &vdst, const Point4 &von, const Point4 &voff, Point2 &tdst, const Point2 &ton, const Point2 &toff, Point3 &ndst, const Point3 &non, const Point3 &noff)
{
	float	delta = -von.z() / (voff.z() - von.z());
	wdst = won + (woff - won) * delta;
	vdst = von + (voff - von) * delta;
	tdst = ton + (toff - ton) * delta;
	ndst = non + (noff - non) * delta;
	ndst.normalize();
}

// ---------------------------------------------------------------------------------------------------------------------------------

static	inline	void	fClip(Point4 &wdst, const Point4 &won, const Point4 &woff, Point4 &vdst, const Point4 &von, const Point4 &voff, Point2 &tdst, const Point2 &ton, const Point2 &toff, Point3 &ndst, const Point3 &non, const Point3 &noff)
{
	float	delta = (von.w() - von.z()) / ((voff.z() + von.w()) - (von.z() + voff.w()));
	wdst = won + (woff - won) * delta;
	vdst = von + (voff - von) * delta;
	tdst = ton + (toff - ton) * delta;
	ndst = non + (noff - non) * delta;
	ndst.normalize();
}

// ---------------------------------------------------------------------------------------------------------------------------------

static	inline	void	lClip(Point4 &wdst, const Point4 &won, const Point4 &woff, Point4 &vdst, const Point4 &von, const Point4 &voff, Point2 &tdst, const Point2 &ton, const Point2 &toff, Point3 &ndst, const Point3 &non, const Point3 &noff)
{
	float	delta = (-von.x() - von.w()) / (voff.w() - von.w() + voff.x() - von.x());
	wdst = won + (woff - won) * delta;
	vdst = von + (voff - von) * delta;
	tdst = ton + (toff - ton) * delta;
	ndst = non + (noff - non) * delta;
	ndst.normalize();
}

// ---------------------------------------------------------------------------------------------------------------------------------

static	inline	void	rClip(Point4 &wdst, const Point4 &won, const Point4 &woff, Point4 &vdst, const Point4 &von, const Point4 &voff, Point2 &tdst, const Point2 &ton, const Point2 &toff, Point3 &ndst, const Point3 &non, const Point3 &noff)
{
	float	delta = (von.x() - von.w()) / (voff.w() - von.w() - voff.x() + von.x());
	wdst = won + (woff - won) * delta;
	vdst = von + (voff - von) * delta;
	tdst = ton + (toff - ton) * delta;
	ndst = non + (noff - non) * delta;
	ndst.normalize();
}

// ---------------------------------------------------------------------------------------------------------------------------------

static	inline	void	tClip(Point4 &wdst, const Point4 &won, const Point4 &woff, Point4 &vdst, const Point4 &von, const Point4 &voff, Point2 &tdst, const Point2 &ton, const Point2 &toff, Point3 &ndst, const Point3 &non, const Point3 &noff)
{
	float	delta = (von.y() - von.w()) / (voff.w() - von.w() - voff.y() + von.y());
	wdst = won + (woff - won) * delta;
	vdst = von + (voff - von) * delta;
	tdst = ton + (toff - ton) * delta;
	ndst = non + (noff - non) * delta;
	ndst.normalize();
}

// ---------------------------------------------------------------------------------------------------------------------------------

static	inline	void	bClip(Point4 &wdst, const Point4 &won, const Point4 &woff, Point4 &vdst, const Point4 &von, const Point4 &voff, Point2 &tdst, const Point2 &ton, const Point2 &toff, Point3 &ndst, const Point3 &non, const Point3 &noff)
{
	float	delta = (-von.y() - von.w()) / (voff.w() - von.w() + voff.y() - von.y());
	wdst = won + (woff - won) * delta;
	vdst = von + (voff - von) * delta;
	tdst = ton + (toff - ton) * delta;
	ndst = non + (noff - non) * delta;
	ndst.normalize();
}

// ---------------------------------------------------------------------------------------------------------------------------------

static	unsigned int	neClip(primitive<> &src)
{
	primitive<>	dst;
	vert<>		tmp;

	for (unsigned int i = 0; i < src.vertexCount(); i++)
	{
		Point4		&wcur = src[i].world();
		Point4		&vcur = src[i].worldView();
		Point2		&tcur = src[i].textureView();
		Point3		&ncur = src[i].normalView();
		Point4		&wnex = (i == src.vertexCount()-1) ? src[0].world():src[i+1].world();
		Point4		&vnex = (i == src.vertexCount()-1) ? src[0].worldView():src[i+1].worldView();
		Point2		&tnex = (i == src.vertexCount()-1) ? src[0].textureView():src[i+1].textureView();
		Point3		&nnex = (i == src.vertexCount()-1) ? src[0].normalView():src[i+1].normalView();

		switch((vcur.z() < 0.0 ? 1:0)|(vnex.z() < 0.0 ? 2:0))
		{
			case 0:	tmp.world() = wcur; tmp.worldView() = vcur; tmp.textureView() = tcur; tmp.normalView() = ncur; dst += tmp; break;
			case 1: nClip(tmp.world(), wnex, wcur, tmp.worldView(), vnex, vcur, tmp.textureView(), tnex, tcur, tmp.normalView(), nnex, ncur); dst += tmp; break;
			case 2:	tmp.world() = wcur; tmp.worldView() = vcur; tmp.textureView() = tcur; tmp.normalView() = ncur; dst += tmp;
				nClip(tmp.world(), wcur, wnex, tmp.worldView(), vcur, vnex, tmp.textureView(), tcur, tnex, tmp.normalView(), ncur, nnex); dst += tmp; break;
		}
	}
	src = dst;
	return src.vertexCount();
}

// ---------------------------------------------------------------------------------------------------------------------------------

static	unsigned int	feClip(primitive<> &src)
{
	primitive<>	dst;
	vert<>		tmp;

	for (unsigned int i = 0; i < src.vertexCount(); i++)
	{
		Point4		&wcur = src[i].world();
		Point4		&vcur = src[i].worldView();
		Point2		&tcur = src[i].textureView();
		Point3		&ncur = src[i].normalView();
		Point4		&wnex = (i == src.vertexCount()-1) ? src[0].world():src[i+1].world();
		Point4		&vnex = (i == src.vertexCount()-1) ? src[0].worldView():src[i+1].worldView();
		Point2		&tnex = (i == src.vertexCount()-1) ? src[0].textureView():src[i+1].textureView();
		Point3		&nnex = (i == src.vertexCount()-1) ? src[0].normalView():src[i+1].normalView();

		switch((vcur.z() > vcur.w() ? 1:0)|(vnex.z() > vnex.w() ? 2:0))
		{
			case 0:	tmp.world() = wcur; tmp.worldView() = vcur; tmp.textureView() = tcur; tmp.normalView() = ncur; dst += tmp; break;
			case 1: fClip(tmp.world(), wnex, wcur, tmp.worldView(), vnex, vcur, tmp.textureView(), tnex, tcur, tmp.normalView(), nnex, ncur); dst += tmp; break;
			case 2:	tmp.world() = wcur; tmp.worldView() = vcur; tmp.textureView() = tcur; tmp.normalView() = ncur; dst += tmp;
				fClip(tmp.world(), wcur, wnex, tmp.worldView(), vcur, vnex, tmp.textureView(), tcur, tnex, tmp.normalView(), ncur, nnex); dst += tmp; break;
		}
	}
	src = dst;
	return src.vertexCount();
}

// ---------------------------------------------------------------------------------------------------------------------------------

static	unsigned int	leClip(primitive<> &src)
{
	primitive<>	dst;
	vert<>		tmp;

	for (unsigned int i = 0; i < src.vertexCount(); i++)
	{
		Point4		&wcur = src[i].world();
		Point4		&vcur = src[i].worldView();
		Point2		&tcur = src[i].textureView();
		Point3		&ncur = src[i].normalView();
		Point4		&wnex = (i == src.vertexCount()-1) ? src[0].world():src[i+1].world();
		Point4		&vnex = (i == src.vertexCount()-1) ? src[0].worldView():src[i+1].worldView();
		Point2		&tnex = (i == src.vertexCount()-1) ? src[0].textureView():src[i+1].textureView();
		Point3		&nnex = (i == src.vertexCount()-1) ? src[0].normalView():src[i+1].normalView();

		switch((vcur.x() < -vcur.w() ? 1:0)|(vnex.x() < -vnex.w() ? 2:0))
		{
			case 0:	tmp.world() = wcur; tmp.worldView() = vcur; tmp.textureView() = tcur; tmp.normalView() = ncur; dst += tmp; break;
			case 1: lClip(tmp.world(), wnex, wcur, tmp.worldView(), vnex, vcur, tmp.textureView(), tnex, tcur, tmp.normalView(), nnex, ncur); dst += tmp; break;
			case 2:	tmp.world() = wcur; tmp.worldView() = vcur; tmp.textureView() = tcur; tmp.normalView() = ncur; dst += tmp;
				lClip(tmp.world(), wcur, wnex, tmp.worldView(), vcur, vnex, tmp.textureView(), tcur, tnex, tmp.normalView(), ncur, nnex); dst += tmp; break;
		}
	}
	src = dst;
	return src.vertexCount();
}

// ---------------------------------------------------------------------------------------------------------------------------------

static	unsigned int	reClip(primitive<> &src)
{
	primitive<>	dst;
	vert<>		tmp;

	for (unsigned int i = 0; i < src.vertexCount(); i++)
	{
		Point4		&wcur = src[i].world();
		Point4		&vcur = src[i].worldView();
		Point2		&tcur = src[i].textureView();
		Point3		&ncur = src[i].normalView();
		Point4		&wnex = (i == src.vertexCount()-1) ? src[0].world():src[i+1].world();
		Point4		&vnex = (i == src.vertexCount()-1) ? src[0].worldView():src[i+1].worldView();
		Point2		&tnex = (i == src.vertexCount()-1) ? src[0].textureView():src[i+1].textureView();
		Point3		&nnex = (i == src.vertexCount()-1) ? src[0].normalView():src[i+1].normalView();

		switch((vcur.x() > vcur.w() ? 1:0)|(vnex.x() > vnex.w() ? 2:0))
		{
			case 0:	tmp.world() = wcur; tmp.worldView() = vcur; tmp.textureView() = tcur; tmp.normalView() = ncur; dst += tmp; break;
			case 1:	rClip(tmp.world(), wnex, wcur, tmp.worldView(), vnex, vcur, tmp.textureView(), tnex, tcur, tmp.normalView(), nnex, ncur); dst += tmp; break;
			case 2:	tmp.world() = wcur; tmp.worldView() = vcur; tmp.textureView() = tcur; tmp.normalView() = ncur; dst += tmp;
				rClip(tmp.world(), wcur, wnex, tmp.worldView(), vcur, vnex, tmp.textureView(), tcur, tnex, tmp.normalView(), ncur, nnex); dst += tmp; break;
		}
	}
	src = dst;
	return src.vertexCount();
}

// ---------------------------------------------------------------------------------------------------------------------------------

static	unsigned int	teClip(primitive<> &src)
{
	primitive<>	dst;
	vert<>		tmp;

	for (unsigned int i = 0; i < src.vertexCount(); i++)
	{
		Point4		&wcur = src[i].world();
		Point4		&vcur = src[i].worldView();
		Point2		&tcur = src[i].textureView();
		Point3		&ncur = src[i].normalView();
		Point4		&wnex = (i == src.vertexCount()-1) ? src[0].world():src[i+1].world();
		Point4		&vnex = (i == src.vertexCount()-1) ? src[0].worldView():src[i+1].worldView();
		Point2		&tnex = (i == src.vertexCount()-1) ? src[0].textureView():src[i+1].textureView();
		Point3		&nnex = (i == src.vertexCount()-1) ? src[0].normalView():src[i+1].normalView();

		switch((vcur.y() > vcur.w() ? 1:0)|(vnex.y() > vnex.w() ? 2:0))
		{
			case 0:	tmp.world() = wcur; tmp.worldView() = vcur; tmp.textureView() = tcur; tmp.normalView() = ncur; dst += tmp; break;
			case 1:	tClip(tmp.world(), wnex, wcur, tmp.worldView(), vnex, vcur, tmp.textureView(), tnex, tcur, tmp.normalView(), nnex, ncur); dst += tmp; break;
			case 2:	tmp.world() = wcur; tmp.worldView() = vcur; tmp.textureView() = tcur; tmp.normalView() = ncur; dst += tmp;
				tClip(tmp.world(), wcur, wnex, tmp.worldView(), vcur, vnex, tmp.textureView(), tcur, tnex, tmp.normalView(), ncur, nnex); dst += tmp; break;
		}
	}
	src = dst;
	return src.vertexCount();
}

// ---------------------------------------------------------------------------------------------------------------------------------

static	unsigned int	beClip(primitive<> &src)
{
	primitive<>	dst;
	vert<>		tmp;

	for (unsigned int i = 0; i < src.vertexCount(); i++)
	{
		Point4		&wcur = src[i].world();
		Point4		&vcur = src[i].worldView();
		Point2		&tcur = src[i].textureView();
		Point3		&ncur = src[i].normalView();
		Point4		&wnex = (i == src.vertexCount()-1) ? src[0].world():src[i+1].world();
		Point4		&vnex = (i == src.vertexCount()-1) ? src[0].worldView():src[i+1].worldView();
		Point2		&tnex = (i == src.vertexCount()-1) ? src[0].textureView():src[i+1].textureView();
		Point3		&nnex = (i == src.vertexCount()-1) ? src[0].normalView():src[i+1].normalView();

		switch((vcur.y() < -vcur.w() ? 1:0)|(vnex.y() < -vnex.w() ? 2:0))
		{
			case 0:	tmp.world() = wcur; tmp.worldView() = vcur; tmp.textureView() = tcur; tmp.normalView() = ncur; dst += tmp; break;
			case 1:	bClip(tmp.world(), wnex, wcur, tmp.worldView(), vnex, vcur, tmp.textureView(), tnex, tcur, tmp.normalView(), nnex, ncur); dst += tmp; break;
			case 2:	tmp.world() = wcur; tmp.worldView() = vcur; tmp.textureView() = tcur; tmp.normalView() = ncur; dst += tmp;
				bClip(tmp.world(), wcur, wnex, tmp.worldView(), vcur, vnex, tmp.textureView(), tcur, tnex, tmp.normalView(), ncur, nnex); dst += tmp; break;
		}
	}
	src = dst;
	return src.vertexCount();
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	clipPrimitive(primitive<> &p)
{
	if (neClip(p) < 3) return false;
	if (feClip(p) < 3) return false;
	if (leClip(p) < 3) return false;
	if (reClip(p) < 3) return false;
	if (teClip(p) < 3) return false;
	if (beClip(p) < 3) return false;
	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------
// Clip.cpp - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
