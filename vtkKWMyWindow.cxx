#include "vtkKWMyWindow.h"

//KWWidget headers:
#include "vtkKWApplication.h"
#include "vtkKWWindow.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWSurfaceMaterialPropertyWidget.h"
#include "vtkKWSimpleAnimationWidget.h"
#include "vtkKWFrameWithScrollbar.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWWidgetsPaths.h"
#include "vtkKWChangeColorButton.h"
#include "vtkKWMenu.h"
#include "vtkKWLoadSaveDialog.h"

//VTK headers:
#include "vtkPolyDataMapper.h"
#include "vtkRenderWindow.h"
#include "vtkActor.h"
#include "vtkProperty.h"
#include "vtkStructuredPointsReader.h"
#include "vtkImageCast.h"
#include "vtkImageData.h"
#include "vtkPolyDataNormals.h"
#include "vtkContourFilter.h"
#include "vtkToolkits.h"
#include "vtkObjectFactory.h"

#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWMyWindow );
vtkCxxRevisionMacro( vtkKWMyWindow, "$Revision: 1.4 $");

//----------------------------------------------------------------------------
vtkKWMyWindow::vtkKWMyWindow()
{
  vtkReader  = vtkStructuredPointsReader::New();
  
  isoLevel   = 128;
  isoOpacity = 0.5f;
}

//----------------------------------------------------------------------------
vtkKWMyWindow::~vtkKWMyWindow()
{
  if (this->renderWidget)     this->renderWidget->Delete();
  if (this->isoLevelScale)    this->isoLevelScale->Delete();
  if (this->isoOpacityScale)  this->isoOpacityScale->Delete();
}

//----------------------------------------------------------------------------
void vtkKWMyWindow::CreateWidget()
{
  // Check if already created
  if (this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }
  // Call the superclass to create the whole widget
  this->Superclass::CreateWidget();
  vtkKWApplication *app = this->GetApplication();

  //Prepare menus:
  int index;
  vtkKWMenu *openMenu = this->GetFileMenu() ;

  index = openMenu->InsertCommand(openMenu->GetNumberOfItems()-2,"Open VTK test data", this, "openVtkFileTestData");
    openMenu->SetBindingForItemAccelerator(index, openMenu->GetParentTopLevel());
    openMenu->SetItemHelpString(index, "Open VTK test data (SciRUN tooth file).");
  index = openMenu->InsertCommand(openMenu->GetNumberOfItems()-2,"Open &Vtk File", this, "openVtkFileDialog");
    openMenu->SetBindingForItemAccelerator(index, openMenu->GetParentTopLevel());
    openMenu->SetItemHelpString(index, "Open VTK Structured Points file format.");
  openMenu->InsertSeparator (openMenu->GetNumberOfItems()-2) ;

  // Add a render widget, attach it to the view frame, and pack
  renderWidget = vtkKWRenderWidget::New();
    renderWidget->SetParent(this->GetViewFrame());
    renderWidget->Create();
    renderWidget->SetRendererBackgroundColor(0.5, 0.6, 0.8);
    renderWidget->SetRendererGradientBackground(4);

    app->Script("pack %s -expand y -fill both -anchor c -expand y", renderWidget->GetWidgetName());

  // Create ISO contour for given value
  skinExtractor = vtkContourFilter::New();
    skinExtractor->SetValue(0, isoLevel);
  vtkPolyDataNormals *skinNormals = vtkPolyDataNormals::New();
    skinNormals->SetInputConnection(skinExtractor->GetOutputPort());
    skinNormals->SetFeatureAngle(60.0);
  vtkPolyDataMapper *skinMapper = vtkPolyDataMapper::New();
    skinMapper->SetInputConnection(skinNormals->GetOutputPort());
    skinMapper->ScalarVisibilityOff();
  contour = vtkActor::New();
    contour->SetMapper(skinMapper);
    contour->GetProperty()->SetOpacity(isoOpacity);

  // Add the actor to the scene
  //renderWidget->AddViewProp(contour);
  renderWidget->ResetCamera();

  // Create a scrolled frame
  vtkKWFrameWithScrollbar *vpw_frame = vtkKWFrameWithScrollbar::New();
    vpw_frame->SetParent(this->GetMainPanelFrame());
    vpw_frame->Create();

    app->Script("pack %s -side top -fill both -expand y", vpw_frame->GetWidgetName());

  isoLevelScale = vtkKWScaleWithEntry::New();
    isoLevelScale->SetParent(vpw_frame->GetFrame());
    isoLevelScale->Create();
    isoLevelScale->SetRange(0,255);
    isoLevelScale->SetLabelText("Iso Level:");
    isoLevelScale->SetValue(isoLevel);
    isoLevelScale->SetCommand(this, "ChangeIsoValue");

  app->Script("pack %s -side top -expand n -fill x -padx 2 -pady 2", isoLevelScale->GetWidgetName());

  isoOpacityScale = vtkKWScaleWithEntry::New();
    isoOpacityScale->SetParent(vpw_frame->GetFrame());
    isoOpacityScale->Create();
    isoOpacityScale->SetRange(0, 1);
    isoOpacityScale->SetResolution(0.01);
    isoOpacityScale->SetLabelText("Opacity:");
    isoOpacityScale->SetValue(isoOpacity);
    isoOpacityScale->SetCommand(this, "ChangeIsoOpacity");

  app->Script("pack %s -side top -expand n -fill x -padx 2 -pady 2", isoOpacityScale->GetWidgetName());

  vtkKWChangeColorButton *colorIsoButton = vtkKWChangeColorButton::New();
    colorIsoButton->SetParent(vpw_frame->GetFrame());
    colorIsoButton->Create();
    colorIsoButton->SetColor( contour->GetProperty()->GetColor() );
    colorIsoButton->LabelOutsideButtonOn();
    colorIsoButton->SetLabelPositionToLeft();
    colorIsoButton->SetCommand(this, "ChangeIsoColor");

    app->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 6", colorIsoButton->GetWidgetName());

  // Create a material property editor
  vtkKWSurfaceMaterialPropertyWidget *mat_prop_widget = vtkKWSurfaceMaterialPropertyWidget::New();
    mat_prop_widget->SetParent(vpw_frame->GetFrame());
    mat_prop_widget->Create();
    mat_prop_widget->SetPropertyChangedCommand(renderWidget, "Render");
    mat_prop_widget->SetPropertyChangingCommand(renderWidget, "Render");
    mat_prop_widget->SetProperty(contour->GetProperty());

    app->Script("pack %s -side top -anchor nw -expand n -fill x", mat_prop_widget->GetWidgetName());

  // Create a simple animation widget
  vtkKWFrameWithLabel *animation_frame = vtkKWFrameWithLabel::New();
    animation_frame->SetParent(vpw_frame->GetFrame());
    animation_frame->Create();
    animation_frame->SetLabelText("Movie Creator");

    app->Script("pack %s -side top -anchor nw -expand n -fill x -pady 2", animation_frame->GetWidgetName());

  vtkKWSimpleAnimationWidget *animation_widget = vtkKWSimpleAnimationWidget::New();
    animation_widget->SetParent(animation_frame->GetFrame());
    animation_widget->Create();
    animation_widget->SetRenderWidget(renderWidget);
    animation_widget->SetAnimationTypeToCamera();

    app->Script("pack %s -side top -anchor nw -expand n -fill x", animation_widget->GetWidgetName());

  renderWidget->ResetCamera();
}

