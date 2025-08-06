// ---------------------------------------------------------------------------------------------------------------------------------
//  _______ __  __                                  
// |__   __|  \/  |                                 
//    | |  | \  / | __ _ _ __       ___ _ __  _ __  
//    | |  | |\/| |/ _` | '_ \     / __| '_ \| '_ \ 
//    | |  | |  | | (_| | |_) | _ | (__| |_) | |_) |
//    |_|  |_|  |_|\__,_| .__/ (_) \___| .__/| .__/ 
//                      | |            | |   | |    
//                      |_|            |_|   |_|    
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

#include "texturebin.h"
#include "tmap.h"
#include "render.h"
#include <cmath>

// ---------------------------------------------------------------------------------------------------------------------------------
// This is handy
// ---------------------------------------------------------------------------------------------------------------------------------

template<class T>
static	inline	T &_min(T &a, T &b)
{
	return a < b ? a : b;
}

// ---------------------------------------------------------------------------------------------------------------------------------
//
//	Ix = result color
//	Lx = light color
//	Ax = ambient color
//	Dx = diffuse color
//	Sx = specular color
//	Ka = ambient coefficient
//	Kd = diffuse coefficient
//	Ks = specular coefficient
//	Att = attenuation coefficient
//	n = usually referred to as "shine" or "roughness"
//	N = surface normal
//	L = light vector
//	R = reflection vector
//	V = view vector
//
//	Given those variables, here's the Phong model (modified to account for specular	color):
//
//	Ix = AxKaDx + AttLx  [KdDx(N dot L) + KsSx(R dot V)^n]
// ---------------------------------------------------------------------------------------------------------------------------------

