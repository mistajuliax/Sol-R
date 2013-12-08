/* 
* Copyright (C) 2011-2012 Cyrille Favreau <cyrille_favreau@hotmail.com>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>. 
*/

/*
* Author: Cyrille Favreau <cyrille_favreau@hotmail.com>
*
*/

#pragma once

// Project
#include "../Consts.h"
#include "VectorUtils.cuh"


// ----------
// Normal mapping
// --------------------
__device__ inline void normalMap(
   const int&       index,
	const Material&  material,
	BitmapBuffer*    textures,
   Vertex&          normal)
{
   int i = material.textureOffset.y + index;
   BitmapBuffer r,g;
	r = textures[i  ];
	g = textures[i+1];
	normal.x = 2.f*(r/256.f-0.5f);
	normal.y = 2.f*(g/256.f-0.5f);
	normal.z = 0.f;
}

// ----------
// Normal mapping
// --------------------
__device__ inline void bumpMap(
   const int&       index,
	const Material&  material,
	BitmapBuffer*    textures,
   Vertex&          intersection)
{
   int i = material.textureOffset.z + index;
   BitmapBuffer r,g,b;
	r = textures[i  ];
	g = textures[i+1];
	b = textures[i+2];
	intersection.x += 10.f*(r/256.f-0.5f);
	intersection.y += 10.f*(g/256.f-0.5f);
	intersection.z += 10.f*(b/256.f-0.5f);
}

__device__ void juliaSet( 
	const Primitive& primitive,
	Material*        materials,
   const SceneInfo& sceneInfo, 
   const float x, 
   const float y, 
   float4& color )
{
   Material& material=materials[primitive.materialId.x];
   float W = (float)material.textureMapping.x;
   float H = (float)material.textureMapping.y;

   //pick some values for the constant c, this determines the shape of the Julia Set
   float cRe = -0.7f + 0.4f*sinf(sceneInfo.misc.y/1500.f);
   float cIm = 0.27015f + 0.4f*cosf(sceneInfo.misc.y/2000.f);

   //calculate the initial real and imaginary part of z, based on the pixel location and zoom and position values
   float newRe = 1.5f * (x - W / 2.f) / (0.5f * W);
   float newIm = (y - H / 2.f) / (0.5f * H);
   //i will represent the number of iterations
   int n;
   //start the iteration process
   float  maxIterations = 40.f+sceneInfo.pathTracingIteration.x;
   for(n = 0; n<maxIterations; n++)
   {
         //remember value of previous iteration
         float oldRe = newRe;
         float oldIm = newIm;
         //the actual iteration, the real and imaginary part are calculated
         newRe = oldRe * oldRe - oldIm * oldIm + cRe;
         newIm = 2.f * oldRe * oldIm + cIm;
         //if the point is outside the circle with radius 2: stop
         if((newRe * newRe + newIm * newIm) > 4.f) break;
   }
   //use color model conversion to get rainbow palette, make brightness black if maxIterations reached
   //color.x += newRe/4.f;
   //color.z += newIm/4.f;
   color.x = 1.f-color.x*(n/maxIterations);
   color.y = 1.f-color.y*(n/maxIterations);
   color.z = 1.f-color.z*(n/maxIterations);
   color.w = 1.f-(n/maxIterations);
}

__device__ void mandelbrotSet( 
	const Primitive& primitive,
	Material*        materials,
   const SceneInfo& sceneInfo, 
   const float x, 
   const float y, 
   float4& color )
{
   Material& material=materials[primitive.materialId.x];
   float W = (float)material.textureMapping.x;
   float H = (float)material.textureMapping.y;

   float  MinRe		= -2.f;
   float  MaxRe		=	1.f;
   float  MinIm		= -1.2f;
   float  MaxIm		=	MinIm + (MaxRe - MinRe) * H/W;
   float  Re_factor	=	(MaxRe - MinRe) / (W - 1.f);
   double Im_factor	=	(MaxIm - MinIm) / (H - 1.f);
   float  maxIterations = NB_MAX_ITERATIONS+sceneInfo.pathTracingIteration.x;

   float c_im = MaxIm - y*Im_factor;
   float c_re = MinRe + x*Re_factor;
   float Z_re = c_re;
   float Z_im = c_im;
   bool isInside = true;
   unsigned n;
   for( n = 0; isInside && n < maxIterations; ++n ) 
   {
      float Z_re2 = Z_re*Z_re;
      float Z_im2 = Z_im*Z_im;
      if ( Z_re2+Z_im2>4.f ) 
      {
         isInside = false;
      }
      Z_im = 2.f*Z_re*Z_im+c_im;
      Z_re = Z_re2 - Z_im2+c_re;
   }

   color.x = 1.f-color.x*(n/maxIterations);
   color.y = 1.f-color.y*(n/maxIterations);
   color.z = 1.f-color.z*(n/maxIterations);
   color.w = 1.f-(n/maxIterations);
}

