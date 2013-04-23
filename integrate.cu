// Integrate Algorithm - An important algorithm of the Particle System API with CUDA in OpenGL.
//
// Copyright 2013 by Lee Flier

/* Use the Cuda OpenGL bindings with the runtime API.
  * Device code.
  */

#include <math.h>
#include "helper_math.h"
#include "math_constants.h"
// CUDA and CUBLAS functions

#include "vector_types.h"
#include "vector_functions.h"
#include "thrust/device_ptr.h"
#include "thrust/host_vector.h"
#include "thrust/device_vector.h"
#include "thrust/for_each.h"
#include "thrust/iterator/zip_iterator.h"
#include "thrust/sort.h"

#include "..\Particle\pVec.h"
#include "ParticleGroup.h"
#include "Particle.h"
using PAPI::pVec;
using PAPI::ParticleList;
using PAPI::Particle_t;

__constant__ float globalDamping;

//欧拉积分法
struct integrate_functor_Euler
{
    float deltaTime;

    __host__ __device__
    integrate_functor_Euler(float delta_time) : deltaTime(delta_time) {}

    __host__ __device__
    template <typename Particle_t>
    __device__
    void operator()(Particle_t &m)
    {
     // new position = old position + velocity * deltaTime
     m.vel += m.gravity * deltaTime;
     m.vel *= globalDamping;
  	  m.pos += m.vel * deltaTime;
  	  m.age +=deltaTime;
    }
};

//欧拉积分法
float cudatime=0; int n=0;
extern "C" void integrateSystem_Euler(ParticleList::iterator ibegin,ParticleList::iterator iend,float deltaTime)
{  
  unsigned int numParticles = (unsigned int)(iend - ibegin);
  Particle_t &host_particle = (*ibegin);
  Particle_t* dev_particle;
  cudaMalloc((void**)&dev_particle,numParticles*sizeof(Particle_t));
  cudaMemcpy(dev_particle,&host_particle,numParticles*sizeof(Particle_t),cudaMemcpyHostToDevice);
  thrust::device_ptr<Particle_t> d_particle(dev_particle);
  
  cudaEvent_t start, stop;
  float time=0;
  cudaEventCreate(&start);
  cudaEventCreate(&stop);
  if(n>=200 && n<400) cudaEventRecord(start,0);
  
  thrust::for_each(d_particle,d_particle+numParticles,integrate_functor_Euler(deltaTime));
  
  if (n<200 && n>=0)
  {
  	n++;
  }
  else if (n>=200 && n<400)
  {
  	cudaEventRecord(stop,0);
  	cudaEventSynchronize(stop);
  	cudaEventElapsedTime(&time,start,stop);
  	cudatime+=time;
  	cudaEventDestroy(start);
  	cudaEventDestroy(stop);
  	n++;
  }
  else if(n==400)
  {
  	std::cout<<"粒子数量为"<<numParticles<<"时，CUDA device端欧拉积分算法平均耗时："<<cudatime/(n-200)<<"ms"<<std::endl;
  	n=-1;
  }

  cudaMemcpy(&host_particle,dev_particle,numParticles*sizeof(Particle_t),cudaMemcpyDeviceToHost);
  cudaFree(dev_particle);//别忘了释放显存
}

//以下代码暂时无法解决数据对齐，暂且搁置
/*struct integrate_functor
{
    float deltaTime;

    __host__ __device__
    integrate_functor(float delta_time) : deltaTime(delta_time) {}

    template <typename Tuple>
    __device__
    void operator()(Tuple t)
    {
        //volatile float4 posData = thrust::get<0>(t);
        //volatile float4 velData = thrust::get<1>(t);
    
        // new position = old position + velocity * deltaTime
        thrust::get<0>(t) += thrust::get<1>(t) * deltaTime;

        // store new position and velocity
        //thrust::get<0>(t) = make_float4(pos, posData.w);
        //thrust::get<1>(t) = make_float4(vel, velData.w);
    }
};

 void integrateSystem(ParticleList::iterator ibegin,ParticleList::iterator iend,float deltaTime)
{
  cudaEvent_t start, stop;
  float time=0;
  cudaEventCreate(&start);
  cudaEventCreate(&stop);

  unsigned int numParticles = (unsigned int)(iend - ibegin);
  Particle_t &host_particle = (*ibegin);
  Particle_t *dev_particle;
  cudaMalloc((void**)&dev_particle,numParticles*sizeof(Particle_t));
  cudaMemcpy(dev_particle,&host_particle,numParticles*sizeof(Particle_t),cudaMemcpyHostToDevice);
  
  float3 dev_pos = make_float3(dev_particle.);

  thrust::device_ptr<float3> d_pos3((float3 *)dev_pos);
    thrust::device_ptr<float3> d_vel3((float3 *)dev_vel);

    cudaEventRecord(start,0);
  thrust::for_each(
        thrust::make_zip_iterator(thrust::make_tuple(d_pos3, d_vel3)),
        thrust::make_zip_iterator(thrust::make_tuple(d_pos3+numParticles, d_vel3+numParticles)),
        integrate_functor(deltaTime));
  cudaEventRecord(stop,0);
  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&time,start,stop);
  cudaEventDestroy(start);
  cudaEventDestroy(stop);
  printf("GPU实现的并行欧拉积分算法消耗时间: %f ms\n",time);
}*/
