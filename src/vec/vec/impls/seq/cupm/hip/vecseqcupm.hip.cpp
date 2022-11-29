#include "../vecseqcupm.hpp" /*I <petscvec.h> I*/

using namespace Petsc::vec::cupm::impl;

static constexpr auto VecSeq_HIP = VecSeq_CUPM<::Petsc::device::cupm::DeviceType::HIP>{};

PetscErrorCode VecCreate_SeqHIP(Vec v)
{
  PetscFunctionBegin;
  PetscCall(VecSeq_HIP.create(v));
  PetscFunctionReturn(0);
}

/*@
  VecCreateSeqHIP - Creates a standard, sequential, array-style vector.

  Collective, Possibly Synchronous

  Input Parameters:
+ comm - the communicator, must be PETSC_COMM_SELF
- n    - the vector length

  Output Parameter:
. v - the vector

  Notes:
  Use VecDuplicate() or VecDuplicateVecs() to form additional vectors of the same type as an
  existing vector.

  This function may initialize PetscDevice, which may incur a device synchronization.

  Level: intermediate

.seealso: PetscDeviceInitialize(), VecCreate(), VecCreateSeq(), VecCreateSeqHIPWithArray(),
VecCreateMPI(), VecCreateMPIHIP(), VecDuplicate(), VecDuplicateVecs(), VecCreateGhost()
@*/
PetscErrorCode VecCreateSeqHIP(MPI_Comm comm, PetscInt n, Vec *v)
{
  PetscFunctionBegin;
  PetscCall(VecCreateSeqCUPMAsync(VecSeq_HIP, comm, n, v));
  PetscFunctionReturn(0);
}

/*@C
  VecCreateSeqHIPWithArrays - Creates a sequential, array-style vector using HIP, where the
  user provides the complete array space to store the vector values.

  Collective, Possibly Synchronous

  Input Parameters:
+ comm     - the communicator, must be PETSC_COMM_SELF
. bs       - the block size
. n        - the local vector length
. cpuarray - CPU memory where the vector elements are to be stored (or NULL)
- gpuarray - GPU memory where the vector elements are to be stored (or NULL)

  Output Parameter:
. v - the vector

  Notes:
  If the user-provided array is NULL, then VecHIPPlaceArray() can be used at a later stage to
  SET the array for storing the vector values. Otherwise, the array must be allocated on the
  device.

  If both cpuarray and gpuarray are provided, the provided arrays must have identical
  values.

  The arrays are NOT freed when the vector is destroyed via VecDestroy(). The user must free
  them themselves, but not until the vector is destroyed.

  This function may initialize PetscDevice, which may incur a device synchronization.

  Level: intermediate

.seealso: PetscDeviceInitialize(), VecCreate(), VecCreateSeqWithArray(), VecCreateSeqHIP(),
VecCreateSeqHIPWithArray(), VecCreateMPIHIP(), VecCreateMPIHIPWithArray(),
VecCreateMPIHIPWithArrays(), VecHIPPlaceArray()
C@*/
PetscErrorCode VecCreateSeqHIPWithArrays(MPI_Comm comm, PetscInt bs, PetscInt n, const PetscScalar cpuarray[], const PetscScalar gpuarray[], Vec *v)
{
  PetscFunctionBegin;
  PetscCall(VecCreateSeqCUPMWithArraysAsync(VecSeq_HIP, comm, bs, n, cpuarray, gpuarray, v));
  PetscFunctionReturn(0);
}

/*@C
  VecCreateSeqHIPWithArray - Creates a sequential, array-style vector using HIP, where the
  user provides the device array space to store the vector values.

  Collective, Possibly Synchronous

  Input Parameters:
+ comm     - the communicator, must be PETSC_COMM_SELF
. bs       - the block size
. n        - the vector length
- gpuarray - GPU memory where the vector elements are to be stored (or NULL)

  Output Parameter:
. v - the vector

  Notes:
  If the user-provided array is NULL, then VecHIPPlaceArray() can be used at a later stage to
  SET the array for storing the vector values. Otherwise, the array must be allocated on the
  device.

  The array is NOT freed when the vector is destroyed via VecDestroy(). The user must free the
  array themselves, but not until the vector is destroyed.

  Use VecDuplicate() or VecDuplicateVecs() to form additional vectors of the same type as an
  existing vector.

  This function may initialize PetscDevice, which may incur a device synchronization.

  Level: intermediate

.seealso: PetscDeviceInitialize(), VecCreate(), VecCreateSeq(), VecCreateSeqWithArray(),
VecCreateMPIWithArray(), VecCreateSeqHIP(), VecCreateMPIHIPWithArray(), VecHIPPlaceArray(),
VecDuplicate(), VecDuplicateVecs(), VecCreateGhost()
@*/
PetscErrorCode VecCreateSeqHIPWithArray(MPI_Comm comm, PetscInt bs, PetscInt n, const PetscScalar gpuarray[], Vec *v)
{
  PetscFunctionBegin;
  PetscCall(VecCreateSeqHIPWithArrays(comm, bs, n, nullptr, gpuarray, v));
  PetscFunctionReturn(0);
}