/*
________________________________________________________________________________

Triangle texture Mapping
________________________________________________________________________________
*/
__device__ float4 triangleUVMapping( 
   const SceneInfo& sceneInfo,
	const Primitive& primitive,
	Material*        materials,
	BitmapBuffer*    textures,
	Vertex&          intersection,
   const Vertex&    areas,
   Vertex&          normal)
{
   Material& material=materials[primitive.materialId.x];
	float4 result = material.color;

	Vertex T = (primitive.vt0*areas.x+primitive.vt1*areas.y+primitive.vt2*areas.z)/(areas.x+areas.y+areas.z);
   int u = T.x*material.textureMapping.x;
	int v = T.y*material.textureMapping.y;

	u = u%material.textureMapping.x;
	v = v%material.textureMapping.y;
	if( u>=0 && u<material.textureMapping.x && v>=0 && v<material.textureMapping.y )
	{
      switch( material.textureIds.x )
      {
      case TEXTURE_MANDELBROT: mandelbrotSet( primitive, materials, sceneInfo, u, v, result ); break;
      case TEXTURE_JULIA: juliaSet( primitive, materials, sceneInfo, u, v, result ); break;
      default:
         {
            int A = (v*material.textureMapping.x+u)*material.textureMapping.w;
            int B = material.textureMapping.x*material.textureMapping.y*material.textureMapping.w;
		      int index = A%B;

            // Diffuse
            int i = material.textureOffset.x + index;
            BitmapBuffer r,g,b;
		      r = textures[i  ];
		      g = textures[i+1];
		      b = textures[i+2];
#ifdef USE_KINECT
            if( material.textureIds.x==0 )
            {
		         r = textures[index+2];
		         g = textures[index+1];
		         b = textures[index  ];
            }
#endif // USE_KINECT
		      result.x = r/256.f;
		      result.y = g/256.f;
		      result.z = b/256.f;

            // Normal mapping
            if( material.textureIds.y!=TEXTURE_NONE) normalMap(index, material, textures, normal);
            // Bump mapping
            if( material.textureIds.z!=TEXTURE_NONE) bumpMap(index, material, textures, intersection);
         }
      }
	}
	return result; 
}

#ifdef EXTENDED_GEOMETRY
/*
________________________________________________________________________________

Sphere texture Mapping
________________________________________________________________________________
*/
__device__ float4 sphereUVMapping( 
	const Primitive& primitive,
	Material*        materials,
	BitmapBuffer*    textures,
	Vertex&          intersection,
   Vertex&          normal)
{
   Material& material=materials[primitive.materialId.x];
	float4 result = material.color;

   float len,U,V;
   Vertex I=intersection;
   vectorRotation(I,primitive.p0,primitive.vt0);
   I = I-primitive.p0;

   float z = I.z;
   len = sqrt(I.x*I.x+I.y*I.y+I.z*I.z);
   if(len>0.0f) 
   {     
      if(I.x==0.0f && I.y==0.0f) 
      {
         U = 0.f;
      }
      else
      {
         U = (1.f - atan2(I.x,I.y)/PI)/2.f;
      }

      z/=len;
      V = 1.f-acos(z)/PI;
   }

   int u = material.textureMapping.x*U; // TODO
   int v = material.textureMapping.y*V;

   if( material.textureMapping.x != 0 ) u = u%material.textureMapping.x;
   if( material.textureMapping.y != 0 ) v = v%material.textureMapping.y;
	if( u>=0 && u<material.textureMapping.x && v>=0 && v<material.textureMapping.y )
	{
      int A = (v*material.textureMapping.x+u)*material.textureMapping.w;
      int B = material.textureMapping.x*material.textureMapping.y*material.textureMapping.w;
		int index = A%B;

      // Diffuse
      int i = material.textureOffset.x + index;
      BitmapBuffer r,g,b;
		r = textures[i  ];
		g = textures[i+1];
		b = textures[i+2];
		result.x = r/256.f;
		result.y = g/256.f;
		result.z = b/256.f;

      // Normal mapping
      if( material.textureIds.y!=TEXTURE_NONE) normalMap(index, material, textures, normal);
      // Bump mapping
      if( material.textureIds.z!=TEXTURE_NONE) bumpMap(index, material, textures, intersection);
	}
	return result; 
}

