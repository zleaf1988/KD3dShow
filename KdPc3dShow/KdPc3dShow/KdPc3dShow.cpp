#include "KdPc3dShow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QString>

#include <vtkAutoInit.h>
#include <vtkPolygon.h>

#include "vtkRenderWindow.h"

#include "vtkProperty.h"
#include "vtkPoints.h"
#include "vtkPolyVertex.h"
#include "vtkUnstructuredGrid.h"

#include"vtkPolyData.h"
#include"vtkPolyDataReader.h"
#include"vtkSurfaceReconstructionFilter.h"
#include"vtkContourFilter.h"
#include"vtkPolyDataMapper.h"
#include"vtkVertexGlyphFilter.h"
#include"vtkGenericOpenGLRenderWindow.h"
#include"vtkInteractorStyleMultiTouchCamera.h"

#include "vtkLookupTable.h"

class vtkPointPickerCallback : public vtkInteractorStyleTrackballCamera
{
public:
	static vtkPointPickerCallback * New()
	{
		return new vtkPointPickerCallback;
	}

	void setWeightPtr(void * p)
	{
		pSelf = (KdPc3dShow *)p;
	}

	void OnRightButtonDown()
	{
		int * viewPosition = this->Interactor->GetEventPosition();

		this->Interactor->GetPicker()->Pick(viewPosition[0], viewPosition[1], 0, pSelf->m_render);

		double picked[3];
		this->Interactor->GetPicker()->GetPickPosition(picked);

		pSelf->ui.lineEdit_xCod->setText(QString::number(picked[0] * 10));
		pSelf->ui.lineEdit_yCod->setText(QString::number(picked[1] * 10));
		pSelf->ui.lineEdit_zCod->setText(QString::number(picked[2]));

		vtkInteractorStyleTrackballCamera::OnRightButtonDown();
	}

private:
	KdPc3dShow * pSelf;
};

KdPc3dShow::KdPc3dShow(QWidget *parent): QMainWindow(parent)
{
	ui.setupUi(this);

	VTK_MODULE_INIT(vtkRenderingOpenGL2);
	VTK_MODULE_INIT(vtkRenderingFreeType);
	VTK_MODULE_INIT(vtkInteractionStyle);
	
	m_pcData = NULL;
	m_vtkGlWeight = NULL;
	m_mapper = NULL;
	m_actorShow = NULL;
	m_actorAxes = NULL;
	m_render = NULL;
	m_intac = NULL;
	m_camera = NULL;
	m_pointPicker = NULL;

	m_vtkGlWeight = new QVTKOpenGLWidget(this);
	m_vtkGlWeight->setObjectName(QStringLiteral("vtkGlWidget"));
	m_vtkGlWeight->setGeometry(QRect(0, 12, 800, 450));

	m_writer = vtkSmartPointer<vtkPolyDataWriter>::New();
	m_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	m_actorShow = vtkSmartPointer<vtkActor>::New();

	m_actorAxes = vtkSmartPointer<vtkAxesActor>::New();
	m_axesTransform = vtkSmartPointer<vtkTransform>::New();
	m_axesTransform->Translate(-5.0, -5.0, -20.0);

	m_render = vtkSmartPointer<vtkRenderer>::New();
	m_camera = vtkSmartPointer<vtkCamera>::New();
	m_intac = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	vtkInteractorStyleMultiTouchCamera *style = vtkInteractorStyleMultiTouchCamera::New();
	m_intac->SetInteractorStyle(style);
	m_pointPicker = vtkSmartPointer<vtkPointPicker>::New();
	m_pointPicker->SetTolerance(0.005);

	m_eleFilter = vtkElevationFilter::New();

	m_xRotAxis = m_yRotAxis = m_zRotAxis = m_rotAngle = 0;

	m_xRoiSta = m_yRoiSta = m_zRoiSta = 0;
	
	m_xRoiEnd = m_yRoiEnd = m_zRoiEnd = 0;
}

KdPc3dShow::~KdPc3dShow() 
{
	if (m_pcData != NULL) {
		delete m_pcData;
		m_pcData = NULL;
	}

	if (m_vtkGlWeight != NULL) {
		delete m_vtkGlWeight;
		m_vtkGlWeight = NULL;
	}

	m_pointPicker = NULL;
	m_writer = NULL;
	m_mapper = NULL;
	m_actorShow = NULL;
	m_actorAxes = NULL;
	m_axesTransform = NULL;
	m_render = NULL;
	m_intac = NULL;
	m_camera = NULL;
}