/*@C
  VecHIPGetArray - Provides access to the device buffer inside a vector

  Asynchronous

  Input Parameter:
. v - the vector

  Output Parameter:
. a - the device buffer

  Notes:
  This routine has semantics similar to VecGetArray(); the returned buffer points to a
  consistent view of the vector data. This may involve copying data from the host to the device
  if the data on the device is out of date. It is also assumed that the returned buffer is
  immediately modified, marking the host data out of date. This is similar to intent(inout) in
  fortran.

  If the user does require strong memory guarantees, they are encocuraged to use
  VecHIPGetArrayRead() and/or VecHIPGetArrayWrite() instead.

  The user must call VecHIPRestoreArray() when they are finished using the array.

  Fortran note:
  This function is not currently available from Fortran.

  Developer Notes:
  If the device memory hasn't been allocated previously it will be allocated as part of this
  routine.

  Level: intermediate

.seealso: VecHIPRestoreArray(), VecHIPGetArrayRead(), VecHIPGetArrayWrite(), VecGetArray(),
VecGetArrayRead(), VecGetArrayWrite()
@*/
PetscErrorCode VecHIPGetArray(Vec v, PetscScalar **a)
{
  PetscFunctionBegin;
  PetscCall(VecCUPMGetArrayAsync(VecSeq_HIP, v, a));
  PetscFunctionReturn(0);
}

/*@C
  VecHIPRestoreArray - Restore a device buffer previously acquired with VecHIPGetArray().

  Asynchronous

  Input Parameters:
+ v - the vector
- a - the device buffer

  Notes:
  The restored pointer is invalid after this function returns. This function also marks the
  host data as out of date. Subsequent access to the vector data on the host side via
  VecGetArray() will incur a (synchronous) data transfer.

  Fortran note:
  This function is not currently available from Fortran.

  Level: intermediate

.seealso: VecHIPGetArray(), VecHIPGetArrayRead(), VecHIPGetArrayWrite(), VecGetArray(),
VecRestoreArray(), VecGetArrayRead()
@*/
PetscErrorCode VecHIPRestoreArray(Vec v, PetscScalar **a)
{
  PetscFunctionBegin;
  PetscCall(VecCUPMRestoreArrayAsync(VecSeq_HIP, v, a));
  PetscFunctionReturn(0);
}

/*@C
  VecHIPGetArrayRead - Provides read access to the HIP buffer inside a vector.

  Asynchronous

  Input Parameter:
. v - the vector

  Output Parameter:
. a - the HIP pointer.

  Notes:
  See VecHIPGetArray() for data movement semantics of this function.

  This function assumes that the user will not modify the vector data. This is analgogous to
  intent(in) in Fortran.

  The device pointer must be restored by calling VecHIPRestoreArrayRead(). If the data on the
  host side was previously up to date it will remain so, i.e. data on both the device and the
  host is up to date. Accessing data on the host side does not incur a device to host data
  transfer.

  Fortran note:
  This function is not currently available from Fortran.

  Level: intermediate

.seealso: VecHIPRestoreArrayRead(), VecHIPGetArray(), VecHIPGetArrayWrite(), VecGetArray(),
VecGetArrayRead()
@*/
PetscErrorCode VecHIPGetArrayRead(Vec v, const PetscScalar **a)
{
  PetscFunctionBegin;
  PetscCall(VecCUPMGetArrayReadAsync(VecSeq_HIP, v, a));
  PetscFunctionReturn(0);
}

/*@C
  VecHIPRestoreArrayRead - Restore a HIP device pointer previously acquired with
  VecHIPGetArrayRead().

  Input Parameters:
+ v - the vector
- a - the HIP device pointer

  Notes:
  This routine does not modify the corresponding array on the host in any way. The pointer is
  invalid after this function returns.

  Fortran note:
  This function is not currently available from Fortran.

  Level: intermediate

.seealso: VecHIPGetArrayRead(), VecHIPGetArrayWrite(), VecHIPGetArray(), VecGetArray(),
VecRestoreArray(), VecGetArrayRead()
@*/
PetscErrorCode VecHIPRestoreArrayRead(Vec v, const PetscScalar **a)
{
  PetscFunctionBegin;
  PetscCall(VecCUPMRestoreArrayReadAsync(VecSeq_HIP, v, a));
  PetscFunctionReturn(0);
}

