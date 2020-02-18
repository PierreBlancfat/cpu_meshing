/* ----------------------------------------------------------------
 **
 ** kernel:  dists_kernel
 **
 ** Purpose: Compute the elementwise sum c = a+b
 **
 ** input: a and b float vectors of length count
 **
 ** output: c float vector of length count holding the sum a + b
 **
 ** ----------------------------------------------------------------
 */


#ifdef cl_khr_fp64
    #pragma OPENCL EXTENSION cl_khr_fp32 : enable
#elif defined(cl_amd_fp64)
    #pragma OPENCL EXTENSION cl_amd_fp32 : enable
#else
    #error "Double precision floating point not supported by OpenCL implementation."
#endif

#pragma OPENCL EXTENSION cl_khr_fp64 : enable


__kernel void dists_kernel(
    __global float* a,
    __global float* b,
    __global float* dists,
    __global float* onex,
    __global float* oney,    
    __global float* twox,    
    __global float* twoy,    
    const unsigned int count)
{

  int i = get_global_id(0);

  if(i < count)  {
    float ab_x = *onex - *twox;
    float ab_y = *oney - *twoy;
    float ac_x, ac_y, bc_x, bc_y,n_ab, n_ac, n_bc;
    ac_x = *onex - a[i];
    ac_y = *oney - b[i];
    bc_x = a[i] - *twox;
    bc_y = b[i] - *twoy;
    n_ab = pow(ab_x,2) + pow(ab_y,2);
    n_ac = pow(ac_x,2) + pow(ac_y,2);
    n_bc = pow(bc_x,2) + pow(bc_y,2);

    dists[i] =  (n_ac * n_ab * n_bc) / sqrt((n_ab+n_ac+n_bc)*(n_ac+n_bc-n_ab)*(n_bc+n_ab-n_ac)*(n_ab+n_ac-n_bc));

  }
}