void KdPc3dShow::on_openFile_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("open file"), " ", tr("all file(*.*);;vtk file(*.vtk);;txt file(*.txt);;csv file(*.csv)"));

	int potNum = 0;
	if (fileName.isNull()) {
		return;
	}

	m_pcData = new PointCloud();
	potNum = m_pcData->ReadPointCloudFile(fileName);

	if (potNum <= 0) {
		delete m_pcData;
		m_pcData = NULL;
		QMessageBox::warning(this, "warning", "can not read!");
		return;
	}
	
	float xSt, xEd;
	float ySt, yEd;
	float zSt, zEd;

	m_pcData->GetPointDimension(m_row, m_col);
	m_pcData->GetPointStep(m_xStep, m_yStep, m_zStep);
	m_pcData->GetRange(xSt, xEd, ySt, yEd, zSt, zEd);

	ui.lineEdit_row->setText(QString::number(m_row));
	ui.lineEdit_col->setText(QString::number(m_col));
	ui.lineEdit_xStep->setText(QString::number(m_xStep));
	ui.lineEdit_yStep->setText(QString::number(m_yStep));
	ui.lineEdit_zStep->setText(QString::number(m_zStep));

	ui.label_xRoiRange->setText("X:" + QString::number(xSt, 'f', 1) + "--" + QString::number(xEd, 'f', 1));
	ui.label_yRoiRange->setText("Y:" + QString::number(ySt, 'f', 1) + "--" + QString::number(yEd, 'f', 1));;
	ui.label_zRoiRange->setText("Z:" + QString::number(zSt, 'f', 1) + "--" + QString::number(zEd, 'f', 1));;
	
	vtkShow();
}

void KdPc3dShow::on_saveFile_clicked()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("save file"), "", tr("*.vtk"));

	if (!fileName.isNull() && m_pcData) {
		m_writer->SetFileName((fileName.toStdString()).c_str());
		m_writer->SetInputData(m_pcData->GetPointCloudData());
		m_writer->Write();
	}
}

void KdPc3dShow::on_reShow_clicked() 
{
	//扫描行列
	int row = (ui.lineEdit_row->text()).toInt();
	int col = (ui.lineEdit_col->text()).toInt();

	//扫描步长
	float xs = (ui.lineEdit_xStep->text()).toFloat();
	float ys = (ui.lineEdit_yStep->text()).toFloat();
	float zs = (ui.lineEdit_zStep->text()).toFloat();

	//旋转
	float xRotAxis = (ui.lineEdit_xRotationAxis->text()).toFloat();
	float yRotAxis = (ui.lineEdit_yRotationAxis->text()).toFloat();
	float zRotAxis = (ui.lineEdit_zRotationAxis->text()).toFloat();
	float rotAngle = (ui.lineEdit_RotationAngle->text()).toFloat();
	
	//ROI立体区域
	float xRoiSta = (ui.lineEdit_xRoiStart->text()).toFloat();
	float xRoiEnd = (ui.lineEdit_xRoiEnd->text()).toFloat();

	float yRoiSta = (ui.lineEdit_yRoiStart->text()).toFloat();
	float yRoiEnd = (ui.lineEdit_yRoiEnd->text()).toFloat();

	float zRoiSta = (ui.lineEdit_zRoiStart->text()).toFloat();
	float zRoiEnd = (ui.lineEdit_zRoiEnd->text()).toFloat();

	//改变点云数据
	if (!m_pcData->BlVtkFile() && (row != m_row || col != m_col || xs != m_xStep || ys != m_yStep || zs != m_zStep)) {
		//m_mapper->Update();
		m_vtkGlWeight->GetRenderWindow()->RemoveRenderer(m_render);
		m_render = NULL; //空直接自动释放	
		
		m_pcData->UpdateGeometry(xs, ys, zs);

		m_render = vtkSmartPointer<vtkRenderer>::New();
		m_render->AddActor(m_actorShow);

		m_vtkGlWeight->GetRenderWindow()->AddRenderer(m_render);
		m_vtkGlWeight->GetRenderWindow()->Render();
		m_row = row;
		m_col = col; 
		m_xStep = xs;
		m_yStep = ys;
		m_zStep = zs;
	}

	//改变旋转角度
	//if (xRotAxis != m_xRotAxis || yRotAxis != m_yRotAxis || zRotAxis != m_zRotAxis || rotAngle != m_rotAngle) {
	if ((xRotAxis != 0 || yRotAxis != 0 || zRotAxis != 0) && rotAngle != 0) {
		m_actorShow->RotateWXYZ(rotAngle, xRotAxis, yRotAxis, zRotAxis);
		m_actorShow->Modified();
		m_vtkGlWeight->GetRenderWindow()->Render();
		
		m_xRotAxis = xRotAxis;
		m_yRotAxis = yRotAxis;
		m_zRotAxis = zRotAxis;
		m_rotAngle = rotAngle;
	}

	//显示ROI区域
	if ((xRoiSta != m_xRoiSta || yRoiSta != m_yRoiSta || zRoiSta != m_zRoiSta || xRoiEnd != m_xRoiEnd || yRoiEnd != m_yRoiEnd || zRoiEnd != m_zRoiEnd) && xRoiEnd > xRoiSta && yRoiEnd > yRoiSta && zRoiEnd > zRoiSta) {
				
		m_pcData->SetRoi(xRoiSta, xRoiEnd, yRoiSta, yRoiEnd, zRoiSta, zRoiEnd, 255, 0, 0);
		m_vtkGlWeight->GetRenderWindow()->Render();

		m_xRoiSta = xRoiSta;
		m_yRoiSta = yRoiSta;
		m_zRoiSta = zRoiSta;
		m_xRoiEnd = xRoiEnd;
		m_yRoiEnd = yRoiEnd;
		m_zRoiEnd = zRoiEnd;
	}
}