static	Point3	light(const Vector3 & N, const Point4 & view, const Point4 & world, const Point3 & diffuse, const std::vector<sLIGHT> & lights, const std::vector<ShadowMap> & shadowMaps, const sPHONG & phong)
{
	// Vector that points to the camera -- since everything is transformed into view space, the camera is at (0,0,0)

	Vector3	V(-view);
	V.normalize();

	// Setup

	Point3	combinedSpecular = phong.specularColor * phong.Ks;
	Point3	combinedDiffuse = diffuse * phong.Kd;

	// Start with ambient

	Point3	result = phong.ambientColor * phong.Ka * diffuse;

	for (unsigned int i = 0; i < lights.size(); ++i)
	{
		const sLIGHT &	curLight = lights[i];
		Vector3		L(curLight.pos - Vector3(world));
		if ((N ^ L) < 0) continue;

		// Distance to the light source

		float	lLength = L.length();

		// Beyond the outer range of the light source?

		if (lLength > curLight.outerRange) continue;

		// Normalize the light vector

		L /= lLength;

		// Spotlight hotspot/falloff

		float	diffuseScalar = -(L ^ curLight.dir);
		if (diffuseScalar < curLight.falloff) diffuseScalar = 0;
		else if (diffuseScalar > curLight.hotspot) diffuseScalar = 1;
		else	diffuseScalar = (diffuseScalar - curLight.falloff) / (curLight.hotspot - curLight.falloff);

		// Calculate shadow

		float	shadowPercent = 0;
		{
			const ShadowMap &	sm = shadowMaps[i];
			float	halfWidth = (float) (sm.camera.width >> 1);
			float	halfHeight = (float) (sm.camera.height >> 1);

			Point4	lPoint = sm.xform >> world;
			float	ow = 1.0f / lPoint.w();

			float	lx = halfWidth + lPoint.x() * ow * halfWidth * ((halfWidth-1)/halfWidth);
			int	ilx = (int) lx;
			if (ilx-1 < 0 || ilx+1 >= (int) sm.camera.width) continue;

			float	ly = halfHeight - lPoint.y() * ow * halfHeight * ((halfHeight-1)/halfHeight);
			int	ily = (int) ly;
			if (ily-1 < 0 || ily+1 >= (int) sm.camera.height) continue;

			// Filtering

			int	vCount = 0;
			int	smIndex = ily * sm.camera.width + ilx;
			float	lw;
			const	float	bias = phong.shadowMapBias;
			lw = sm.zBuffer[smIndex-1]; if (lw > 0 && (lPoint.w() <= (1/lw)+bias)) vCount++;
			lw = sm.zBuffer[smIndex+0]; if (lw > 0 && (lPoint.w() <= (1/lw)+bias)) vCount++;
			lw = sm.zBuffer[smIndex+1]; if (lw > 0 && (lPoint.w() <= (1/lw)+bias)) vCount++;
			smIndex += sm.camera.width;
			lw = sm.zBuffer[smIndex-1]; if (lw > 0 && (lPoint.w() <= (1/lw)+bias)) vCount++;
			lw = sm.zBuffer[smIndex+0]; if (lw > 0 && (lPoint.w() <= (1/lw)+bias)) vCount++;
			lw = sm.zBuffer[smIndex+1]; if (lw > 0 && (lPoint.w() <= (1/lw)+bias)) vCount++;
			smIndex += sm.camera.width;
			lw = sm.zBuffer[smIndex-1]; if (lw > 0 && (lPoint.w() <= (1/lw)+bias)) vCount++;
			lw = sm.zBuffer[smIndex+0]; if (lw > 0 && (lPoint.w() <= (1/lw)+bias)) vCount++;
			lw = sm.zBuffer[smIndex+1]; if (lw > 0 && (lPoint.w() <= (1/lw)+bias)) vCount++;
			if (!vCount) continue;

			// Calculate the shadow percentage

			shadowPercent = (float) vCount / 9;
		}

		float	NdotL = N ^ L;

		// Attenuation

		float	attenuation = 1;
		if (lLength > curLight.innerRange) attenuation = 1-(lLength - curLight.innerRange) / (curLight.outerRange - curLight.innerRange);

		// Reflection vector for specular

		Vector3	R = N*2 * NdotL - L;
		float	RdotV = R ^ V;
		float	specular = 0;
		if (RdotV > 0) specular = static_cast<float>(pow(RdotV, phong.Sh));

		// The Phong equation

		result += curLight.color * attenuation * (combinedDiffuse * NdotL * diffuseScalar + combinedSpecular * specular) * shadowPercent;
	}

	return result;
}

// ---------------------------------------------------------------------------------------------------------------------------------

static	inline	void	calcEdgeDeltas(sEDGE &edge, sVERT *top, sVERT *bot)
{
	// Edge deltas

	float	overHeight = 1.0f / (bot->screen.y() - top->screen.y());
	edge.dsx      = (bot->screen.x() - top->screen.x()) * overHeight;
	edge.dtexture = (bot->texture    - top->texture)    * overHeight;
	edge.dview    = (bot->view       - top->view)       * overHeight;
	edge.dworld   = (bot->world      - top->world)      * overHeight;
	edge.dnormal  = (bot->normal     - top->normal)     * overHeight;

	// Screen pixel Adjustments (some call this "sub-pixel accuracy")

	float	subPix = (float) top->iy - top->screen.y();
	edge.sx      = top->screen.x() + edge.dsx      * subPix;
	edge.texture = top->texture    + edge.dtexture * subPix;
	edge.view    = top->view       + edge.dview    * subPix;
	edge.world   = top->world      + edge.dworld   * subPix;
	edge.normal  = top->normal     + edge.dnormal  * subPix;
}

// ---------------------------------------------------------------------------------------------------------------------------------

