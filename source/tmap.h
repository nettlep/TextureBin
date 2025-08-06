// ---------------------------------------------------------------------------------------------------------------------------------
//  _______ __  __                 _     
// |__   __|  \/  |               | |    
//    | |  | \  / | __ _ _ __     | |__  
//    | |  | |\/| |/ _` | '_ \    | '_ \ 
//    | |  | |  | | (_| | |_) | _ | | | |
//    |_|  |_|  |_|\__,_| .__/ (_)|_| |_|
//                      | |              
//                      |_|              
//
// Generic texture mapper
//
// Best viewed with 8-character tabs and (at least) 132 columns
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

#ifndef	_H_TMAP
#define	_H_TMAP

// ---------------------------------------------------------------------------------------------------------------------------------
// Forward Declarations
// ---------------------------------------------------------------------------------------------------------------------------------

class	Bitmap;
class	ShadowMap;

#include "primitive.h"

// ---------------------------------------------------------------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------------------------------------------------------------

extern	const	unsigned int	textureWidth;
extern	const	unsigned int	textureHeight;
extern	const	unsigned int	subShift;
extern	const	unsigned int	subSpan;

// ---------------------------------------------------------------------------------------------------------------------------------

typedef	struct vertex
{
	int	iy;
	Point2	screen;
	Point2	texture;
	Point4	view;
	Point4	world;
	Vector3	normal;
	int	polygonID;
	struct	vertex *next;
} sVERT;

// ---------------------------------------------------------------------------------------------------------------------------------

typedef	struct
{
	Point4	pos;
	Vector3	dir;
	Point3	color;
	float	innerRange, outerRange;
	float	hotspot, falloff;
} sLIGHT;

// ---------------------------------------------------------------------------------------------------------------------------------

typedef	struct
{
	float	Ka;
	float	Kd;
	float	Ks;
	float	Sh; // Shininess
	float	shadowMapBias;
	int	shadowMapRes;
	Point3	ambientColor;
	Point3	specularColor;
} sPHONG;
// ---------------------------------------------------------------------------------------------------------------------------------

typedef	struct
{
	float	sx, dsx;
	int	height;
	Point2	texture, dtexture;
	Point4	view, dview;
	Point4	world, dworld;
	Vector3	normal, dnormal;
} sEDGE;

// ---------------------------------------------------------------------------------------------------------------------------------
// Prototypes
// ---------------------------------------------------------------------------------------------------------------------------------

void	drawPerspectiveTexturedPolygon(sVERT *verts, const std::vector<sLIGHT> & lights, const std::vector<ShadowMap> & shadowMaps, const sPHONG & phong, unsigned int *frameBuffer, unsigned int *textureBuffer, float *zBuffer, const unsigned int pitch, const unsigned int textureWidth, const unsigned int textureHeight);
void	drawShadowMapPolygon(sVERT *verts, float *zBuffer, const unsigned int pitch);

#endif
// ---------------------------------------------------------------------------------------------------------------------------------
// TMap.h - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
