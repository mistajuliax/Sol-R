/* 
 * Copyright (C) 2014 Cyrille Favreau - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Cyrille Favreau <cyrille_favreau@hotmail.com>
 */

#include <math.h>
#include <stdlib.h>

#include "FractalsScene.h"

FractalsScene::FractalsScene( const std::string& name, const int nbMaxPrimitivePerBox  )
 : Scene( name, nbMaxPrimitivePerBox )
{
}

FractalsScene::~FractalsScene(void)
{
}

/*
________________________________________________________________________________

Create simple fractal scene
________________________________________________________________________________
*/
void FractalsScene::createFractals( float maxIterations, FLOAT4 center, int material )
{
   SceneInfo& sceneInfo=m_gpuKernel->getSceneInfo();
   float W = (float)sceneInfo.size.x;
   float H = (float)sceneInfo.size.y;

   float  MinRe		= -2.f;
   float  MaxRe		=	1.f;
   float  MinIm		= -1.2f;
   float  MaxIm		=	MinIm + (MaxRe - MinRe) * H/W;
   float  Re_factor	=	(MaxRe - MinRe) / (W - 1.f);
   float  Im_factor	=	(MaxIm - MinIm) / (H - 1.f);

   float step=5.f;

   for( float x=0.f; x<500.f; x+=step )
   {
      for( float y=0.f; y<500.f; y+=step )
      {
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

         if( n>5 )
         {
            m_nbPrimitives = m_gpuKernel->addPrimitive(  ptTriangle);
            m_gpuKernel->setPrimitive( m_nbPrimitives, 
               (x     -250.f)*center.w, (y     -250.f)*center.w, 0.f,
               (x+step-250.f)*center.w, (y     -250.f)*center.w, 0.f,
               (x+step-250.f)*center.w, (y+step-250.f)*center.w, 0.f,
               10.f*center.w, 0.f, 0.f,
               material+(n%40));
            m_gpuKernel->setPrimitiveBellongsToModel(m_nbPrimitives,true);

            m_nbPrimitives = m_gpuKernel->addPrimitive(  ptTriangle);
            m_gpuKernel->setPrimitive( m_nbPrimitives, 
               (x+step-250.f)*center.w, (y+step-250.f)*center.w, 0.f,
               (x     -250.f)*center.w, (y+step-250.f)*center.w, 0.f,
               (x     -250.f)*center.w, (y     -250.f)*center.w, 0.f,
               10.f*center.w, 0.f, 0.f,
               material+(n%40));
            m_gpuKernel->setPrimitiveBellongsToModel(m_nbPrimitives,true);
         }
      }
   }

}

FLOAT4 FractalsScene::MandelBox(Vertex V, Vertex Scale, float R, float S, float C)
{

   V.x /= Scale.x;
   V.y /= Scale.y;
   V.z /= Scale.z;
   V.x -= 0.5f;
   V.y -= 0.5f;
   V.z -= 0.5f;
   V.x *= R;
   V.y *= R;
   V.z *= R;
   int iter = 0;
   int maxiter = 5;
   while(m_gpuKernel->dotProduct(V,V) <= (2*2) && iter < maxiter)
   {
      if (V.x>1.f)
         V.x = 2.f-V.x;
      else if (V.x<-1.f)
         V.x = -2.f-V.x;

      if (V.y>1.f)
         V.y = 2.f-V.y;
      else if (V.y<-1.f)
         V.y = -2.f-V.y;

      if (V.z>1.f)
         V.z = 2.f-V.z;
      else if (V.z<-1.f)
         V.z = -2.f-V.z;

      float len = m_gpuKernel->vectorLength(V);
      if( len < 0.5f )
      {
         V.x *= 4.f;
         V.y *= 4.f;
         V.z *= 4.f;
      }
      else if (len < 1.f)
      {
         V.x /= (len*len);
         V.y /= (len*len);
         V.z /= (len*len);
      }
      V.x = S*V.x+C;
      V.y = S*V.y+C;
      V.z = S*V.z+C;
      iter++;
   }

   FLOAT4 color = {0.f,0.f,0.f,0.f};
   if(iter >= maxiter)
   {
      // Empty call
   }
   else
   {
      int div = maxiter / 5;
      m_nbPrimitives = m_gpuKernel->addPrimitive(  ptSphere);
      m_gpuKernel->setPrimitive( m_nbPrimitives, 
         V.x,V.y,V.z,
         Scale.x, Scale.y, Scale.z,
         1000+div);
      m_gpuKernel->setPrimitiveBellongsToModel(m_nbPrimitives,true);
      /*
      switch(iter/div)
      {
      case 0: color = Color(1,1,1,1); break;
      case 1: color = Color(0.75f,0.75f,0.75f,1); break;
      case 2: color = Color(0.5f,0.5f,0.5f,1); break;
      case 3: color = Color(0.25f,0.25f,0.25f,1); break;
      case 4: color = Color(0.1f,0.1f,0.1f,1); break;
      }
      */

   }
   return color;
}

