/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageGaussianSmooth3D.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to C. Charles Law who developed this class.

Copyright (c) 1993-1995 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/
#include "vtkImageGaussianSmooth3D.h"

//----------------------------------------------------------------------------
// Description:
// This method sets up the 2 1d filters that perform the convolution.
vtkImageGaussianSmooth3D::vtkImageGaussianSmooth3D()
{
  // create the filter chain 
  this->Filter0 = new vtkImageGaussianSmooth1D;
  this->Filter1 = new vtkImageGaussianSmooth1D;
  this->Filter2 = new vtkImageGaussianSmooth1D;

  this->SetAxes(VTK_IMAGE_X_AXIS, VTK_IMAGE_Y_AXIS, VTK_IMAGE_Z_AXIS);
}


//----------------------------------------------------------------------------
// Description:
// This method sets the StandardDeviation. Both axes are the same.  
// A future simple extension could make the kernel eliptical.
void vtkImageGaussianSmooth3D::SetStandardDeviation(float std)
{
  ((vtkImageGaussianSmooth1D *)
   (this->Filter0))->SetStandardDeviation(std);
  ((vtkImageGaussianSmooth1D *)
   (this->Filter1))->SetStandardDeviation(std);
  ((vtkImageGaussianSmooth1D *)
   (this->Filter2))->SetStandardDeviation(std);

  this->Modified();
}

//----------------------------------------------------------------------------
// Description:
// This method sets the radius of the kernel in standard deviation units.
void vtkImageGaussianSmooth3D::SetRadiusFactor(float factor)
{
  ((vtkImageGaussianSmooth1D *)
   (this->Filter0))->SetRadiusFactor(factor);
  ((vtkImageGaussianSmooth1D *)
   (this->Filter1))->SetRadiusFactor(factor);
  ((vtkImageGaussianSmooth1D *)
   (this->Filter2))->SetRadiusFactor(factor);

  this->Modified();
}
