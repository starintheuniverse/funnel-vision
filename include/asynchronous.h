/* =============================================================================
 * asynchronous.h
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

#ifndef _ASYNCHRONOUS_H
#define _ASYNCHRONOUS_H

#include "vtkObject.h"

#include "vtkCommand.h"
#include "vtkRenderWindow.h"
#include "vtkCamera.h"

#include "scenemapper.h"  // To get vtk441Mapper.

/*
 * KeypressCallbackFunction (prototype)
 */
void KeypressCallbackFunction (
  vtkObject* caller,
  long unsigned int eventId,
  void* clientData,
  void* callData );


/*
 * vtkTimerCallback class
 */
class vtkTimerCallback : public vtkCommand
{
  public:
    static vtkTimerCallback *New();

    void   SetMapper(vtk441Mapper *m) { mapper = m; };
    void   SetRenderWindow(vtkRenderWindow *rw) { renWin = rw; };
    void   SetCamera(vtkCamera *c) { cam = c; };
 
    virtual void Execute(vtkObject *vtkNotUsed(caller), unsigned long eventId,
                         void *vtkNotUsed(callData));

  private:
    int TimerCount;
    vtk441Mapper *mapper;
    vtkRenderWindow *renWin;
    vtkCamera *cam;
    float angle;
};

#endif /* _ASYNCHRONOUS_H */
