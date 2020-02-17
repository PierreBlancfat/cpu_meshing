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

__kernel void dists_kernel(
    __global float* a,
    __global float* b,
    __global float* dists,
    __global float onex,
    __global float oney,
    __global float twox,
    __global float twoy,
    const unsigned int count)
{
  int i = get_global_id(0);
  if(i < count)  {
    dists[i] = a[i] + b[i];
  }
}
