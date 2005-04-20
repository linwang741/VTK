/*=========================================================================

  Program:   Visualization Toolkit
  Module:    HierarchicalBoxPipeline.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// This example demonstrates how hierarchical box (uniform rectilinear)
// AMR datasets can be processed using the new vtkHierarchicalBoxDataSet class. 
// 
// The command line arguments are:
// -D <path> => path to the data (VTKData); the data should be in <path>/Data/

#include "vtkActor.h"
#include "vtkAMRBox.h"
#include "vtkCellDataToPointData.h"
#include "vtkContourFilter.h"
#include "vtkDebugLeaks.h"
#include "vtkHierarchicalBoxDataSet.h"
#include "vtkHierarchicalDataSetGeometryFilter.h"
#include "vtkImageData.h"
#include "vtkOutlineCornerFilter.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkTestUtilities.h"
#include "vtkRegressionTestImage.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkUniformGrid.h"
#include "vtkXMLImageDataReader.h"

int main(int argc, char* argv[])
{
  // Standard rendering classes
  vtkRenderer *ren = vtkRenderer::New();
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  renWin->AddRenderer(ren);
  vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
  iren->SetRenderWindow(renWin);

  // Since there is no AMR reader avaible yet, we will load a
  // collection of VTK files and create our own vtkHierarchicalBoxDataSet.
  // To create the files, I loaded a Chombo file with an experimental
  // Chombo reader and wrote the datasets separately.
  int i;
  vtkXMLImageDataReader* reader = vtkXMLImageDataReader::New();
  // vtkHierarchicalBoxDataSet represents hierarchical box 
  // (uniform rectilinear) AMR datasets, See the class documentation 
  // for more information.
  vtkHierarchicalBoxDataSet* hb = vtkHierarchicalBoxDataSet::New();

  for (i=0; i<16; i++)
    {
    // Here we load the 16 separate files (each containing
    // an image dataset -uniform rectilinear grid-)
    ostrstream fname;
    fname << "Data/chombo3d/chombo3d_" << i << ".vti" << ends;
    char* fstr = fname.str();
    char* cfname = 
      vtkTestUtilities::ExpandDataFileName(argc, argv, fstr);
    reader->SetFileName(cfname);
    // We have to update since we are working without a VTK pipeline.
    // This will read the file and the output of the reader will be
    // a valid image data.
    reader->Update();
    delete[] fstr;
    delete[] cfname;
    
    // We now create a vtkUniformGrid. This is essentially a simple
    // vtkImageData (not a sub-class though) with blanking. Since
    // VTK readers do not know vtkUniformGrid, we simply create our
    // own by copying from the image data.
    vtkUniformGrid* ug = vtkUniformGrid::New();
    ug->ShallowCopy(reader->GetOutput());

    // Each sub-dataset in a vtkHierarchicalBoxDataSet has an associated
    // vtkAMRBox. This is similar to extent but is stored externally
    // since it is possible to have sub-dataset nodes with NULL
    // vtkUniformGrid pointers.
    vtkAMRBox box;

    // This is a hack (do not do this at home). Normally, the
    // region (box) information should be available in the file.
    // In this case, since there is no such information available,
    // we obtain it by looking at each image data's extent.
    // -- begin hack
    int extent[6];
    double spacing[3];
    double origin[3];
    ug->GetExtent(extent);
    ug->GetSpacing(spacing);
    ug->GetOrigin(origin);

    for (int j=0; j<3; j++)
      {
      box.LoCorner[j] = 
        static_cast<int>(origin[j]/spacing[j] + extent[2*j]);
      box.HiCorner[j] = 
        static_cast<int>(origin[j]/spacing[j] + extent[2*j+1] - 1);
      }
    
    // Similarly, the level of each sub-dataset is normally 
    // available in the file. Since this is not the case, I
    // hard-coded this into the example program.
    // Level 0 = { 0 }, Level 1 = { 1 }, 
    // Level 2 = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }
    int level;
    int dsindex;
    if (i == 0)
      {
      level = 0;
      dsindex = 0;
      }
    else if (i == 1)
      {
      level = 1;
      dsindex = 0;
      }
    else
      {
      level = 2;
      dsindex = i-2;
      }
    // -- end hack

    // Given the level, index and box, add the sub-dataset to
    // hierarchical dataset.
    hb->SetDataSet(level, dsindex, box, ug);

    ug->Delete();
    }
  reader->Delete();
  
  // I hard-coded the refinement ratios. These should normally
  // be available in the file.
  hb->SetRefinementRatio(0, 2);
  hb->SetRefinementRatio(1, 2);

  // This call generates visibility (blanking) arrays that mask
  // regions of lower level datasets that overlap with regions
  // of higher level datasets (it is assumed that, when available,
  // higher level information should always be used instead of
  // lower level information)
  hb->GenerateVisibilityArrays();

  // We now create a simple pipeline.
  // AMR datasets can be processed with regular VTK filters in two ways:
  // 1. Pass through a AMR aware consumer. Since a AMR 
  //    aware mapper is not yet available, vtkHierarchicalDataSetGeometryFilter
  //    can be used
  // 2. Assign the composite executive (vtkCompositeDataPipeline) to
  //    all "simple" (that work only on simple, non-composite datasets) filters  
  vtkCellDataToPointData* c2p = vtkCellDataToPointData::New();
  c2p->SetInput(hb);

  // contour
  vtkContourFilter* contour = vtkContourFilter::New();
  contour->SetInputConnection(0, c2p->GetOutputPort(0));
  contour->SetValue(0, -0.013);

  // geometry filter
  // This filter is AMR aware and will request blocks from the
  // input. These blocks will be processed by simple processes as if they
  // are the whole dataset
  vtkHierarchicalDataSetGeometryFilter* geom1 = 
    vtkHierarchicalDataSetGeometryFilter::New();
  geom1->SetInputConnection(0, contour->GetOutputPort(0));

  // Rendering objects
  vtkPolyDataMapper* ctMapper = vtkPolyDataMapper::New();
  ctMapper->SetInputConnection(0, geom1->GetOutputPort(0));

  vtkActor* ctActor = vtkActor::New();
  ctActor->SetMapper(ctMapper);
  ren->AddActor(ctActor);

  // outline
  vtkOutlineCornerFilter* outline = vtkOutlineCornerFilter::New();
  outline->SetInput(hb);

  vtkHierarchicalDataSetGeometryFilter* geom2 = 
    vtkHierarchicalDataSetGeometryFilter::New();
  geom2->SetInputConnection(0, outline->GetOutputPort(0));
  
  // Rendering objects
  vtkPolyDataMapper* outMapper = vtkPolyDataMapper::New();
  outMapper->SetInputConnection(0, geom2->GetOutputPort(0));

  vtkActor* outActor = vtkActor::New();
  outActor->SetMapper(outMapper);
  outActor->GetProperty()->SetColor(0, 0, 0);
  ren->AddActor(outActor);

  // In the future (once the pipeline changes are finished), it
  // Standard testing code.
  ren->SetBackground(1,1,1);
  renWin->SetSize(300,300);
  iren->Start();

  // Cleanup
  geom1->Delete();
  geom2->Delete();
  ren->Delete();
  renWin->Delete();
  iren->Delete();
  ctMapper->Delete();
  ctActor->Delete();
  outMapper->Delete();
  outActor->Delete();
  outline->Delete();
  c2p->Delete();
  contour->Delete();
  hb->Delete();

  return 0;
}