static	inline	void	calcEdgeDeltasShadowMap(sEDGE &edge, sVERT *top, sVERT *bot)
{
	// Edge deltas

	float	overHeight = 1.0f / (bot->screen.y() - top->screen.y());
	edge.dsx        = (bot->screen.x() - top->screen.x()) * overHeight;
	edge.dview.w()  = (bot->view.w()   - top->view.w())   * overHeight;

	// Screen pixel Adjustments (some call this "sub-pixel accuracy")

	float	subPix = (float) top->iy - top->screen.y();
	edge.sx       = top->screen.x() + edge.dsx       * subPix;
	edge.view.w() = top->view.w()   + edge.dview.w() * subPix;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	drawPerspectiveTexturedPolygon(sVERT *verts, const std::vector<sLIGHT> & lights, const std::vector<ShadowMap> & shadowMaps, const sPHONG & phong, unsigned int *frameBuffer, unsigned int *textureBuffer, float *zBuffer, const unsigned int pitch, const unsigned int textureWidth, const unsigned int textureHeight)
{
	// Find the top-most vertex

	sVERT		*v = verts, *lastVert = verts, *lTop = verts, *rTop;

	while(v)
	{
		if (v->screen.y() < lTop->screen.y()) lTop = v;
		lastVert = v;
		v->iy = (int) ceil(v->screen.y());
		v = v->next;
	}

	// Make sure we have the top-most vertex that is earliest in the winding order

	if (lastVert->screen.y() == lTop->screen.y() && verts->screen.y() == lTop->screen.y()) lTop = lastVert;

	rTop = lTop;

	// Top scanline of the polygon in the frame buffer

	unsigned int	*fb = &frameBuffer[lTop->iy * pitch];
	float		*zb = &zBuffer[lTop->iy * pitch];

	// Left & Right edges (primed with 0)

	sEDGE		le, re;
	le.height = 0;
	re.height = 0;

	// Render the polygon

	bool	done = false;
	while(!done)
	{
		if (!le.height)
		{
			sVERT	*lBot = lTop - 1; if (lBot < verts) lBot = lastVert;
			le.height = lBot->iy - lTop->iy;
			if (le.height < 0) return;
			calcEdgeDeltas(le, lTop, lBot);
			lTop = lBot;
			if (lTop == rTop) done = true;
			if (lTop != rTop && done) return;
		}

		if (!re.height)
		{
			sVERT	*rBot = rTop + 1; if (rBot > lastVert) rBot = verts;
			re.height = rBot->iy - rTop->iy;
			if (re.height < 0) return;
			calcEdgeDeltas(re, rTop, rBot);
			rTop = rBot;
			if (lTop == rTop) done = true;
			if (lTop != rTop && done) return;
		}

		// Get the height

		int	height = _min(le.height, re.height);

		// Subtract the height from each edge

		le.height -= height;
		re.height -= height;

		// Render the current trapezoid defined by left & right edges

		while(height-- > 0)
		{
			// Texture coordinates

			float		overWidth = 1.0f / (re.sx - le.sx);
			Point2		dtexture = (re.texture - le.texture) * overWidth;
			Point4		dview    = (re.view    - le.view   ) * overWidth;
			Point4		dworld   = (re.world   - le.world  ) * overWidth;
			Vector3		dnormal  = (re.normal  - le.normal ) * overWidth;

			// Find the end-points

			int		start = (int) ceil(le.sx);
			int		end   = (int) ceil(re.sx);

			// Texture adjustment (some call this "sub-texel accuracy")

			float		subTex = (float) start - le.sx;
			Point2		texture = le.texture + dtexture * subTex;
			Point4		view    = le.view    + dview    * subTex;
			Point4		world   = le.world   + dworld   * subTex;
			Vector3		normal  = le.normal  + dnormal  * subTex;

			// Fill the entire span

			unsigned int	*span = fb + start;
			float		*zspan = zb + start;

			for (; start < end; start++)
			{
				if (view.w() > *zspan)
				{
					float	z = 1.0f / view.w();
					int	s = (int) (texture.x() * z) % textureWidth;
					int	t = (int) (texture.y() * z) % textureHeight;

					Vector3	n(normal*z);
					n.normalize();

					int	c = textureBuffer[t * textureWidth + s];
					int	r = (c >> 16) & 0xff;
					int	g = (c >>  8) & 0xff;
					int	b = (c      ) & 0xff;
					Point3	diffuseColor(r/255.0f, g/255.0f, b/255.0f);
					Point3	result = light(n, view*z, world*z, diffuseColor, lights, shadowMaps, phong);

					r = static_cast<int>(result.r() * 255);
					g = static_cast<int>(result.g() * 255);
					b = static_cast<int>(result.b() * 255);
					if (r < 0) r = 0;	if (r > 255) r = 255;
					if (g < 0) g = 0;	if (g > 255) g = 255;
					if (b < 0) b = 0;	if (b > 255) b = 255;

					*span = (r<<16) | (g<<8) | b;
					*zspan = view.w();
				}
				texture += dtexture;
				view += dview;
				world += dworld;
				normal += dnormal;
				span++;
				zspan++;
			}

			// Step

			le.sx += le.dsx;
			le.texture += le.dtexture;
			le.view += le.dview;
			le.world += le.dworld;
			le.normal += le.dnormal;

			re.sx += re.dsx;
			re.texture += re.dtexture;
			re.view += re.dview;
			re.world += re.dworld;
			re.normal += re.dnormal;

			fb += pitch;
			zb += pitch;
		}
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	drawShadowMapPolygon(sVERT *verts, float *zBuffer, const unsigned int pitch)
{
	// Find the top-most vertex

	sVERT		*v = verts, *lastVert = verts, *lTop = verts, *rTop;

	while(v)
	{
		if (v->screen.y() < lTop->screen.y()) lTop = v;
		lastVert = v;
		v->iy = (int) ceil(v->screen.y());
		v = v->next;
	}

	// Make sure we have the top-most vertex that is earliest in the winding order

	if (lastVert->screen.y() == lTop->screen.y() && verts->screen.y() == lTop->screen.y()) lTop = lastVert;

	rTop = lTop;

	// Top scanline of the polygon in the frame buffer

	float		*zb = &zBuffer[lTop->iy * pitch];

	// Left & Right edges (primed with 0)

	sEDGE		le, re;
	le.height = 0;
	re.height = 0;

	// Render the polygon

	bool	done = false;
	while(!done)
	{
		if (!le.height)
		{
			sVERT	*lBot = lTop - 1; if (lBot < verts) lBot = lastVert;
			le.height = lBot->iy - lTop->iy;
			if (le.height < 0) return;
			calcEdgeDeltasShadowMap(le, lTop, lBot);
			lTop = lBot;
			if (lTop == rTop) done = true;
			if (lTop != rTop && done) return;
		}

		if (!re.height)
		{
			sVERT	*rBot = rTop + 1; if (rBot > lastVert) rBot = verts;
			re.height = rBot->iy - rTop->iy;
			if (re.height < 0) return;
			calcEdgeDeltasShadowMap(re, rTop, rBot);
			rTop = rBot;
			if (lTop == rTop) done = true;
			if (lTop != rTop && done) return;
		}

		// Get the height

		int	height = _min(le.height, re.height);

		// Subtract the height from each edge

		le.height -= height;
		re.height -= height;

		// Render the current trapezoid defined by left & right edges

		while(height-- > 0)
		{
			// Find the end-points

			int		start = (int) ceil(le.sx);
			int		end   = (int) ceil(re.sx);

			// Depth

			float		dw = (re.view.w() - le.view.w()) / (re.sx - le.sx);
			float		w = le.view.w() + dw * ((float) start - le.sx);

			// Fill the entire span

			float		*zspan = zb + start;

			for (; start < end; start++)
			{
				if (w > *zspan) *zspan = w;
				w += dw;
				zspan++;
			}

			// Step

			le.sx += le.dsx;
			le.view.w() += le.dview.w();

			re.sx += re.dsx;
			re.view.w() += re.dview.w();

			zb += pitch;
		}
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------
// TMap.cpp - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
