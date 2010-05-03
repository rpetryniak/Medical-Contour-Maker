#ifndef __vtkKWMyWindow_h
#define __vtkKWMyWindow_h

#include "vtkKWWindow.h"

class vtkKWRenderWidget;
class vtkKWScaleWithEntry;
class vtkActor;
class vtkContourFilter;
class vtkStructuredPointsReader;

class vtkKWMyWindow : public vtkKWWindow
{
public:
  static vtkKWMyWindow* New();
  vtkTypeRevisionMacro(vtkKWMyWindow,vtkKWWindow);

  // Callbacks
  virtual void ChangeIsoValue(double value);
  virtual void ChangeIsoOpacity(double value);
  virtual void ChangeIsoColor(double r, double g, double b);

  virtual void openVtkFileDialog();
  void openVtkFile(char *filename);
  void openVtkFileTestData();
  
protected:
  vtkKWMyWindow();
  ~vtkKWMyWindow();
  
  virtual void CreateWidget();

  vtkKWRenderWidget           *renderWidget;
  vtkKWScaleWithEntry         *isoLevelScale;
  vtkKWScaleWithEntry         *isoOpacityScale;
  vtkActor                    *contour;
  vtkContourFilter            *skinExtractor;
  vtkStructuredPointsReader   *vtkReader;
  
  int     isoLevel;
  float   isoOpacity;
  double  *rangeData;
  int     *dimData;
private:
  vtkKWMyWindow(const vtkKWMyWindow&);   // Not implemented.
  void operator=(const vtkKWMyWindow&);  // Not implemented.

  void refreshApplicationAfterDataLoad();
};

#endif
