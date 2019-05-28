#include "pointCloud.h"

#include <fstream>
#include "stdio.h"
#include <sstream>
#include <iostream>

#include <vtkCellData.h>
#include <vtkPointData.h>

unsigned char gWhite[3] = { 255, 255, 255 };
unsigned char gRed[3]   = { 255, 0,   0   };
unsigned char gGreen[3] = { 0,   255, 0   };
unsigned char gBlue[3]  = { 0,   0,   255 };

PointCloud::PointCloud()
{
	m_pointNum = 0;
	m_rawPoints = NULL;
	m_proPoints = NULL;
	m_polygons = NULL;
	m_cpData = NULL;
	m_reader = NULL;
	m_color = NULL;
	m_rawPoints = vtkSmartPointer<vtkPoints>::New();
	m_proPoints = vtkSmartPointer<vtkPoints>::New();
	m_polygons = vtkSmartPointer<vtkCellArray>::New();
	m_cpData = vtkSmartPointer<vtkPolyData>::New();
	m_color = vtkSmartPointer<vtkUnsignedCharArray>::New();
}

PointCloud::~PointCloud()
{
	if (m_rawPoints) {
		m_rawPoints = NULL;
	}

	if (m_proPoints) {
		m_proPoints = NULL;
	}

	if (m_polygons) {
		m_polygons = NULL;
	}

	if (m_cpData) {
		m_cpData = NULL;
	}
	if (m_reader) {
		m_reader = NULL;
	}
	if (m_color) {
		m_color = NULL;
	}
}

int PointCloud::ReadPointCloudFile(QString file)
{
	int potId = file.lastIndexOf('.');
	QString suffix = file.mid(potId, file.length() - potId);

	m_pointNum = 0;

	if (suffix == ".vtk") {
		m_pointNum = cpReadVtk(file.toStdString());
		m_proPoints = m_reader->GetOutput()->GetPoints();
		m_polygons = m_reader->GetOutput()->GetPolys();
		setColor();
		setPolyData(m_proPoints, m_polygons, m_color);
		
		return m_pointNum;
	}
	else if (suffix == ".csv") {
		m_pointNum = cpReadCsv(file.toStdString());
		setGeometry(0.1, 0.1, 1.0);
		setTopology(m_rows, m_cols);
		setColor();
		setPolyData(m_proPoints, m_polygons, m_color);
		
		return m_pointNum;
	}
	else if (suffix == ".txt") {
		m_pointNum = cpReadTxt(file.toStdString());
		return -1;
	}
	else {
		return -1;
	}
}

void PointCloud::SetRoi(float xs, float xe, float ys, float ye, float zs, float ze, unsigned char r = 255, unsigned char g = 0, unsigned char b = 0)
{
	double * ppot;
	unsigned char cor[3] = { r,   g,   b };
	for (int i = 0; i < m_pointNum; i++) {
		ppot = m_proPoints->GetPoint(i);
		if(ppot[0] <= xe && ppot[0] >= xs && ppot[1] <= ye && ppot[1] >= ys && ppot[2] <= ze && ppot[2] >= zs)
			m_color->SetTypedTuple(i, cor);
		else 
			m_color->SetTypedTuple(i, gWhite);
	}

	m_cpData->GetPointData()->SetScalars(m_color);

	m_cpData->Modified();
}

void PointCloud::UpdateGeometry(float xStep, float yStep, float zStep) 
{
	//vtkPoints输入数值之后就会自动归一化
	m_proPoints->Reset();
	
	setGeometry(xStep, yStep, zStep);
	
	m_cpData->SetPoints(m_proPoints);

	//m_cpData->Modified();
	
}

void PointCloud::UpdateTopology(int rows, int cols) 
{
	m_polygons->Reset();
	setTopology(rows, cols);
	m_cpData->SetPolys(m_polygons);
}

int PointCloud::cpReadVtk(string file) 
{
	m_reader = vtkSmartPointer<vtkPolyDataReader>::New();
	m_reader->SetFileName(file.c_str());
	m_reader->Update();
	return m_reader->GetOutput()->GetNumberOfPoints();
}

int PointCloud::cpReadCsv(string file) 
{
	m_pointNum = 0;
	ifstream cpFile(file);
	if (!cpFile.is_open()) {
		cout << "Error opening file";
		return -1;
	}
	string lineStr;
	int i = 0;
	int j = 0;
	int n = 0;

	while (getline(cpFile, lineStr)) {
		stringstream ss(lineStr);
		string str;
		j = 0;
		while (getline(ss, str, ',')) {
			float zv = atof(str.c_str());

			if (zv < -999) {
				zv = -20;
			}
			m_rawPoints->InsertNextPoint(j, i, zv);
			m_pointNum++;
			j++;

			if (m_pointNum >= KD_CLOUDPOINTS_NUM_TOTAL)
				return	m_pointNum;
		}

		if (i == 0) {
			m_cols = j;
		}
		m_rows = i++;
	}
	return m_pointNum;
}

int PointCloud::cpReadTxt(string file) 
{
	return 0;
}

int PointCloud::setGeometry(float xStep = 1.0, float yStep = 1.0, float zStep = 1.0)
{
	m_xStep = xStep;
	m_yStep = yStep;
	m_zStep = zStep;

	double * ppot;

	for (int i = 0; i < m_pointNum; i++) {
		ppot = m_rawPoints->GetPoint(i);
		m_proPoints->InsertPoint(i, ppot[0] * xStep, ppot[1] * yStep, ppot[2] * zStep);
	}

	return 0;
}

int PointCloud::setTopology(int rows, int cols)
{
	vtkSmartPointer<vtkPolygon> zPolygon;
	for (int i = 0; i < rows - 1; i++) {
		for (int j = 0; j < cols - 1; j++) {
			zPolygon = vtkSmartPointer<vtkPolygon>::New();
			zPolygon->GetPointIds()->SetNumberOfIds(4);
			zPolygon->GetPointIds()->SetId(0, i * cols + j);
			zPolygon->GetPointIds()->SetId(1, i * cols + j + 1);
			zPolygon->GetPointIds()->SetId(2, (i + 1) * cols + j + 1);
			zPolygon->GetPointIds()->SetId(3, (i + 1) * cols + j);
			m_polygons->InsertNextCell(zPolygon);
		}
	}
	return 0;
}

int PointCloud::setColor() 
{
	m_color->SetNumberOfComponents(3);
	m_color->SetName("Colors");
	for (int i = 0; i < m_pointNum; i++) {
		m_color->InsertNextTypedTuple(gWhite);
	}
	return 0;
}

int PointCloud::setPolyData(vtkSmartPointer<vtkPoints> pot, vtkSmartPointer<vtkCellArray> cel, vtkSmartPointer<vtkUnsignedCharArray> color)
{
	double * bound = pot->GetBounds(); 
	m_xMin = bound[0];
	m_xMax = bound[1];
	m_yMin = bound[2];
	m_yMax = bound[3];
	m_zMin = bound[4];
	m_zMax = bound[5];

	m_cpData->SetPoints(pot);
	m_cpData->SetPolys(cel);

	//m_cpData->GetCellData()->SetScalars(colors);
	m_cpData->GetPointData()->SetScalars(color);

	return 0;
}