void KdPc3dShow::on_showDepthColor_clicked() 
{	
	//由Z坐标改变颜色
	if (ui.pushButton_showDepthColor->text() == QString::fromLocal8Bit("深度显示")) {
		m_eleFilter->SetInputData(m_pcData->GetPointCloudData());
		m_eleFilter->SetLowPoint(0, 0, 15);
		m_eleFilter->SetHighPoint(0, 0, -15);
		m_mapper->SetInputConnection(m_eleFilter->GetOutputPort());
		m_mapper->Modified();
		m_mapper->Update();
		m_vtkGlWeight->GetRenderWindow()->Render();
		ui.pushButton_showDepthColor->setText(QString::fromLocal8Bit("还原"));
	}
	else if(ui.pushButton_showDepthColor->text() == QString::fromLocal8Bit("还原")) {
		m_mapper->SetInputData(m_pcData->GetPointCloudData());
		m_mapper->Modified();
		m_mapper->Update();
		m_vtkGlWeight->GetRenderWindow()->Render();
		ui.pushButton_showDepthColor->setText(QString::fromLocal8Bit("深度显示"));
	}
}

void  KdPc3dShow::vtkShow() 
{	
	///vtkVertexGlyphFilter
	//vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
	//glyphFilter->SetInputData(m_pcData->getPointCloudData());
	//glyphFilter->Update();
	//m_mapper->SetInputConnection(glyphFilter->GetOutputPort());

	//vtkSmartPointer<vtkLookupTable> colorTab = vtkSmartPointer<vtkLookupTable>::New();
	////设置颜色表中的颜色
	//colorTab->SetNumberOfColors(256);
	//colorTab->SetHueRange(0.67, 0.0);        //色调范围从红色到蓝色
	//colorTab->Build();
	//m_mapper->SetScalarRange(0, 7);
	//m_mapper->SetLookupTable(colorTab);


	m_mapper->SetInputData(m_pcData->GetPointCloudData());
	m_actorShow->SetMapper(m_mapper);
	m_render->AddActor(m_actorShow);

	//设置坐标轴
	m_actorAxes->SetUserTransform(m_axesTransform);
	m_actorAxes->SetTotalLength(30.0, 30.0, 30.0);
	m_actorAxes->SetShaftType(0);
	m_actorAxes->SetAxisLabels(1);
	m_actorAxes->SetCylinderRadius(0.02);
	//m_actorAxes->GetXAxisCaptionActor2D()->GetCaptionTextProperty()->SetColor(1, 0, 0);
	//m_actorAxes->SetXAxisLabelText("test");
	m_render->AddActor(m_actorAxes);

	//设置camera view
	//m_camera->SetClippingRange(0.0475, 2.3786);			
	//m_camera->SetFocalPoint(0.0573, -0.2134, -0.0523);
	//m_camera->SetPosition(0, 0, 1);						
	//m_camera->ComputeViewPlaneNormal();					
	//m_camera->SetViewUp(0, 1, 0);
	//m_render->SetActiveCamera(m_camera);
	//double views[4] = { -1, -1, -1, 1 };
	//m_render->SetViewPoint(views);
	
	m_vtkGlWeight->GetRenderWindow()->AddRenderer(m_render);
	m_vtkGlWeight->GetRenderWindow()->Render();
	m_vtkGlWeight->show();
	
	//vtkSmartPointer构造New无法传参  还不如不用
	vtkSmartPointer<vtkPointPickerCallback> style = vtkSmartPointer<vtkPointPickerCallback>::New();
	style->setWeightPtr((void*)this);
	m_intac->SetInteractorStyle(style);
	
	m_intac->SetRenderWindow(m_vtkGlWeight->GetRenderWindow());
	m_intac->SetPicker(m_pointPicker);
	//m_intac->Initialize();
	//m_intac->AddObserver()

	m_intac->Start();
}