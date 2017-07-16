/* Copyright (c) 2011-2014, Cyrille Favreau
 * All rights reserved. Do not distribute without permission.
 * Responsible Author: Cyrille Favreau <cyrille_favreau@hotmail.com>
 *
 * This file is part of Sol-R <https://github.com/cyrillefavreau/Sol-R>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#pragma once

#include "../DLL_API.h"
#include "../GPUKernel.h"

class RAYTRACINGENGINE_API CudaKernel : public GPUKernel {
public:
  CudaKernel(bool activeLogging, int optimalNbOfPrimmitivesPerBox, int platform,
             int device);
  ~CudaKernel();

  void initBuffers() final;
  void cleanup() final;

public:
  void recompileKernels(const std::string &kernelCode = "");

public:
  // ---------- Devices ----------
  void initializeDevice();
  void releaseDevice();

  virtual void reshape();

  void deviceQuery();

  void resetBoxesAndPrimitives();

public:
  // ---------- Rendering ----------
  void render_begin(const float timer);
  void render_end();

public:
  virtual std::string getGPUDescription();

public:
  void setBlockSize(int x, int y, int z) {
    m_blockSize.x = x;
    m_blockSize.y = y;
    m_blockSize.z = z;
  };
  void setSharedMemSize(int sharedMemSize) { m_sharedMemSize = sharedMemSize; };

private:
  // Runtime kernel execution parameters
  int4 m_blockSize;
  int m_sharedMemSize;
};