/*@C
  VecHIPGetArrayWrite - Provides write access to the HIP buffer inside a vector.

  Input Parameter:
. v - the vector

  Output Parameter:
. a - the HIP pointer

  Notes:
  The data pointed to by the device pointer is uninitialized. The user may not read from this
  data. Furthermore, the entire array needs to be filled by the user to obtain well-defined
  behaviour. The device memory will be allocated by this function if it hasn't been allocated
  previously. This is analogous to intent(out) in Fortran.

  The device pointer needs to be released with VecHIPRestoreArrayWrite(). When the pointer is
  released the host data of the vector is marked as out of data. Subsequent access of the host
  data with e.g. VecGetArray() incurs a device to host data transfer.

  Fortran Note:
  This function is not currently available from Fortran.

  Level: advanced

.seealso: VecHIPRestoreArrayWrite(), VecHIPGetArray(), VecHIPGetArrayRead(),
VecHIPGetArrayWrite(), VecGetArray(), VecGetArrayRead()
@*/
PetscErrorCode VecHIPGetArrayWrite(Vec v, PetscScalar **a)
{
  PetscFunctionBegin;
  PetscCall(VecCUPMGetArrayWriteAsync(VecSeq_HIP, v, a));
  PetscFunctionReturn(0);
}

/*@C
  VecHIPRestoreArrayWrite - Restore a HIP device pointer previously acquired with
  VecHIPGetArrayWrite().

  Input Parameters:
+ v - the vector
- a - the HIP device pointer.  This pointer is invalid after VecHIPRestoreArrayWrite() returns.

  Notes:
  Data on the host will be marked as out of date. Subsequent access of the data on the host
  side e.g. with VecGetArray() will incur a device to host data transfer.

  Fortran Note:
  This function is not currently available from Fortran.

  Level: intermediate

.seealso: VecHIPGetArrayWrite(), VecHIPGetArray(), VecHIPGetArrayRead(),
VecHIPGetArrayWrite(), VecGetArray(), VecRestoreArray(), VecGetArrayRead()
@*/
PetscErrorCode VecHIPRestoreArrayWrite(Vec v, PetscScalar **a)
{
  PetscFunctionBegin;
  PetscCall(VecCUPMRestoreArrayWriteAsync(VecSeq_HIP, v, a));
  PetscFunctionReturn(0);
}

/*@C
  VecHIPPlaceArray - Allows one to replace the GPU array in a vector with a GPU array provided
  by the user.

  Not Collective

  Input Parameters:
+ vec - the vector
- array - the GPU array

  Notes:
  This routine is useful to avoid copying an array into a vector, though you can return to the
  original GPU array with a call to VecHIPResetArray().

  It is not possible to use VecHIPPlaceArray() and VecPlaceArray() at the same time on the
  same vector.

  `vec` does not take ownership of `array` in any way. The user must free `array` themselves
  but be careful not to do so before the vector has either been destroyed, had its original
  array restored with `VecHIPResetArray()` or permanently replaced with
  `VecHIPReplaceArray()`.

  Level: advanced

.seealso: VecPlaceArray(), VecGetArray(), VecRestoreArray(), VecReplaceArray(),
VecResetArray(), VecHIPResetArray(), VecHIPReplaceArray()
@*/
PetscErrorCode VecHIPPlaceArray(Vec vin, const PetscScalar a[])
{
  PetscFunctionBegin;
  PetscCall(VecCUPMPlaceArrayAsync(VecSeq_HIP, vin, a));
  PetscFunctionReturn(0);
}

/*@C
  VecHIPReplaceArray - Permanently replace the GPU array in a vector with a GPU array provided
  by the user.

  Not Collective

  Input Parameters:
+ vec   - the vector
- array - the GPU array

  Notes:
  This is useful to avoid copying a GPU array into a vector.

  This frees the memory associated with the old GPU array. The vector takes ownership of the
  passed array so it CANNOT be freed by the user. It will be freed when the vector is
  destroyed.

  Fortran Notes:
  Not supported from Fortran

  Level: advanced

.seealso: VecGetArray(), VecRestoreArray(), VecPlaceArray(), VecResetArray(),
VecHIPResetArray(), VecHIPPlaceArray(), VecReplaceArray()
@*/
PetscErrorCode VecHIPReplaceArray(Vec vin, const PetscScalar a[])
{
  PetscFunctionBegin;
  PetscCall(VecCUPMReplaceArrayAsync(VecSeq_HIP, vin, a));
  PetscFunctionReturn(0);
}

/*@C
  VecHIPResetArray - Resets a vector to use its default memory.

  Not Collective

  Input Parameters:
. vec - the vector

  Notes:
  Call this after the use of VecHIPPlaceArray().

  Level: advanced

.seealso: VecGetArray(), VecRestoreArray(), VecReplaceArray(), VecPlaceArray(),
VecResetArray(), VecHIPPlaceArray(), VecHIPReplaceArray()
@*/
PetscErrorCode VecHIPResetArray(Vec vin)
{
  PetscFunctionBegin;
  PetscCall(VecCUPMResetArrayAsync(VecSeq_HIP, vin));
  PetscFunctionReturn(0);
}
