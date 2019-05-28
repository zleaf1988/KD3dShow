#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_KdPc3dShow.h"
#include "pointCloud.h"

#include <QVTKOpenGLWidget.h>
#include <vtkSmartPointer.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkAxesActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPolyDataWriter.h>
#include <vtkPointPicker.h>
#include <vtkTransform.h>
#include <vtkElevationFilter.h>
#include <vtkCamera.h>

class KdPc3dShow : public QMainWindow
{
	Q_OBJECT

public:
	KdPc3dShow(QWidget *parent = Q_NULLPTR);
	
	~KdPc3dShow();
	
	Ui::KdPc3dShowClass ui;	
	
	vtkSmartPointer<vtkRenderer> m_render;	

private:

	void vtkShow();

	vtkSmartPointer<vtkPolyDataMapper> m_mapper;				
	vtkSmartPointer<vtkActor> m_actorShow;						
	vtkSmartPointer<vtkAxesActor> m_actorAxes;

	vtkSmartPointer<vtkCamera>m_camera;							
	QVTKOpenGLWidget * m_vtkGlWeight;
						
	vtkSmartPointer<vtkRenderWindowInteractor> m_intac;			
	vtkSmartPointer<vtkPolyDataWriter> m_writer;
	vtkSmartPointer<vtkPointPicker> m_pointPicker;
	vtkSmartPointer<vtkTransform> m_axesTransform;
	PointCloud * m_pcData;
	vtkSmartPointer<vtkElevationFilter> m_eleFilter;


	float m_xStep, m_yStep, m_zStep;				//x,y,z步长
	int m_row, m_col;								//x,y扫描条数
	float m_xRotAxis, m_yRotAxis, m_zRotAxis;		//旋转轴
	float m_rotAngle;								//旋转角度
	float m_xRoiSta, m_yRoiSta, m_zRoiSta;			//x,y,z ROI区域起始点
	float m_xRoiEnd, m_yRoiEnd, m_zRoiEnd;			//x,y,z ROI区域结束点

private slots:
	void on_openFile_clicked();
	void on_reShow_clicked();
	void on_saveFile_clicked();
	void on_showDepthColor_clicked();
};