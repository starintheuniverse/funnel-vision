
/* =============================================================================
 * main.cxx
 * Masado Ishii
 * v0.1 2016-12-26
 * 
 * Description: Instantiates and hooks up components of the VTK pipeline, 
 *   including the custom vtk441MapperMishii. Configures event loop and keyboard 
 *   input capture. Affects the GL configuration, such as the presence of the 
 *   stencil buffer.
 *
 * Attributions:
 *   > Modified from Hank Childs's pipeline harness.
 * =============================================================================
 */


#include "vtkSmartPointer.h"
#include "vtkSphereSource.h"
#include "vtkCallbackCommand.h"
#include "vtkCommand.h"
#include "vtkActor.h"
#include "vtkInteractorStyle.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkProperty.h"
//#include "vtkCamera.h"
#include "vtkLight.h"
//#include "vtkJPEGReader.h"
//#include "vtkImageData.h"

#include "../include/scenemapper.h"   // vtk441MapperMishii
#include "../include/asynchronous.h"  // KeypressCallbackFunction, vtkTimerCallback


int main()
{
  // Dummy input so VTK pipeline mojo is happy.
  //
  vtkSmartPointer<vtkSphereSource> sphere =
    vtkSmartPointer<vtkSphereSource>::New();
  sphere->SetThetaResolution(100);
  sphere->SetPhiResolution(50);

  // The mapper is responsible for pushing the geometry into the graphics
  // library. It may also do color mapping, if scalars or other attributes
  // are defined. 
  //

  vtkSmartPointer<vtk441MapperMishii> winMapper =
    vtkSmartPointer<vtk441MapperMishii>::New();
  winMapper->SetInputConnection(sphere->GetOutputPort());

  vtkSmartPointer<vtkActor> winActor =
    vtkSmartPointer<vtkActor>::New();
  winActor->SetMapper(winMapper);

  vtkSmartPointer<vtkRenderer> ren =
    vtkSmartPointer<vtkRenderer>::New();

  vtkSmartPointer<vtkRenderWindow> renWin =
    vtkSmartPointer<vtkRenderWindow>::New();
  renWin->AddRenderer(ren);
  ren->SetViewport(0.0, 0.0, 1.0, 1);
  renWin->StencilCapableOn();    // Important for proper portals.

  vtkSmartPointer<vtkRenderWindowInteractor> iren =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  iren->SetRenderWindow(renWin);

  // Add the actor(s) to the renderer, set the background and size.
  //
  ren->AddActor(winActor);
  ren->SetBackground(0.0, 0.0, 0.0);
  renWin->SetSize(1200, 600);

  // Set up the lighting.
  //
     ren->GetActiveCamera()->SetFocalPoint(0,0,0);
     ren->GetActiveCamera()->SetPosition(0,0,70);
     ren->GetActiveCamera()->SetViewUp(0,1,0);
     ren->GetActiveCamera()->SetClippingRange(20, 120);
     ren->GetActiveCamera()->SetDistance(70);
  
  // This starts the event loop and invokes an initial render.
  //
  ((vtkInteractorStyle *)iren->GetInteractorStyle())->SetAutoAdjustCameraClippingRange(0);
  iren->Initialize();

  // Sign up to receive TimerEvent
  vtkSmartPointer<vtkTimerCallback> cb = 
    vtkSmartPointer<vtkTimerCallback>::New();
  iren->AddObserver(vtkCommand::TimerEvent, cb);
  cb->SetMapper(winMapper);
  cb->SetRenderWindow(renWin);
  cb->SetCamera(ren->GetActiveCamera());
 
  vtkSmartPointer<vtkCallbackCommand> keypressCallback = 
    vtkSmartPointer<vtkCallbackCommand>::New();
  keypressCallback->SetCallback ( KeypressCallbackFunction );
  iren->AddObserver ( vtkCommand::KeyPressEvent, keypressCallback );

  int timerId = iren->CreateRepeatingTimer(10);  // repeats every 10 milliseconds <--> 0.01 seconds
  std::cout << "timerId: " << timerId << std::endl;  
 
  iren->Start();

  return EXIT_SUCCESS;
}
