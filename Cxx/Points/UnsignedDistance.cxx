#include <vtkSmartPointer.h>
#include <vtkPLYReader.h>
#include <vtkUnsignedDistance.h>
#include <vtkImageMapToColors.h>
#include <vtkLookupTable.h>
#include <vtkImageData.h>

#include <vtkImageActor.h>
#include <vtkScalarBarActor.h>

#include <vtkImageMapper3D.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>

int main (int argc, char *argv[])
{
  if (argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " file.ply" << std::endl;
    return EXIT_FAILURE;
  }
  vtkSmartPointer<vtkPLYReader> reader =
    vtkSmartPointer<vtkPLYReader>::New();
  reader->SetFileName (argv[1]);
  reader->Update();

  double bounds[6];
  reader->GetOutput()->GetBounds(bounds);
  double range[3];
  for (int i = 0; i < 3; ++i)
  {
    range[i] = bounds[2*i + 1] - bounds[2*i];
  }

  int sampleSize = reader->GetOutput()->GetNumberOfPoints() * .00005;
  if (sampleSize < 10)
    {
    sampleSize = 10;
    }
  std::cout << "Sample size is: " << sampleSize << std::endl;
  std::cout << "Range: "
            << range[0] << ", "
            << range[1] << ", "
            << range[2] << std::endl;
  int dimension = 256;
  dimension = 128;
  double radius = range[0] * .02;
  radius = range[0] / static_cast<double>(dimension) * 5;; // ~5 voxels
  std::cout << "Radius: " << radius << std::endl;
  vtkSmartPointer<vtkUnsignedDistance> distance =
    vtkSmartPointer<vtkUnsignedDistance>::New();
  distance->SetInputConnection (reader->GetOutputPort());
  distance->SetRadius(radius);
  distance->SetDimensions(dimension, dimension, dimension);
  distance->SetBounds(
    bounds[0] - range[0] * .1,
    bounds[1] + range[0] * .1,
    bounds[2] - range[1] * .1,
    bounds[3] + range[1] * .1,
    bounds[4] - range[2] * .1,
    bounds[5] + range[2] * .1);

  // Create a lookup table that consists of the full hue circle
  // (from HSV).
  vtkSmartPointer<vtkLookupTable> hueLut =
    vtkSmartPointer<vtkLookupTable>::New();
  hueLut->SetTableRange (-.99 * radius, .99 * radius);
  hueLut->SetHueRange (.667, 0);
  hueLut->SetSaturationRange (1, 1);
  hueLut->SetValueRange (1, 1);
  hueLut->UseAboveRangeColorOn();
  hueLut->SetAboveRangeColor(0, 0, 0, 0);
  hueLut->SetNumberOfColors(5);
  hueLut->Build();
  double *last = hueLut->GetTableValue(4);
  hueLut->SetAboveRangeColor(last[0], last[1], last[2], 0);

  vtkSmartPointer<vtkImageMapToColors> sagittalColors =
    vtkSmartPointer<vtkImageMapToColors>::New();
  sagittalColors->SetInputConnection(distance->GetOutputPort());
  sagittalColors->SetLookupTable(hueLut);
  sagittalColors->Update();

  vtkSmartPointer<vtkImageActor> sagittal =
    vtkSmartPointer<vtkImageActor>::New();
  sagittal->GetMapper()->SetInputConnection(sagittalColors->GetOutputPort());
  sagittal->SetDisplayExtent(dimension/2, dimension/2, 0, dimension - 1, 0, dimension - 1);

  vtkSmartPointer<vtkImageMapToColors> axialColors =
    vtkSmartPointer<vtkImageMapToColors>::New();
  axialColors->SetInputConnection(distance->GetOutputPort());
  axialColors->SetLookupTable(hueLut);
  axialColors->Update();

  vtkSmartPointer<vtkImageActor> axial =
    vtkSmartPointer<vtkImageActor>::New();
  axial->GetMapper()->SetInputConnection(axialColors->GetOutputPort());
  axial->SetDisplayExtent(0, dimension - 1, 0, dimension - 1, dimension/2, dimension/2);

  vtkSmartPointer<vtkImageMapToColors> coronalColors =
    vtkSmartPointer<vtkImageMapToColors>::New();
  coronalColors->SetInputConnection(distance->GetOutputPort());
  coronalColors->SetLookupTable(hueLut);
  coronalColors->Update();

  vtkSmartPointer<vtkImageActor> coronal =
    vtkSmartPointer<vtkImageActor>::New();
  coronal->GetMapper()->SetInputConnection(coronalColors->GetOutputPort());
  coronal->SetDisplayExtent(0, dimension - 1, dimension/2, dimension/2, 0, dimension - 1);

  // Create a scalar bar
  vtkSmartPointer<vtkScalarBarActor> scalarBar = 
    vtkSmartPointer<vtkScalarBarActor>::New();
  scalarBar->SetLookupTable(hueLut);
  scalarBar->SetTitle("Distance");
  scalarBar->SetNumberOfLabels(5);

  // Create graphics stuff
  //
  vtkSmartPointer<vtkRenderer> ren1 =
    vtkSmartPointer<vtkRenderer>::New();
  ren1->SetBackground(.3, .4, .6);

  vtkSmartPointer<vtkRenderWindow> renWin =
    vtkSmartPointer<vtkRenderWindow>::New();
  renWin->AddRenderer(ren1);
  renWin->SetSize(512,512);

  vtkSmartPointer<vtkRenderWindowInteractor> iren =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  iren->SetRenderWindow(renWin);
  
  // Add the actors to the renderer, set the background and size
  //
  ren1->AddActor(sagittal);
  ren1->AddActor(axial);
  ren1->AddActor(coronal);
  ren1->AddActor2D(scalarBar);

  // Generate an interesting view
  //
  ren1->ResetCamera();
  ren1->GetActiveCamera()->Azimuth(120);
  ren1->GetActiveCamera()->Elevation(30);
  ren1->GetActiveCamera()->Dolly(1.5);
  ren1->ResetCameraClippingRange();

  iren->Initialize();
  iren->Start();
  std::cout << distance->GetOutput()->GetScalarRange()[0] << ", "
            << distance->GetOutput()->GetScalarRange()[1] << std::endl;
  return EXIT_SUCCESS;
}