/*
________________________________________________________________________________

Cube texture mapping
________________________________________________________________________________
*/
__device__ float4 cubeMapping(
   const SceneInfo& sceneInfo,
	const Primitive& primitive, 
	Material*        materials,
	BitmapBuffer*    textures,
	Vertex&          intersection,
   Vertex&          normal)
{
   Material& material=materials[primitive.materialId.x];
	float4 result = material.color;

#ifdef USE_KINECT
	if( primitive.type.x == ptCamera )
	{
		int x = (intersection.x-primitive.p0.x+primitive.size.x)*material.textureMapping.x;
		int y = gKinectVideoHeight - (intersection.y-primitive.p0.y+primitive.size.y)*material.textureMapping.y;

		x = (x+gKinectVideoWidth)%gKinectVideoWidth;
		y = (y+gKinectVideoHeight)%gKinectVideoHeight;

		if( x>=0 && x<gKinectVideoWidth && y>=0 && y<gKinectVideoHeight ) 
		{
			int index = (y*gKinectVideoWidth+x)*gKinectVideo;
         index = index%(material.textureMapping.x*material.textureMapping.y*material.textureMapping.w);
			BitmapBuffer r = textures[index+2];
			BitmapBuffer g = textures[index+1];
			BitmapBuffer b = textures[index+0];
			result.x = r/256.f;
			result.y = g/256.f;
			result.z = b/256.f;
		}
	}
	else
#endif // USE_KINECT
	{
		int u = ((primitive.type.x == ptCheckboard) || (primitive.type.x == ptXZPlane) || (primitive.type.x == ptXYPlane))  ? 
			(intersection.x-primitive.p0.x+primitive.size.x):
		(intersection.z-primitive.p0.z+primitive.size.z);

		int v = ((primitive.type.x == ptCheckboard) || (primitive.type.x == ptXZPlane)) ? 
			(intersection.z+primitive.p0.z+primitive.size.z) :
		(intersection.y-primitive.p0.y+primitive.size.y);

      if( material.textureMapping.x != 0 ) u = u%material.textureMapping.x;
      if( material.textureMapping.y != 0 ) v = v%material.textureMapping.y;

		if( u>=0 && u<material.textureMapping.x && v>=0 && v<material.textureMapping.x )
		{
         switch( material.textureIds.x )
         {
         case TEXTURE_MANDELBROT: mandelbrotSet( primitive, materials, sceneInfo, u, v, result ); break;
         case TEXTURE_JULIA: juliaSet( primitive, materials, sceneInfo, u, v, result ); break;
         default:
            {
               int A = (v*material.textureMapping.x+u)*material.textureMapping.w;
               int B = material.textureMapping.x*material.textureMapping.y*material.textureMapping.w;
		         int index = A%B;
               int i = material.textureOffset.x + index;
               BitmapBuffer r,g,b;
		         r = textures[i  ];
		         g = textures[i+1];
		         b = textures[i+2];
			      result.x = r/256.f;
			      result.y = g/256.f;
			      result.z = b/256.f;

               // Normal mapping
               if( material.textureIds.y!=TEXTURE_NONE) normalMap(index, material, textures, normal);
               // Bump mapping
               if( material.textureIds.z!=TEXTURE_NONE) bumpMap(index, material, textures, intersection);
            }
            break;
         }
		}
	}
	return result;
}
#endif // EXTENDED_GEOMETRY

__device__ bool wireFrameMapping( float x, float y, int width, const Primitive& primitive )
{
	int X = abs(x);
	int Y = abs(y);
	int A = 100; // TODO
	int B = 100; // TODO
	return ( X%A<=width ) || ( Y%B<=width );
}
