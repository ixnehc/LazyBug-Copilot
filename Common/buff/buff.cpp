
#include "stdh.h"
#include "linear.h"

/*
	一元线性回归
	Y = ax + b
	dataX : 输入自变量数据
	dataY : 输入因变量数据
	ds	  : 剩余方差
*/

void LinearRegression(float *dataX ,float *dataY,int n,float &a,float &b,float &ds)
{
	if(n<2||!dataX||!dataY)
		return;

	float avgX = 0;
	float avgY = 0;
	float mulXY = 0;
	
	if(dataX){
		for(int i = 0;i<n;++i){
			avgX += dataX[i];
		}
		avgX /= float(n);
	}
	else{
		avgX = float(n)/2;
	}

	for(int i = 0;i<n;++i){
		avgY += dataY[i];
	}
	avgY /= float(n);
	


	float Lxx = 0,Lxy = 0;
	for(int i = 0;i<n;++i){
		float x = dataX?dataX[i]:float(i);
		Lxx += (x - avgX)*(x - avgX);
		Lxy += (x - avgX)*(x - avgY);
	}
	a = Lxy/Lxx;
	b = avgY - a*avgX;

	float fQ = 0;
	for(int i = 0;i<n;++i){
		float x = dataX?dataX[i]:float(i);
		float lv = a*x + b;
		fQ += (dataY[i]-lv)*(dataY[i]-lv);
	}

	ds = fQ/float(n);
}


/*
线性拟和	:	 将输入数据 划分为若干个线性段
dataX :		 输入自变量数据
dataY :		 输入因变量数据
tolerance:	 允许最大剩余方差
segs	:	 线性段划分节点的位置
nseg	:	 线性段划分节点的个数 [0...segs...n]
fa,fb,fq:    Y = ax + b; q:剩余方差   array size = nSeg 
*/
void LinearFitting(float *dataX ,float *dataY,int n,float tolerance,int *segs,float *fa,float *fb,float *fq,int &nSeg)
{
	float avgX,avgY,a,b,alast,blast,qlast;
	float sumX = 0,sumY = 0;
	float Lxx,Lxy,ds;
	int iStart = 0;

	nSeg = 0;
	int i = 0;
	while(i<n){
		sumX += dataX?dataX[i]:float(i);
		sumY += dataY[i];

		if(i-iStart<2){	//0,1
			i++;
			continue;
		}

		float nElems = float(i-iStart + 1); 
		avgX = sumX/nElems;
		avgY = sumY/nElems;
		Lxx = Lxy = 0;
		for(int k = iStart;k<=i;++k){
			float x = dataX?dataX[k]:float(k);
			Lxx += (x - avgX)*(x - avgX);
			Lxy += (x - avgX)*(dataY[k] - avgY);
		}
		a = Lxy/Lxx;
		b = avgY - a*avgX;

		float fQ = 0;
		for(int k = iStart;k<=i;++k){
			float x = dataX?dataX[k]:float(k);
			float lv = a*x + b;
			fQ += (dataY[k]-lv)*(dataY[k]-lv);
		}
		ds = fQ/nElems;

		bool bpast = true;
		if(i-iStart>2){
			//大范围测试
			bpast = (ds<tolerance);
			//小范围线性测试
			if(bpast){
				float aa,bb,dds;
				float *dx = (dataX)?dataX+i-2:NULL;
				LinearRegression(dx,dataY+i-2,3,aa,bb,dds);
				bpast = (dds<2.0f*tolerance);
			}
		}
		
		//最后一个
		if(i==n-1){
			alast = a;
			blast = b;
			qlast = ds;
			bpast = false;
			i++;
		}

		//测试通过
		if(bpast){
			alast = a;
			blast = b;
			qlast = ds;
			i++;
		}
		else{
			iStart = i;		// 产生新的线性段
			segs[nSeg] = i; // [segs[nSeg-1],segs[nSeg])
			if(fa)	//斜率
				fa[nSeg] = alast;
			if(fb)  //截距
				fb[nSeg] = blast;
			if(fq)	//剩余平均方差
				fq[nSeg] = qlast;
			nSeg++;
			sumX = 0;
			sumY = 0;
		}
		////
	}
}









