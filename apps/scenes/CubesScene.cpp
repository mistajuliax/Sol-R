/* 
 * Copyright (C) 2014 Cyrille Favreau - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Cyrille Favreau <cyrille_favreau@hotmail.com>
 */

#include <stdlib.h>

#include "CubesScene.h"

CubesScene::CubesScene( const std::string& name, const int nbMaxPrimitivePerBox  )
    : Scene( name, nbMaxPrimitivePerBox )
{
}

CubesScene::~CubesScene(void)
{
}

void CubesScene::doInitialize()
{
    int s = 5000;
    for( int i(0); i<10; ++i)
    {
        float X = static_cast<float>(rand()%(2*s)-s);
        float Y = static_cast<float>(rand()%(2*s)) -2500.f;
        float Z = static_cast<float>(rand()%(2*s)-s);
        for( int j(0); j<10; j++)
        {
            float x = 0.3f*static_cast<float>(rand()%(2*s)-s);
            float y = 0.3f*static_cast<float>(rand()%(2*s));
            float z = 0.3f*static_cast<float>(rand()%(2*s)-s);
            float r = static_cast<float>(500+rand()%500);
            m_gpuKernel->addCube(
                        X+x, Y+y, Z+z, r,
                        1);
        }
    }
    m_gpuKernel->getSceneInfo().nbRayIterations = 20;
}

void CubesScene::doAnimate()
{
}

void CubesScene::doAddLights()
{
    // lights
    if( m_gpuKernel->getNbActiveLamps()==0 )
    {
        m_nbPrimitives = m_gpuKernel->addPrimitive( ptSphere ); m_gpuKernel->setPrimitive( m_nbPrimitives,  8000.f, 8000.f, -8000.f, 500.f, 0.f, 0.f, DEFAULT_LIGHT_MATERIAL); m_gpuKernel->setPrimitiveIsMovable( m_nbPrimitives, false );
    }
}