void vtkKWMyWindow::refreshApplicationAfterDataLoad()
{
  //ISO parameter
  isoLevel = (rangeData[1] - rangeData[0])/2;
  skinExtractor->SetValue(0, isoLevel);
  skinExtractor->SetInputConnection(vtkReader->GetOutputPort());
  isoLevelScale->SetRange(rangeData);

  renderWidget->AddViewProp(contour);
  renderWidget->Reset();
  renderWidget->Render();
}

void vtkKWMyWindow::openVtkFile(char *filename)
{
  vtkReader->SetFileName(filename);
  vtkReader->Update();
  
  rangeData = ((vtkImageData*)vtkReader->GetOutput())->GetScalarRange();
  dimData   = ((vtkImageData*)vtkReader->GetOutput())->GetDimensions();
  
  refreshApplicationAfterDataLoad();
}

void vtkKWMyWindow::openVtkFileTestData()
{
  openVtkFile("../test_data/tooth.vtk");
}

void vtkKWMyWindow::openVtkFileDialog()
{
  vtkKWLoadSaveDialog *open_dialog = vtkKWLoadSaveDialog::New();
  open_dialog->SetParent(this->GetParentTopLevel());
  open_dialog->RetrieveLastPathFromRegistry("OpenFilePath");
  open_dialog->Create();
  open_dialog->SetTitle("Open VTK file");
  int res = open_dialog->Invoke();
  
  if (res)
  {
    char *filename = open_dialog->GetFileName();
    openVtkFile(filename);
    open_dialog->SaveLastPathToRegistry("OpenFilePath");
  }
  open_dialog->Delete();
}

void vtkKWMyWindow::ChangeIsoValue(double value)
{
  isoLevel = this->isoLevelScale->GetValue();
  
  skinExtractor->SetValue(0, isoLevel);
  renderWidget->Render();
}

void vtkKWMyWindow::ChangeIsoOpacity(double value)
{
  isoOpacity = this->isoOpacityScale->GetValue();
  
  contour->GetProperty()->SetOpacity(isoOpacity);
  renderWidget->Render();
}

void vtkKWMyWindow::ChangeIsoColor(double r, double g, double b)
{
  contour->GetProperty()->SetColor( r,g,b );
  renderWidget->Render();
}
