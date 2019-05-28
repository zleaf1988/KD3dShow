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

	//读数据
	int ReadPointCloudFile(QString file);
	
	//获取数据
	inline vtkSmartPointer<vtkPolyData> GetPointCloudData() { return m_cpData; }

	//获取扫描行列
	inline void GetPointDimension(int & row, int &col) { row = m_rows; col = m_cols; }

	//获取扫描步长
	inline void GetPointStep(float & x, float & y, float & z) { x = m_xStep; y = m_yStep; z = m_zStep;}
	
	//是否是VTK文件
	inline bool BlVtkFile() { return m_reader ? true : false; }
	
	//获取xyz范围
	inline void GetRange(float &xs, float &xe, float &ys, float &ye, float &zs, float &ze) { xs = m_xMin; xe = m_xMax; ys = m_yMin; ye = m_yMax; zs = m_zMin; ze = m_zMax; }

	//设置ROI
	void SetRoi(float xs, float xe, float ys, float ye, float zs, float ze, unsigned char r, unsigned char g, unsigned char b);
	
	//跟新
	void UpdateGeometry(float xStep, float yStep, float zStep);
	void UpdateTopology(int rows, int cols);

private:
	
	int cpReadVtk(string file);
	int cpReadCsv(string file);
	int cpReadTxt(string file);
	
	//设置物体几何
	int setGeometry(float xStep, float yStep, float zStep);

	//设置物体拓扑
	int setTopology(int rows, int cols);

	//设置颜色
	int setColor();

	//设置几何、拓扑、颜色
	int setPolyData(vtkSmartPointer<vtkPoints> pot, vtkSmartPointer<vtkCellArray> cel, vtkSmartPointer<vtkUnsignedCharArray> color);

	vtkSmartPointer<vtkPoints> m_rawPoints;											///原始点
	vtkSmartPointer<vtkPoints> m_proPoints;											///处理点
	vtkSmartPointer<vtkCellArray> m_polygons;										///区域
	vtkSmartPointer<vtkPolyData> m_cpData;											///数据
	vtkSmartPointer<vtkPolyDataReader> m_reader;									///
	vtkSmartPointer<vtkUnsignedCharArray> m_color;									///颜色

	int m_rows;																		///行列
	int m_cols;
	float m_xStep;																	///步长
	float m_yStep;
	float m_zStep;

	int m_pointNum;																	///点数

	float m_xMax, m_xMin;															///点范围
	float m_yMax, m_yMin;
	float m_zMax, m_zMin;
};

#endif