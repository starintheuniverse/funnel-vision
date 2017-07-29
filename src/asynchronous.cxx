/* =============================================================================
 * asynchronous.cxx
 * Masado Ishii
 * v0.1 2016-12-26
 * 
 * Description: Details of the keyboard capture and timer callbacks that can 
 *   hook into the VTK pipeline.
 *
 * Attributions:
 *   > Adapted from code by Hank Childs.
 * =============================================================================
 */

#include "vtkObject.h"
#include "vtkRenderWindowInteractor.h"

#include "../include/asynchronous.h"

/* --------------------------------------------------------------------
 * KeypressCallbackFunction
 * --------------------------------------------------------------------
 */
void KeypressCallbackFunction ( vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* vtkNotUsed(clientData), void* vtkNotUsed(callData) )
{
  std::cout << "Keypress callback" << std::endl;
 
  vtkRenderWindowInteractor *iren = 
    static_cast<vtkRenderWindowInteractor*>(caller);
 
  std::cout << "Pressed: " << iren->GetKeySym() << std::endl;

    /* Disabled because doesn't work... */
    /* Test: Show the camera transform matrices access. */
    //float current_modelview[16];
    //glGetFloatv(GL_MODELVIEW_MATRIX, current_modelview);
    //std::cout << "------------------------------------------" << endl;
    //mishii_PrintMatrix(std::cout, current_modelview, 4, 4);
    //std::cout << "------------------------------------------" << endl;

    /* Disabled because doesn't work... */
    /* Test: Show the camera transform matrices access. */
    //vtkRenderer *ren = iren->FindPokedRenderer(1, 0); //Some dummy coordinates that hopefully land me near the right window.
    //ren->PrintSelf(std::cout, vtkIndent(2));
    //vtkCamera *camera = ren->GetActiveCamera();
    //camera->PrintSelf(std::cout, vtkIndent(2));
}


/* --------------------------------------------------------------------
 * vtkTimerCallback member functions.
 * --------------------------------------------------------------------
 */

/*
 * New() - static factory function.
 */
vtkTimerCallback *vtkTimerCallback::New()
{
  vtkTimerCallback *cb = new vtkTimerCallback;
  cb->TimerCount = 0;
  cb->mapper = NULL;
  cb->renWin = NULL;
  cb->cam    = NULL;
  cb->angle  = 0;
  return cb;
}

/*
 * Execute()
 */
void vtkTimerCallback::Execute(vtkObject *vtkNotUsed(caller),
       unsigned long eventId, void *vtkNotUsed(callData))
{
    // THIS IS WHAT GETS CALLED EVERY TIMER
    //cout << "Got a timer!!" << this->TimerCount << endl;

    // NOW DO WHAT EVER ACTIONS YOU WANT TO DO...
    if (vtkCommand::TimerEvent == eventId)
        {
        ++this->TimerCount;
        }

    // Make a call to the mapper to make it alter how it renders...
    if (mapper != NULL)
        mapper->AdvanceAnimation();

    // Force a render...
    if (renWin != NULL)
        renWin->Render();
}

