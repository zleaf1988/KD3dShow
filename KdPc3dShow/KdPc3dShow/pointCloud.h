#ifndef __KD_CLOUD_POINTS_HPP__
#define __KD_CLOUD_POINTS_HPP__

#define KD_CLOUDPOINTS_NUM_TOTAL 1600000
#define KD_CLOUDPOINTS_STEP 3

#include <QString>

#include <vtkPoints.h>
#include <vtkPolygon.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkUnsignedCharArray.h>
#include <string>

using namespace std;

class PointCloud
{
public:
	PointCloud();

	~PointCloud();

	//������
	int ReadPointCloudFile(QString file);
	
	//��ȡ����
	inline vtkSmartPointer<vtkPolyData> GetPointCloudData() { return m_cpData; }

	//��ȡɨ������
	inline void GetPointDimension(int & row, int &col) { row = m_rows; col = m_cols; }

	//��ȡɨ�貽��
	inline void GetPointStep(float & x, float & y, float & z) { x = m_xStep; y = m_yStep; z = m_zStep;}
	
	//�Ƿ���VTK�ļ�
	inline bool BlVtkFile() { return m_reader ? true : false; }
	
	//��ȡxyz��Χ
	inline void GetRange(float &xs, float &xe, float &ys, float &ye, float &zs, float &ze) { xs = m_xMin; xe = m_xMax; ys = m_yMin; ye = m_yMax; zs = m_zMin; ze = m_zMax; }

	//����ROI
	void SetRoi(float xs, float xe, float ys, float ye, float zs, float ze, unsigned char r, unsigned char g, unsigned char b);
	
	//����
	void UpdateGeometry(float xStep, float yStep, float zStep);
	void UpdateTopology(int rows, int cols);

private:
	
	int cpReadVtk(string file);
	int cpReadCsv(string file);
	int cpReadTxt(string file);
	
	//�������弸��
	int setGeometry(float xStep, float yStep, float zStep);

	//������������
	int setTopology(int rows, int cols);

	//������ɫ
	int setColor();

	//���ü��Ρ����ˡ���ɫ
	int setPolyData(vtkSmartPointer<vtkPoints> pot, vtkSmartPointer<vtkCellArray> cel, vtkSmartPointer<vtkUnsignedCharArray> color);

	vtkSmartPointer<vtkPoints> m_rawPoints;											///ԭʼ��
	vtkSmartPointer<vtkPoints> m_proPoints;											///�����
	vtkSmartPointer<vtkCellArray> m_polygons;										///����
	vtkSmartPointer<vtkPolyData> m_cpData;											///����
	vtkSmartPointer<vtkPolyDataReader> m_reader;									///
	vtkSmartPointer<vtkUnsignedCharArray> m_color;									///��ɫ

	int m_rows;																		///����
	int m_cols;
	float m_xStep;																	///����
	float m_yStep;
	float m_zStep;

	int m_pointNum;																	///����

	float m_xMax, m_xMin;															///�㷶Χ
	float m_yMax, m_yMin;
	float m_zMax, m_zMin;
};

#endif