float FractalsScene::DE(Vertex pos, const int iterations, const FLOAT2 params)
{
   float Bailout = params.x; //2000.f;
   float Power = params.y; // 1.f;

	Vertex z = pos;
	float dr = 1.f;
	float r = 0.f;
   bool add(false);

	for (int i = 0; i < iterations ; i++) {
      r = m_gpuKernel->vectorLength(z);
		if (r>Bailout) 
      {
         add = true;
         break;
      }
		
		// convert to polar coordinates
		float theta = acos(z.z/r);
		float phi = atan(z.y/z.x);
		dr =  powf( r, Power-1.f)*Power*dr + 1.f;
		
		// scale and rotate the point
		float zr = powf( r,Power);
		theta = theta*Power;
		phi = phi*Power;
		
		// convert back to cartesian coordinates
		z.x = zr*sin(theta)*cos(phi);
      z.y = sin(phi)*sin(theta);
      z.z = cos(theta);
		z.x += pos.x;
		z.y += pos.y;
		z.z += pos.z;
	}

   if( !add )
   {
      m_nbPrimitives = m_gpuKernel->addPrimitive(  ptSphere);
      m_gpuKernel->setPrimitive( m_nbPrimitives, 
         pos.x, pos.y, pos.z,
         0.01f*log(r)*r/dr, 0.f, 0.f,
         40);
      m_gpuKernel->setPrimitiveBellongsToModel(m_nbPrimitives,true);
   }
	return 0.f;
}


/**
* Decides if a point at a specific location is filled or not.  This works by iteration first checking if
* the pixel is unfilled in successively larger squares or cannot be in the center of any larger square.
* @param x is the x coordinate of the point being checked with zero being the first pixel
* @param y is the y coordinate of the point being checked with zero being the first pixel
* @return 1 if it is to be filled or 0 if it is open
*/
bool FractalsScene::isSierpinskiCarpetPixelFilled(int i, INT3 v)
{
   while(v.x>0 || v.y>0 || v.z>0) // when either of these reaches zero the pixel is determined to be on the edge 
   {
      if(v.x%i==1 && v.y%i==1 && v.z%i==1) //checks if the pixel is in the center for the current square level
         return false;
      v.x /= i; //x and y are decremented to check the next larger square level
      v.y /= i;
      v.z /= i;
   }
   return true; // if all possible square levels are checked and the pixel is not determined 
   // to be open it must be filled
}

void FractalsScene::doInitialize()
{
   m_groundHeight = -5000.f;

    FLOAT4 center = { 0.f, 0.f, 0.f, 20.f };
   int size = 22;
   int step = 1;
   int nbIterations = 5;
   float radius = 200.f;
   
   FLOAT2 params = { static_cast<float>(rand()%10000), static_cast<float>(rand()%100)/100.f };
   for( int x=0; x<=size; x+=step )
   {
      for( int y=0; y<=size; y+=step )
      {
         for( int z=0; z<=size; z+=step )
         {
            INT3 v = {x,y,z};
            if( !isSierpinskiCarpetPixelFilled(nbIterations,v) )
            {
               m_nbPrimitives = m_gpuKernel->addCube(
                  (x-size/2.f)*radius, (y-size/2.f)*radius, (z-size/2.f)*radius,
                  step*radius, 
                  39);
            }
         }
      }
   }
}

void FractalsScene::doAnimate()
{
   m_rotationAngles.x = 0.02f;
   m_rotationAngles.y = 0.01f;
   m_rotationAngles.z = 0.015f;
   m_gpuKernel->rotatePrimitives( m_rotationCenter, m_rotationAngles );
   m_gpuKernel->compactBoxes(false);
}

void FractalsScene::doAddLights()
{
	// lights
	m_nbPrimitives = m_gpuKernel->addPrimitive( ptXZPlane );  
   m_gpuKernel->setPrimitive(  m_nbPrimitives, 0.f, 5000.f, 0.f, 500.f, 0.f, 500.f, DEFAULT_LIGHT_MATERIAL); 
}
