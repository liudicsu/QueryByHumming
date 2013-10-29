#include "DTW.h"
#include "emd.h"
#include "lib.h"

const int inf=100000;
const int Dpenalty=0.0;
const int emdLength=23000; //ƥ��5.5�볤�ȵ�EMD
const double penalty=0;
const double disConst=4.0;
const double disConstString=4.0;//RA�Ĺ̶�����  ԭʼ��16
const double disConstLSHRA=9.0;
//string CurrentFileName;
map<int,vector<int>> samePoint;
map<string,int> ListSong;
map<string,vector <float>> ResultDis1;
map<string,vector <string>> ResultLable1;
map<string,vector <float>> ResultDis2;
map<string,vector <string>> ResultLable2;
map<string,vector <float>> ResultDis3;
map<string,vector <string>> ResultLable3;
HANDLE hMutex;
CRITICAL_SECTION g_cs;


float distemd(feature_t *F1, feature_t *F2)
{
	return abs( *F1 - *F2); 
}

double MyMin(double a, double b, double c)
{
	double min;
	if (a<b)
	{
		min=a;
	}
	else
	{
		min=b;
	}
	if (c<min)
	{
		min=c;
	}
	return min;
}

//��������Сֵ
double MyMinTwo(double a, double b)
{
	return a<b?a:b;
}


bool sortRule(const pair<float,int>& s1, const pair<float,int>& s2) 
{
  return s1.first < s2.first;
}

int MyMinTwoInt(int a, int b)
{
	int min;
	if (a<b)
	{
		min=a;
	}
	else
	{
		min=b;
	}
	return min;
}

double MyDistance(double &a, double &b)
{

	double D=0;

		//D+=pow(a[i]-b[i],2);
	D+=abs(a-b);

	return D;
	//return sqrt(D);
}

double PitchDistance(vector<vector<double>> &a, vector<vector<double>> &b)
{
	int i,j,m,n,k;
	m=MyMinTwoInt(a.size(),b.size());
	if (m>0)
	{
		n=a[0].size();
	}
	else
		n=0;
	double D=0;
	double disPitch=0;
	for (i=0;i<m;i++)
	{
		disPitch=0;
		for (j=0;j<n;j++)
		{
			disPitch+=MyMinTwo(pow(a[i][j]-b[i][j],2),disConstLSHRA);
		}
		D+=disPitch;
	}
	//D/=m;
	return D;
	//return sqrt(D);
}


//ȥ�������е���ֵ
void NoZero( vector<float>  &x)
{
	vector <float>::iterator iter=x.begin();
	for (;iter!=x.end();)
	{
		if (*iter==0)
			iter=x.erase(iter);
		else
			iter++;
	}
}


void NoZeroOldtow(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;
	int Nozero=0;
	int vec=0;

	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			if(x[i][j]!=0)
			{
				;
			}
			else
			{
				x[i].erase(x[i].begin()+j);
			}
		}
	}
	vector <vector <double>>::iterator iter=x.begin();
	for (;iter!=x.end();)
	{
		if (iter->empty())
		{
			x.erase(iter);
			iter=x.begin();
		}
		else
			iter++;
	}
}



void ZeroToForwardThreshold(vector< vector<double> > &x , int BeginFrame)
{
	double mean=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;
	int Nozero=0;
	int vec=0;
	int Threshold=0;//�о�ʲôʱ��ʼ��



	/*ofstream outf("datail.txt",ofstream::app);
	for (i=0;i<m;i++)
	{
		outf<<i<<":"<<x[i][0]<<" ";
	}
	outf<<endl;*/
	vector <vector <double>>::iterator iter=x.begin();
	for (i=0;i<BeginFrame;i++)
	{
		
		x.erase(iter);
		iter=x.begin();
			

	}
	m=x.size();
	if	(m>0)
	{
		if (x[0][0] !=0 )
		{
			Threshold=1;
		}

	}
	for (i=0;i<m-6 && Threshold==0;i++)
	{
		
		x[i].erase(x[i].begin());
		/*int numZero=0;
		for (j=i;j<i+12 && j<m-6;j++)
		{
			if (x[j+1][0] ==0)
			{
				numZero++;
			}
		}*/
		/*if (numZero<6 && x[i+1][0] !=0 )
		{
			Threshold=1;
		}*/
		if (x[i+1][0] !=0/* && x[i+2][0] !=0*/ )
		{
			Threshold=1;
		}


	}
	iter=x.begin();
	for (;iter!=x.end();)
	{
		if (iter->empty())
		{
			x.erase(iter);
			iter=x.begin();
		}
		else
			iter++;

	}
	m=x.size();

	for (i=1;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			if(x[i][j]!=0)
			{			
				;
			}
			else
			{
				x[i][j]=x[i-1][j];
			}
		}
	}
}


void ZeroToForwardThresholdAndLongZero(vector< vector<double> > &x , int BeginFrame)////ȥ���м��Լ���β�ľ�������
{
	double mean=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;
	int Nozero=0;
	int vec=0;
	int Threshold=0;//�о�ʲôʱ��ʼ��

	vector <vector <double>>::iterator iter=x.begin();
	for (i=0;i<BeginFrame;i++)
	{

		x.erase(iter);
		iter=x.begin();
	}
	m=x.size();
	if	(m>0)
	{
		if (x[0][0] !=0 )
		{
			Threshold=1;
		}

	}
	for (i=0;i<m-6 && Threshold==0;i++)
	{

		x[i].erase(x[i].begin());
		/*int numZero=0;
		for (j=i;j<i+12 && j<m-6;j++)
		{
		if (x[j+1][0] ==0)
		{
		numZero++;
		}
		}*/
		/*if (numZero<6 && x[i+1][0] !=0 )
		{
		Threshold=1;
		}*/
		if (x[i+1][0] !=0/* && x[i+2][0] !=0*/ )
		{
			Threshold=1;
		}


	}
	iter=x.begin();
	for (;iter!=x.end();)
	{
		if (iter->empty())
		{
			x.erase(iter);
			iter=x.begin();
		}
		else
			iter++;

	}
	m=x.size();
	int ZeroSequenceNum=0;
	int LastZero=-10;//�ж���һ��������
	int ZeroBegin=-10;
	bool ExistSilence=FALSE;
	bool NoSilence=FALSE;

	while (!NoSilence)//ȥ���м��Լ���β�ľ�������
	{
		m=x.size();
		ExistSilence=FALSE;
		for (i=0;i<m-1 && ExistSilence==FALSE;i++)
		{
			if (i==m-2)
			{
				NoSilence=TRUE;
			}
			if (x[i][0] ==0 /*|| x[i+1][0] ==0*/)
			{
				if (LastZero+1==i)//�����һ������
				{
					LastZero=i;
					ZeroSequenceNum++;
				}
				else//����ϴβ����㣬�������ţ�����ZeroSequenceNum����
				{
					LastZero=i;
					ZeroBegin=i;//�����￪ʼ�Ǿ���
					ZeroSequenceNum=0;
				}
			}
			if (ZeroSequenceNum>25)
			{
				
				ExistSilence=TRUE;
				//for (j=ZeroBegin;j<m-1 && (x[j][0] ==0 || x[j+1][0] ==0);j++)
				/*for (j=ZeroBegin;j<m-1 && x[j][0] ==0 ;j++)
				{

					x[j].erase(x[j].begin());
				}*/
				for (j=ZeroBegin;j<m-1 && x[j][0] ==0 && j<ZeroBegin+ZeroSequenceNum-15;j++)//ֻɾ��15������
				{

					x[j].erase(x[j].begin());
				}
				LastZero=-10;
				ZeroBegin=-10;
				ZeroSequenceNum=0;
			}
			if (ExistSilence==TRUE)
			{
				iter=x.begin();
				for (;iter!=x.end();)
				{
					if (iter->empty())
					{
						x.erase(iter);
						iter=x.begin();
					}
					else
						iter++;

				}
			}

		}
	}
	
	m=x.size();

	for (i=1;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			if(x[i][j]!=0)
			{			
				;
			}
			else
			{
				x[i][j]=x[i-1][j];
			}
		}
	}
}




void ZeroToForwardThresholdAndLongZeroToHalfBefor( vector<float>  &x , int BeginFrame)//�����м��Լ���β�����㰴������Ϊǰ���������
{
	float mean=0;
	int m=x.size();
	int i,j;
	int Nozero=0;
	int vec=0;
	int Threshold=0;//�о�ʲôʱ��ʼ��

	vector <float>::iterator iter=x.begin();
	for (i=0;i<BeginFrame;i++)
	{

		x.erase(iter);
		iter=x.begin();


	}
	m=x.size();
	if	(m>0)
	{
		if (x[0] !=0 )
		{
			Threshold=1;
		}

	}
	for (i=0;i<m-6 && Threshold==0;i++)
	{
		if (x[i+1] !=0/* && x[i+2][0] !=0*/ )
		{
			Threshold=1;
		}


	}
	iter=x.begin();
	for (;iter!=x.end();)
	{
		if (*iter==0)
		{
			x.erase(iter);
			iter=x.begin();
		}
		else
			iter++;

	}
	m=x.size();
	int ZeroSequenceNum=0;
	int LastZero=-10;//�ж���һ��������
	int ZeroBegin=-10;
	bool ExistSilence=FALSE;
	bool NoSilence=FALSE;

	while (!NoSilence)//ȥ���м��Լ���β�ľ�������
	{
		m=x.size();
		ExistSilence=FALSE;
		for (i=0;i<m-1 && ExistSilence==FALSE;i++)
		{
			if (i==m-2)
			{
				NoSilence=TRUE;
			}
			if (x[i] ==0 /*|| x[i+1][0] ==0*/)
			{
				if (LastZero+1==i)//�����һ������
				{
					LastZero=i;
					ZeroSequenceNum++;
				}
				else//����ϴβ����㣬�������ţ�����ZeroSequenceNum����
				{
					LastZero=i;
					ZeroBegin=i;//�����￪ʼ�Ǿ���
					ZeroSequenceNum=0;
				}
			}
			if (ZeroSequenceNum>25)//����25֡Ϊ������
			{

				ExistSilence=TRUE;
				//for (j=ZeroBegin;j<m-1 && (x[j][0] ==0 || x[j+1][0] ==0);j++)
				for (j=ZeroBegin;j<m-1 && x[j] ==0 && j<ZeroBegin+ZeroSequenceNum-10;j++)//ֻɾ��10������
				{

					x[j]=0;
				}
				LastZero=-10;
				ZeroBegin=-10;
				ZeroSequenceNum=0;
			}
			if (ExistSilence==TRUE)
			{
				iter=x.begin();
				for (;iter!=x.end();)
				{
					if (*iter==0)
					{
						x.erase(iter);
						iter=x.begin();
					}
					else
						iter++;

				}
			}

		}
	}
	ZeroSequenceNum=0;
	LastZero=-10;//�ж���һ��������
	ZeroBegin=-10;
    ExistSilence=FALSE;
	NoSilence=FALSE;
	bool ZeroExist=FALSE;
	while (!NoSilence)//�������ְ�������Ϊǰ��֡���ߺ���֡
	{
		m=x.size();
		ExistSilence=FALSE;
		for (i=0;i<m-1 && ExistSilence==FALSE;i++)
		{
			if (i==m-2)
			{
				NoSilence=TRUE;
			}
			if (x[i] ==0 /*|| x[i+1][0] ==0*/)
			{
				if (LastZero+1==i)//�����һ������
				{
					LastZero=i;
					ZeroSequenceNum++;
				}
				else//����ϴβ����㣬�������ţ�����ZeroSequenceNum����
				{
					LastZero=i;
					ZeroBegin=i;//�����￪ʼ�Ǿ���
					ZeroSequenceNum=0;//�µľ�����ʼ
				}
			}
			if (x[i] ==0 && x[i+1] !=0)
			{
				if (i>ZeroSequenceNum)
				{
					for (int k=i-ZeroSequenceNum;k<=i-ZeroSequenceNum/2;k++)
					{
						x[k]=x[k-1];
					}
					for (int k=i;k>i-ZeroSequenceNum/2;k--)
					{
						x[k]=x[k+1];
					}
				}
				ExistSilence=TRUE;
			}

		}
	}

	m=x.size();
	ZeroSequenceNum=0;
	LastZero=-10;//�ж���һ��������
	ZeroBegin=-10;
	ExistSilence=FALSE;
	NoSilence=FALSE;

	//while (!NoSilence)//ȥ����β�ľ������֣������������10֡��
	//{
	//	m=x.size();
	//	ExistSilence=FALSE;
	//	for (i=0;i<m-1 && ExistSilence==FALSE;i++)
	//	{
	//		if (i==m-2)
	//		{
	//			NoSilence=TRUE;
	//		}
	//		if (x[i][0] ==0 /*|| x[i+1][0] ==0*/)
	//		{
	//			if (LastZero+1==i)//�����һ������
	//			{
	//				LastZero=i;
	//				ZeroSequenceNum++;
	//			}
	//			else//����ϴβ����㣬�������ţ�����ZeroSequenceNum����
	//			{
	//				LastZero=i;
	//				ZeroBegin=i;//�����￪ʼ�Ǿ���
	//				ZeroSequenceNum=0;
	//			}
	//		}
	//		if (ZeroSequenceNum>10)
	//		{

	//			ExistSilence=TRUE;
	//			//for (j=ZeroBegin;j<m-1 && (x[j][0] ==0 || x[j+1][0] ==0);j++)
	//			for (j=ZeroBegin;j<m-1 && x[j][0] ==0 && j<ZeroBegin+ZeroSequenceNum-6;j++)//ֻɾ��6������
	//			{

	//				x[j].erase(x[j].begin());
	//			}
	//			LastZero=-10;
	//			ZeroBegin=-10;
	//			ZeroSequenceNum=0;
	//		}
	//		if (ExistSilence==TRUE)
	//		{
	//			iter=x.begin();
	//			for (;iter!=x.end();)
	//			{
	//				if (iter->empty())
	//				{
	//					x.erase(iter);
	//					iter=x.begin();
	//				}
	//				else
	//					iter++;

	//			}
	//		}

	//	}
	//}

	m=x.size();

	for (i=1;i<m;i++)
	{
	
		if(x[i]!=0)
		{			
			;
		}
		else
		{
			x[i]=x[i-1];
		}

	}
}

void ZeroToForward(vector< vector<double> > &x )
{
	double mean=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;
	int Nozero=0;
	int vec=0;
	int Threshold=0;//�о�ʲôʱ��ʼ��

	for (i=0;i<m-6 && Threshold==0;i++)
	{
		
		x[i].erase(x[i].begin());
		int numZero=0;
		for (j=i;j<i+12 && j<m-6;j++)
		{
			if (x[j+1][0] ==0)
			{
				numZero++;
			}
		}

		if (x[i+1][0] !=0 && x[i+2][0] !=0)
		{
			Threshold=1;
		}


	}
	vector <vector <double>>::iterator iter=x.begin();
	for (;iter!=x.end();)
	{
		if (iter->empty())
		{
			x.erase(iter);
			iter=x.begin();
		}
		else
			iter++;

	}
	/*vector <vector <double>>::iterator iter=x.begin();
	for (;iter!=x.begin()+BeginFrame;)
	{

		x.erase(iter);


	}*/
	m=x.size();

	for (i=1;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			if(x[i][j]!=0)
			{			
				;
			}
			else
			{
				x[i][j]=x[i-1][j];
			}
		}
	}
}



void MeanTowToOne(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;
	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		for (j=0;j<m;j++)
		{
			mean+=x[j][i]/m;
		}
		XMean.push_back(mean);
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=XMean[j];
				;
			}
		}
		//if (x[i].empty())
		//{
		//	x.erase(x.begin()+i);
		//}
	}  //��ֵ����
	for (i=0;i<n;i++)
	{
		for (j=1;j<m;j++,j++)
		{
			x[j][i]=(x[j][i]+x[j-1][i])/2;
			x[j-1][i]=0;
		}
	}
	//NoZero(x);
	//smooth(x);
}


double Scale(vector< vector<double> > &x)
{
	int i,j,a0=0,a1=0,a2=0,a3=0,a4=0,a5=0,a6=0,a7=0,a8=0,a9=0;
	int m=x.size();
	int n=x[0].size();
	for (i=0;i<n;i++)
	{
		for (j=4;j<m-4;j++)
		{
			if (x[j][i]-int(x[j][i])>0.9)
			{
				a9++;
			}
			if (x[j][i]-int(x[j][i])>0.8)
			{
				a8++;
			}
			if (x[j][i]-int(x[j][i])>0.7 && x[j][i]-int(x[j][i])<0.9)
			{
				a7++;
			}
			if (x[j][i]-int(x[j][i])>0.6 && x[j][i]-int(x[j][i])<0.8)
			{
				a6++;
			}
			if (x[j][i]-int(x[j][i])>0.5 && x[j][i]-int(x[j][i])<0.7)
			{
				a5++;
			}
			if (x[j][i]-int(x[j][i])>0.4 && x[j][i]-int(x[j][i])<0.6)
			{
				a4++;
			}
			if (x[j][i]-int(x[j][i])>0.3 && x[j][i]-int(x[j][i])<0.5)
			{
				a3++;
			}
			if (x[j][i]-int(x[j][i])>0.2 && x[j][i]-int(x[j][i])<0.4)
			{
				a2++;
			}
			if (x[j][i]-int(x[j][i])>0.1 && x[j][i]-int(x[j][i])<0.3)
			{
				a1++;
			}
			if (x[j][i]-int(x[j][i])<0.2)
			{
				a0++;
			}

		}
	}
	vector <int>dis;
	dis.push_back(a0);
	dis.push_back(a1);
	dis.push_back(a2);
	dis.push_back(a3);
	dis.push_back(a4);
	dis.push_back(a5);
	dis.push_back(a6);
	dis.push_back(a7);
	dis.push_back(a8);
	dis.push_back(a9);
	vector <int> ::iterator iter;

	iter=max_element(dis.begin(),dis.end());
	int pos=iter-dis.begin();
	double scale_my=(pos+1)*0.1;
	return scale_my;

}

void MeanInt(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=x[0].size();
	int Nozero=0;
	int vec=0;
	int i,j;
	//for (i=10;i<m-10;i++)
	//{
	//	for (j=0;j<n;j++)
	//	{
	//		if(x[i][j]!=0)
	//		{			
	//			vec=1;
	//		}
	//	}
	//	if (vec==1)
	//	{
	//		Nozero++;
	//	}
	//	vec=0;
	//}
	//ofstream outf("datail.txt",ofstream::app);
	//double variance;
	//vector<double> XVariance;
	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		for (j=8;j<m-8;j++)
		{
			mean+=x[j][i]/(m-16);
		}
		//mean=int(mean+0.5);
		XMean.push_back(mean);
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=XMean[j];
				//cout<<i<<","<<x[i][j]<<" ";
				//outf<<i<<","<<x[i][j]<<" ";
				if (x[i][j]>10)
				{
					x[i][j]=x[i][j]-12;
				}
				if (x[i][j]<-10)
				{
					x[i][j]=x[i][j]+12;
				}
			}
		}
	}  //��ֵ����
	double scale=Scale(x);
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=scale;
				if (x[i][j]-(int)x[i][j]>0.7 || x[i][j]-(int)x[i][j]<0.3)
				{
					x[i][j]=int(x[i][j]+0.5);
				}
				
				cout<<i<<","<<x[i][j]<<" ";
				//outf<<i<<","<<x[i][j]<<" ";
			}
		}
	}  //��ֵ����
}


float MeanPlus( vector<float>  &x, float plus)
{
	float mean=0;
	int m=x.size();
	int Nozero=0;
	int i,j;
	mean=0;
	for (j=0;j<m;j++)
	{
		mean+=x[j]/m;
	}
	mean+=plus;
	for (i=0;i<m;i++)
	{
		if(x[i]!=0)
		{			
			x[i]-=mean;
			if (x[i]>12)
			{
				x[i]=x[i]-12;
			}
			if (x[i]<-12)
			{
				x[i]=x[i]+12;
			}
		}
	}  //��ֵ����
	return mean;

}




float Mean( vector<float>  &x)
{
	float mean=0;
	int m=x.size();
	int Nozero=0;
	int i,j;
	mean=0;
	for (j=0;j<m;j++)
	{
	    mean+=x[j]/m;
	}
	for (i=0;i<m;i++)
	{
		if(x[i]!=0)
		{			
			x[i]-=mean;
			if (x[i]>12)
			{
				x[i]=x[i]-12;
			}
			if (x[i]<-12)
			{
				x[i]=x[i]+12;
			}
		}
	}  //��ֵ����
	return mean;

}



float Mean( vector<float>::iterator  Ybegin,  vector<float>::iterator  Yend)
{
	float mean=0;
	int m=Yend-Ybegin;
	int Nozero=0;
	int i,j;
	mean=0;
	for (j=0;j<m;j++)
	{
		mean+=*(Ybegin+j)/m;
	}
	for (i=0;i<m;i++)
	{
		if(*(Ybegin+i)!=0)
		{			
			*(Ybegin+i)-=mean;
			if (*(Ybegin+i)>12)
			{
				*(Ybegin+i)=*(Ybegin+i)-12;
			}
			if (*(Ybegin+i)<-12)
			{
				*(Ybegin+i)=*(Ybegin+i)+12;
			}
		}
	}  //��ֵ����
	return mean;

}



void Mean( float  *x ,int length)
{
	double mean=0;
	int Nozero=0;
	int i,j;
	mean=0;
	for (j=0;j<length;j++)
	{
		mean+=x[j]/length;
	}
	for (i=0;i<length;i++)
	{
		if(x[i]!=0)
		{			
			x[i]-=mean;
			if (x[i]>12)
			{
				x[i]=x[i]-12;
			}
			if (x[i]<-12)
			{
				x[i]=x[i]+12;
			}
		}
	}  //��ֵ����

}



void MeanFirst(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=0;
	if (m>0)
	{
		n=x[0].size();
	}
	int Nozero=0;
	int vec=0;
	int i,j;
	//for (i=8;i<m-8;i++)
	//{
	//	for (j=0;j<n;j++)
	//	{
	//		if(x[i][j]!=0)
	//		{			
	//			vec=1;
	//		}
	//	}
	//	if (vec==1)
	//	{
	//		Nozero++;
	//	}
	//	vec=0;
	//}
	//ofstream outf("datail.txt",ofstream::app);
	//double variance;
	//vector<double> XVariance;
	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		mean=0;
		for (j=0;j<m;j++)
		{
			mean+=x[j][i]/m;
		}
		XMean.push_back(mean);
	}
	//if ((double)Nozero/(double)m<0.4)
	//{
	//	ZeroToForward(x);

	//}
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=XMean[j];
				//cout<<i<<","<<x[i][j]<<" ";
				//outf<<i<<","<<x[i][j]<<" ";
				if (x[i][j]>10)
				{
					x[i][j]=x[i][j]-12;
				}
				if (x[i][j]<-10)
				{
					x[i][j]=x[i][j]+12;
				}
			}
		}
	}  //��ֵ����

}




void MeanBefor8AndAfter8(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=x[0].size();
	int Nozero=0;
	int vec=0;
	int i,j;
	//for (i=8;i<m-8;i++)
	//{
	//	for (j=0;j<n;j++)
	//	{
	//		if(x[i][j]!=0)
	//		{			
	//			vec=1;
	//		}
	//	}
	//	if (vec==1)
	//	{
	//		Nozero++;
	//	}
	//	vec=0;
	//}
	//ofstream outf("datail.txt",ofstream::app);
	//double variance;
	//vector<double> XVariance;
	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		for (j=8;j<m-8;j++)
		{
			mean+=x[j][i]/(m-16);
		}
		XMean.push_back(mean);
	}
	//if ((double)Nozero/(double)m<0.4)
	//{
	//	ZeroToForward(x);

	//}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=XMean[j];
				cout<<i<<","<<x[i][j]<<" ";
				//outf<<i<<","<<x[i][j]<<" ";
				if (x[i][j]>10)
				{
					x[i][j]=x[i][j]-12;
				}
				if (x[i][j]<-10)
				{
					x[i][j]=x[i][j]+12;
				}
			}
		}
	}  //��ֵ����

}

//����ֵ
void MeanDimentionLSH(vector<float> &x)
{
	float mean=0;
	int m=x.size();
	for (int i=0;i<m;i++)
	{
		mean+=x[i]/m;
	}
	for (int i=0;i<m;i++)
	{
		x[i]-=mean;
	}
}



void Dimention20LSHToToneInteger(vector<double> &x)
{
	int m=x.size();
	for (int i=0;i<m;i++)
	{
		x[i]=(int)(x[i]+0.5);
	}

}

void MeanOld(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=x[0].size();
	int Nozero=0;
	int vec=0;
	int i,j;
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			if(x[i][j]!=0)
			{			
				vec=1;
			}
		}
		if (vec==1)
		{
			Nozero++;
		}
		vec=0;
	}
	//double variance;
	//vector<double> XVariance;
	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		for (j=0;j<m;j++)
		{
			mean+=x[j][i]/Nozero;
		}
		XMean.push_back(mean);
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=XMean[j];
				cout<<i<<","<<x[i][j]<<" ";
			}
		}
	}  //��ֵ����
}


void MeanVar(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=x[0].size();
	int i,j;
	double variance=0;
	vector<double> XVariance;
	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		for (j=0;j<m;j++)
		{
			mean+=x[j][i]/m;
		}
		XMean.push_back(mean);
		mean=0;
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=XMean[j];
				;
			}
		}
	}  //��ֵ����
	for (i=0;i<n;i++)
	{
		for (j=0;j<m;j++)
		{
			variance+=x[j][i]*x[j][i];
		}
		variance=sqrt(variance)/m;
		XVariance.push_back(variance);
		variance=0;
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]/=XVariance[j];
				;
			}
		}
	}  //�����
}


void Var(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=x[0].size();
	int i,j;
	double variance=0;
	vector<double> XVariance;
	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		for (j=0;j<m;j++)
		{
			variance+=x[j][i]*x[j][i];
		}
		variance=sqrt(variance/m);
		XVariance.push_back(variance);
		variance=0;
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{	
				if (XVariance[j]>8)
				{
				}
				x[i][j]/=2;
				;
			}
		}
	}  //�����
}

//����ֵ������������ǰthd֡�ľ�ֵ
void Mean8Minutes(vector<float>  &x, int thd)
{
	double mean=0;
	int m=x.size();
	int i,j;
	int min = MyMinTwo(m,thd);

	for (j=0;j<min;j++)
	{
		mean+=x[j]/min;
	}
	for (i=0;i<m;i++)
	{
		if(x[i]!=0)
		{			
			x[i]-=mean;
		}
	}
}

void Mean8MinutesInt(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;
	int min=MyMinTwo(m,251);
	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		for (j=0;j<min;j++)
		{
			mean+=x[j][i]/min;
		}
		mean=int(mean+0.5);
		XMean.push_back(mean);
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=XMean[j];
				//cout<<i<<":"<<x[i][j]<<"  ";
			}
		}
		//if (x[i].empty())
		//{
		//	x.erase(x.begin()+i);
		//}
	}  //��ֵ����
}

void Var8Minutes(vector< vector<double> > &x)
{
	double variance=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;
	int min=MyMinTwo(m,251);
	vector<double> XVariance;
	for (i=0;i<n;i++)
	{
		for (j=0;j<min;j++)
		{
			variance+=x[j][i]*x[j][i];
		}
		variance=sqrt(variance/min);
		XVariance.push_back(variance);
		variance=0;
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]/=XVariance[j];
				;
			}
		}
	}  //�����
}

void Mean8MinutesVar(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;
	double variance=0;
	vector<double> XVariance;
	int min=MyMinTwo(m,251);
	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		for (j=0;j<min;j++)
		{
			mean+=x[j][i]/min;
		}
		XMean.push_back(mean);
		mean=0;
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=XMean[j];
			}
		}
		//if (x[i].empty())
		//{
		//	x.erase(x.begin()+i);
		//}
	}  //��ֵ����
	for (i=0;i<n;i++)
	{
		for (j=0;j<min;j++)
		{
			variance+=x[j][i]*x[j][i];
		}
		variance=sqrt(variance)/min;
		XVariance.push_back(variance);
		variance=0;
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]/=XVariance[j];
			}
		}
	}  //�����
}

int DistanceMatrix(vector< vector<double> > &queryX, vector< vector<double> > &dataY,vector< vector<double> > &D)
{
	int i,j,m,n;
	m=queryX.size();
	n=dataY.size();
	for (i=0;i<m;i++)
	{
		for (j=0;j<n && j<m*3;j++)
		{
			//D[i][j]=MyDistance(queryX[i],dataY[j]);
		}
	}

	//vector<vector<double>> D(m,vector<double>(n));
	/*for (j=0;j<n-m/2;j++)
	{
	D[0][j]=MyDistance(x[0],y[j]);
	}*/

	return 0;
}

double DTW(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	D[1][1]+=D[0][0];
	D[2][1]+=D[0][0]+penalty;
	for (j=2;j<n-(m-1)/2;j++)
	{
		D[1][j]+=min(D[0][j-1],D[0][j-2]+penalty);
	}
	for (j=2;j<n-(m-2)/2;j++)
	{
		D[2][j]+=MyMin(D[0][j-1]+penalty,D[1][j-1],D[1][j-2]+penalty);

	}
	for (i=3;i<m;i++)
	{
		for (j=(i+1)/2;j<n-(m-i)/2;j++)
		{
			D[i][j]+=MyMin(D[i-2][j-1]+penalty,D[i-1][j-1],D[i-1][j-2]+penalty);
		}
	}
	for (j=1;j<D[i-1].size();j++)
	{
		DL.push_back(D[i-1][j]);
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	//stable_sort(DL.begin(),DL.end());
	return *disIter;
}
double DTWtotalOrig(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	//D[1][1]+=D[0][0];
	//D[2][1]+=D[0][0]+penalty;
	for (j=1;j<m;j++)
	{
		D[j][0]+=D[j-1][0];
	}
	for (i=1;i<m;i++)
	{
		for (j=1;j<n;j++)
		{
			D[i][j]+=MyMin(D[i][j-1]+penalty,D[i-1][j-1],D[i-1][j]+penalty);
		}
	}
	for (j=1;j<D[i-1].size();j++)
	{
		DL.push_back(D[m-1][j]);
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	//stable_sort(DL.begin(),DL.end());
	return *disIter;
}

double DTWtotalFive(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	//D[1][1]+=D[0][0];
	//D[2][1]+=D[0][0]+penalty;
	for (j=1;j<m;j++)
	{
		D[j][0]+=D[j-1][0];
	}
	D[1][1]=MyMin(D[1][0]+penalty,D[0][0],D[0][1]+penalty);
	for (j=2;j<m;j++)
	{
		D[j][1]+=MyMinTwo(MyMin(D[j][0]+penalty,D[j-1][0],D[j-1][1]+penalty),D[j-2][0]+penalty);
	}
	for (j=2;j<n;j++)
	{
		D[1][j]+=MyMinTwo(MyMin(D[0][j]+penalty,D[0][j-1],D[1][j-1]+penalty),D[0][j-2]+penalty);
	}
	for (i=2;i<m;i++)
	{
		for (j=2;j<n;j++)
		{
			D[i][j]+=MyMin(MyMin(D[i][j-1]+penalty,D[i-1][j-1],D[i-1][j]+penalty),D[i-1][j-2]+penalty,D[i-2][j-1]+penalty);
		}
	}
	for (j=1;j<D[i-1].size();j++)
	{
		DL.push_back(D[m-1][j]);
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	//stable_sort(DL.begin(),DL.end());
	return *disIter;
}

double DTWtotal(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	//D[1][1]+=D[0][0];
	//D[2][1]+=D[0][0]+penalty;
	for (j=2;j<m;j++)
	{
		D[j][0]=inf;
	}
	//for (j=1;j<m;j++)
	//{
	//	D[j][0]+=D[j-1][0];
	//}
	D[1][1]=MyMin(D[1][0]+penalty,D[0][0],D[0][1]+penalty);
	for (j=2;j<m;j++)
	{
		D[j][1]+=MyMinTwo(MyMin(D[j][0]+penalty,D[j-1][0],D[j-1][1]+penalty),D[j-2][0]+penalty);
	}
	for (j=2;j<n;j++)
	{
		D[1][j]+=MyMinTwo(MyMin(D[0][j]+penalty,D[0][j-1],D[1][j-1]+penalty),D[0][j-2]+penalty);
	}
	for (i=2;i<m;i++)
	{
		for (j=2;j<n;j++)
		{
			D[i][j]+=MyMin(D[i-1][j-1],D[i-1][j-2]+penalty,D[i-2][j-1]+penalty);
		}
	}
	for (j=1;j<D[i-1].size();j++)
	{
		DL.push_back(D[m-1][j]);
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	//stable_sort(DL.begin(),DL.end());
	return *disIter;
}
//
double DTWBeginQueryAndSongSevenNo(vector< vector<double> > &queryX, vector< vector<double> > &dataY,vector< vector<double> > &D)
{
	double distanceM1;
	double distanceM2;
	double distanceM3;
	double distanceM4;
	double distanceM5;
	double distanceMin;
	double distanceM6;
	double distanceM7;
	int m=queryX.size();
	int n=queryX[0].size();
	int i,j;
	vector<vector<double>>query(m,vector<double>(n,0));
	DistanceMatrix(queryX,dataY,D);
	distanceM1=DTWBeginNew(D);
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			query[i][j]=queryX[i][j]+2;
		}
	}
	DistanceMatrix(query,dataY,D);
	distanceM2=DTWBeginNew(D);
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			query[i][j]=queryX[i][j]+1;
		}
	}
	DistanceMatrix(query,dataY,D);
	distanceM3=DTWBeginNew(D);
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			query[i][j]=queryX[i][j]-2;
		}
	}
	DistanceMatrix(query,dataY,D);
	distanceM4=DTWBeginNew(D);
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			query[i][j]=queryX[i][j]-1;
		}
	}
	DistanceMatrix(query,dataY,D);
	distanceM5=DTWBeginNew(D);
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			query[i][j]=queryX[i][j]-3;
		}
	}
	DistanceMatrix(query,dataY,D);
	distanceM6=DTWBeginNew(D);
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			query[i][j]=queryX[i][j]+3;
		}
	}
	DistanceMatrix(query,dataY,D);
	distanceM7=DTWBeginNew(D);
	distanceMin=MyMin(MyMin(distanceM1,distanceM2,distanceM3),distanceM4,distanceM5);
	distanceMin=MyMin(distanceMin,distanceM6,distanceM7);
	return distanceMin;
}


double DTWdisRecur(vector< vector<double> > &queryX, vector< vector<double> > &dataY,vector< vector<double> > &D,double disMid,double num,double bound)
{                  
	int m=queryX.size();
	int n=queryX[0].size();
	int i,j;
	vector<vector<double>> queryPlus(m,vector<double>(n,0));
	vector<vector<double>> queryMinus(m,vector<double>(n,0));
	double disMinus=0,disPlus=0,distanceMin=0;
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			queryMinus[i][j]=queryX[i][j]-num;//num������ƫ��ֵ
		}
	}
	DistanceMatrix(queryMinus,dataY,D);
	disMinus=DTWBeginNew(D);
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			queryPlus[i][j]=queryX[i][j]+num;
		}
	}
	DistanceMatrix(queryPlus,dataY,D);
	disPlus=DTWBeginNew(D);
	distanceMin=MyMin(disMid,disMinus,disPlus);
	if (num<=bound)//bound��������ƫ����Ŀ
	{
		return distanceMin;
	}
	else if (distanceMin==disMid)
	{
		return DTWdisRecur(queryX,dataY,D,distanceMin,num/2,bound);
	}
	else if (distanceMin==disMinus)
	{
		return DTWdisRecur(queryMinus,dataY,D,distanceMin,num/2,bound);
	}
	else if (distanceMin==disPlus)
	{
		return DTWdisRecur(queryPlus,dataY,D,distanceMin,num/2,bound);
	}
	else
		return distanceMin;
}
double DTWBeginQueryAndSongSeven(vector< vector<double> > &queryX, vector< vector<double> > &dataY,vector< vector<double> > &D)
{
	double distanceMin;
	int m=queryX.size();
	int n=queryX[0].size();
	int i,j;
	//vector<vector<double>>query(m,vector<double>(n,0));
	DistanceMatrix(queryX,dataY,D);
	distanceMin=DTWBeginNew(D);
	return DTWdisRecur(queryX,dataY,D,distanceMin,1,1);//���������1��1��ʾ��������ƽ��һ��
	//return distanceMin;
}

//
//double DTWBeginRecurseLinear(vector< vector<double> > &queryX, vector< vector<double> > &dataY,vector< vector<double> > &D)
//{
//	double distanceMin;
//	int m=queryX.size();
//	int n=queryX[0].size();
//	int i,j;
//	int l=dataY.size();
//	double stretch=0.75;
//	double distanceM;
//	vector <double> distanceStretch;
//	map <double,double>disMapLinear;
//	for (;stretch<1.3;)
//	{
//		vector<vector<double>> queryStretch;
//		StringToString(queryX,queryStretch,stretch);
//		distanceM=StringMatch(queryStretch,dataY);
//		stretch+=0.05;
//		disMapLinear.insert(make_pair(distanceM,stretch));
//		distanceStretch.push_back(distanceM);
//	}
//	stable_sort(distanceStretch.begin(),distanceStretch.end());
//	distanceM=distanceStretch[0];
//	if (disMapLinear.count(distanceM))
//	{
//		stretch=disMapLinear[distanceM];
//	}
//	vector< vector<double> > queryNew;
//	StringToString(queryX,queryNew,stretch);
//	//vector<vector<double>>query(m,vector<double>(n,0));
//	int x=queryNew.size();
//	int y=dataY.size();
//	vector<vector<double>> matrixD(x,vector<double>(y));
//	DistanceMatrix(queryNew,dataY,matrixD);
//	distanceMin=DTWBeginNew(matrixD);
//	return DTWdisRecur(queryNew,dataY,matrixD,distanceMin,1,1);
//	//return distanceMin;
//}

double DTWBeginNew(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	//D[1][1]+=D[0][0];
	//D[2][1]+=D[0][0]+penalty;
	for (j=1;j<m;j++)
	{
		D[j][0]=inf;
	}
	for (j=1;j<n;j++)
	{
		D[0][j]=0;
	}
	//for (j=1;j<m;j++)
	//{
	//	D[j][0]+=D[j-1][0];
	//}
	D[1][1]=MyMin(D[1][0]+penalty,D[0][0],D[0][1]+penalty)+MyMinTwo(D[1][1],disConst);
	for (j=2;j<m;j++)
	{
		D[j][1]=MyMinTwo(MyMin(D[j][0]+penalty,D[j-1][0],D[j-1][1]+penalty),D[j-2][0]+penalty)+MyMinTwo(D[j][1],disConst);
	}
	for (j=2;j<n;j++)
	{
		D[1][j]=MyMinTwo(MyMin(D[0][j]+penalty,D[0][j-1],D[1][j-1]+penalty),D[0][j-2]+penalty)+MyMinTwo(D[1][j],disConst);
	}
	for (i=2;i<m;i++)
	{
		for (j=2;j<n && j<m*1.7;j++)
		{
			D[i][j]=MyMin(D[i-1][j-1]+penalty,D[i-1][j-2],D[i-2][j-1]+penalty)-Dpenalty+MyMinTwo(D[i][j],disConst);
		}
	}
	for (j=1;j<D[i-1].size() && j<m*1.7;j++)
	{
		DL.push_back(D[m-1][j]);
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	//stable_sort(DL.begin(),DL.end());
	return *disIter;
}







double DTWBeginNewLSH(vector<vector<double>> &D)//�õ��ǹ�һ���ľ���
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	//D[1][1]+=D[0][0];
	//D[2][1]+=D[0][0]+penalty;
	for (j=1;j<m;j++)
	{
		D[j][0]=inf;
	}
	for (j=1;j<n;j++)
	{
		D[0][j]=inf;
	}
	//for (j=1;j<m;j++)
	//{
	//	D[j][0]+=D[j-1][0];
	//}
	D[1][1]=MyMin(D[1][0]+penalty,D[0][0],D[0][1]+penalty)+MyMinTwo(D[1][1],disConst);
	for (j=2;j<m;j++)
	{
		D[j][1]=MyMinTwo(MyMin(D[j][0]+penalty,D[j-1][0],D[j-1][1]+penalty),D[j-2][0]+penalty)+MyMinTwo(D[j][1],disConst);
	}
	for (j=2;j<n;j++)
	{
		D[1][j]=MyMinTwo(MyMin(D[0][j]+penalty,D[0][j-1],D[1][j-1]+penalty),D[0][j-2]+penalty)+MyMinTwo(D[1][j],disConst);
	}
	for (i=2;i<m;i++)
	{
		for (j=2;j<n && j<m*1.7;j++)
		{
			D[i][j]=MyMin(D[i-1][j-1],D[i-1][j-2]+penalty,D[i-2][j-1]+penalty)-Dpenalty+MyMinTwo(D[i][j],disConst);
		}
	}
	for (j=1;j<D[i-1].size() && j<m*1.7;j++)
	{
		DL.push_back(D[m-1][j]/m);//�õ��ǹ�һ���ľ���,���query���Ȳ�һ��
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	//stable_sort(DL.begin(),DL.end());
	//return *disIter;
	if	(n>m+1)
	{
		return D[m-1][n-1]/m;
    }
	else
		return *disIter;
	return D[m-1][n-1]/m;//������������Ǹ���������֪�������
}


double DTWBeginAndRA(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	//D[1][1]+=D[0][0];
	//D[2][1]+=D[0][0]+penalty;
	for (j=1;j<m;j++)
	{
		D[j][0]=inf;
	}
	for (j=1;j<n;j++)
	{
		D[0][j]=inf;
	}
	//for (j=1;j<m;j++)
	//{
	//	D[j][0]+=D[j-1][0];
	//}
	D[1][1]=MyMin(D[1][0]+penalty,D[0][0],D[0][1]+penalty)+MyMinTwo(D[1][1],disConst);
	for (j=2;j<m;j++)
	{
		D[j][1]=MyMinTwo(MyMin(D[j][0]+penalty,D[j-1][0],D[j-1][1]+penalty),D[j-2][0]+penalty)+MyMinTwo(D[j][1],disConst);
	}
	for (j=2;j<n;j++)
	{
		D[1][j]=MyMinTwo(MyMin(D[0][j]+penalty,D[0][j-1],D[1][j-1]+penalty),D[0][j-2]+penalty)+MyMinTwo(D[1][j],disConst);
	}
	for (i=2;i<m;i++)
	{
		for (j=2;j<n && j<m*1.7;j++)
		{
			D[i][j]=MyMin(D[i-1][j-1],D[i-1][j-2]+penalty,D[i-2][j-1]+penalty)-Dpenalty+MyMinTwo(D[i][j],disConst);
		}
	}
	
	//if	(n>m+1)
	//{
	//	return D[m-1][n-1]/m;
	//}
	return D[m-1][n-1];//������������Ǹ�
}







double DTWBeginThreeOld(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	//D[1][1]+=D[0][0];
	//D[2][1]+=D[0][0]+penalty;
	for (j=2;j<m;j++)
	{
		D[j][0]=inf;
	}
	for (j=2;j<n;j++)
	{
		D[0][j]=inf;
	}
	//for (j=1;j<m;j++)
	//{
	//	D[j][0]+=D[j-1][0];
	//}
	D[1][1]=MyMin(D[1][0]+penalty,D[0][0],D[0][1]+penalty);
	for (j=2;j<m;j++)
	{
		D[j][1]+=MyMinTwo(MyMin(D[j][0]+penalty,D[j-1][0],D[j-1][1]+penalty),D[j-2][0]+penalty);
	}
	for (j=2;j<n;j++)
	{
		D[1][j]+=MyMinTwo(MyMin(D[0][j]+penalty,D[0][j-1],D[1][j-1]+penalty),D[0][j-2]+penalty);
	}
	for (i=2;i<m;i++)
	{
		for (j=2;j<n && j<m*1.4;j++)
		{
			D[i][j]+=MyMin(D[i-1][j-1],D[i-1][j-2]+penalty,D[i-2][j-1]+penalty)-Dpenalty;
		}
	}
	for (j=1;j<D[i-1].size() && j<m*1.4;j++)
	{
		DL.push_back(D[m-1][j]);
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	//stable_sort(DL.begin(),DL.end());
	return *disIter;
}


double DTWtotalPlusPe(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	//D[1][1]+=D[0][0];
	//D[2][1]+=D[0][0]+penalty;
	for (j=1;j<m;j++)
	{
		D[j][0]+=D[j-1][0];
	}
	D[1][1]=MyMin(D[1][0]+penalty,D[0][0],D[0][1]+penalty);
	for (j=2;j<m;j++)
	{
		D[j][1]+=MyMinTwo(MyMin(D[j][0]+penalty,D[j-1][0],D[j-1][1]+penalty),D[j-2][0]+penalty);
	}
	for (j=2;j<n;j++)
	{
		D[1][j]+=MyMinTwo(MyMin(D[0][j]+penalty,D[0][j-1],D[1][j-1]+penalty),D[0][j-2]+penalty);
	}
	for (i=2;i<m;i++)
	{
		for (j=2;j<n;j++)
		{
			D[i][j]+=MyMin(D[i-1][j-1],D[i-1][j-2]+penalty,D[i-2][j-1]+penalty)-0.3;
		}
	}
	for (j=1;j<D[i-1].size();j++)
	{
		DL.push_back(D[m-1][j]);
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	//stable_sort(DL.begin(),DL.end());
	return *disIter;
}
double DTWbegin(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	//D[1][1]+=D[0][0];
	//D[2][1]+=D[0][0]+penalty;
	for (j=1;j<m;j++)
	{
		D[j][0]+=D[j-1][0];
	}
	for (i=1;i<m;i++)
	{
		for (j=1;j<n && j<m*2;j++)
		{
			D[i][j]+=MyMin(D[i][j-1]+penalty,D[i-1][j-1],D[i-1][j]+penalty);
		}
	}
	for (j=1;j<D[i-1].size() && j<m*2;j++)
	{
		DL.push_back(D[m-1][j]);
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	//stable_sort(DL.begin(),DL.end());
	return *disIter;
}
int PitchToTone(vector <vector <double>> &queryPitch)
{
	int n=queryPitch.size();
	double pitchnum=0;
	int k=FREQ,sam=win;
	for (int i=0;i!=n;i++)
	{
		if (queryPitch[i][0]!=0)
		{
		
		pitchnum=queryPitch[i][0];
		//pitchnum=pitchnum*k*1000/sam;
		pitchnum=k*1000/pitchnum;
		pitchnum=69+12*log(pitchnum/440)/log(2.0);
		cout<<i<<","<<pitchnum<<" ";
		queryPitch[i][0]=pitchnum;
		}
		//else
		//{
		//	vector<vector<double>>::iterator iter=queryPitch.begin();
		//	queryPitch.erase(iter+i);
		//}
	}
	return 0;

}
int realPitchToToneShengda(vector <float> &queryPitch)
{
	int n=queryPitch.size();
	float pitchnum=0;
	static int numpitchall=0;
	for (int i=0;i!=n;i++)
	{
		if (queryPitch[i]!=0)
		{

			pitchnum=queryPitch[i];
			//pitchnum=pitchnum*k*1000/sam;
			//pitchnum=69+12*log(pitchnum/440)/log(2.0);
            pitchnum=(12.0f*(pitchnum-log(440.0f)/log(2.0f))+69.0f);
			queryPitch[i]=pitchnum;
		}
	}
	return 0;


}



int realPitchToTone(vector <float> &queryPitch)
{
	int n=queryPitch.size();
	float pitchnum=0;
	//int k=FREQ,sam=win;
	static int numpitchall=0;
	//ofstream outf("datail.txt"/*,ofstream::app*/);
	for (int i=0;i!=n;i++)
	{
		if (queryPitch[i]!=0)
		{

			pitchnum=queryPitch[i];
			pitchnum=69+12*log(pitchnum/440)/log(2.0);
			queryPitch[i]=pitchnum;
		}
	}
	//outf<<"over"<<endl<<numpitchall<<endl<<endl<<endl<<endl<<endl<<endl;
	return 0;

}


int realPitchToThreeTone(vector <vector <double>> &queryPitch,vector <vector <double>> &queryPitchTow,vector <vector <double>> &queryPitchThree)
{
	int n=queryPitch.size();
	double pitchnum=0;
	//int k=FREQ,sam=win;
	static int numpitchall=0;
	//ofstream outf("datail.txt"/*,ofstream::app*/);
	for (int i=0;i!=n;i++)
	{
		queryPitchTow.push_back(queryPitch[i]);
		queryPitchThree.push_back(queryPitch[i]);
		if (queryPitch[i][0]!=0)
		{

			pitchnum=queryPitch[i][0];
			//pitchnum=pitchnum*k*1000/sam;
			pitchnum=69+12*log(pitchnum/440)/log(2.0);
			//cout<<i<<","<<pitchnum<<" ";
			//outf<<pitchnum<<endl;
			int pitch=queryPitch[i][0];
			if ((pitch >=82.4) && (pitch <=1046.5))
			{
				;
			}
			queryPitchTow[i][0]=69+12*log(queryPitch[i][0]*2/440)/log(2.0);
			queryPitchThree[i][0]=69+12*log(queryPitch[i][0]/2/440)/log(2.0);
			queryPitch[i][0]=pitchnum;
		}
	}
	//outf<<"over"<<endl<<numpitchall<<endl<<endl<<endl<<endl<<endl<<endl;
	return 0;

}




int realPitchToAnotherTowTone(vector <vector <double>> &queryPitch,vector <vector <double>> &queryPitchTow,vector <vector <double>> &queryPitchThree)
{
	int n=queryPitch.size();
	double pitchnum=0;
	//int k=FREQ,sam=win;
	static int numpitchall=0;
	//ofstream outf("datail.txt"/*,ofstream::app*/);
	for (int i=0;i!=n;i++)
	{
		queryPitchTow.push_back(queryPitch[i]);
		queryPitchThree.push_back(queryPitch[i]);
		if (queryPitch[i][0]!=0)
		{

			pitchnum=queryPitch[i][0];
			//pitchnum=pitchnum*k*1000/sam;
			//pitchnum=69+12*log(pitchnum/440)/log(2.0);
			pitchnum=440*pow(2.0,(pitchnum-69)/12);//��ԭΪԭʼ��Ƶ
			//cout<<i<<","<<pitchnum<<" ";
			//outf<<pitchnum<<endl;
			int pitch=queryPitch[i][0];
			if ((pitch >=82.4) && (pitch <=1046.5))
			{
				;
			}
			queryPitchTow[i][0]=69+12*log(pitchnum*1.2/440)/log(2.0);//�ӱ�
			queryPitchThree[i][0]=69+12*log(pitchnum/1.2/440)/log(2.0);//����
		}
	}
	//outf<<"over"<<endl<<numpitchall<<endl<<endl<<endl<<endl<<endl<<endl;
	return 0;

}






int ToneTorealPitch(vector <vector <double>> &queryPitch)
{
	int n=queryPitch.size();
	double pitchnum=0;
	//int k=FREQ,sam=win;
	static int numpitchall=0;
	//ofstream outf("c://humming//datail.txt"/*,ofstream::app*/);
	for (int i=0;i!=n;i++)
	{
		if (queryPitch[i][0]!=0)
		{

			pitchnum=queryPitch[i][0];
			//pitchnum=pitchnum*k*1000/sam;
			//pitchnum=69+12*log(pitchnum/440)/log(2.0);
			pitchnum=pow(2.0,(pitchnum-69)/12)*440;
			//cout<<i<<","<<pitchnum<<" ";
			//outf<<pitchnum<<endl;
			queryPitch[i][0]=pitchnum;
		}
		//else
		//{
		//	vector<vector<double>>::iterator iter=queryPitch.begin();
		//	queryPitch.erase(iter+i);
		//}
	}
	//outf<<"over"<<endl<<numpitchall<<endl<<endl<<endl<<endl<<endl<<endl;
	return 0;

}

void readinstance(char *wavename,vector <vector <double>> &queryPitch)
{
	ifstream indexFile(wavename);
	//map<string , vector<vector<double>>> indexSongName;
	vector <double> pitchNum;
	string songPitchNum;
	while (indexFile>>songPitchNum)
	{
		pitchNum.clear();
		pitchNum.push_back(atof(songPitchNum.c_str()));
		queryPitch.push_back(pitchNum);

	}
	indexFile.close();


}
void discre(vector <vector <double>> &x)
{
	double mean=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;

	for (i=1;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			if(x[i][j]!=0)
			{			
				x[i-1][j]=x[i][j]-x[i-1][j];
			}
		}

	}
	for (i=0;i<n;i++)
	{
		x[m-1][i]=0;
	}

}


void Zerodiscre(vector <vector <double>> &x)
{
	double mean=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;

	for (i=1;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			if(x[i][j]!=0&&x[i-1][j]!=0)
			{			
				x[i-1][j]=x[i][j]-x[i-1][j];
			}
			else
			{
				x[i-1][j]=0;
			}

		}

	}
	for (i=0;i<n;i++)
	{
		x[m-1][i]=0;
	}

}
void StringToString( vector<float>  &queryX,  vector<float>  &dataY,float stretch)
{
	int i,j,m,n,k;
	m=queryX.size();
	float ratio=0;
	for (i=0;i<(m-1)*stretch;i++)
	{
		float pitch_query=0;
		k=i/stretch;
		ratio=i/stretch-k;
		if (k<m-1)
		{

			pitch_query=(queryX[k]*(1-ratio)+queryX[k+1]*ratio);
			dataY.push_back(pitch_query);
		}
	}

}




void StringToStringSame( vector<float>  &queryX,  vector<float>  &dataY,float stretch)
{
	int i,j,m,n,k;
	m=queryX.size();
	float ratio=0;
	for (i=0;i<(m-1)*stretch;i++)
	{
		float pitch_query=0;
		k=i/stretch;
		ratio=i/stretch-k;
		if (k<m-1)
		{
			dataY.push_back(queryX[k]);
		}
	}

}




float LinearToDis( vector<float>  &queryX,  vector<float>  &dataY)
{
	int i,j,m,n=0,k;
	m=queryX.size();
	if	(m>0 && dataY.size()>0)
	{
	   float stretch=((double)dataY.size())/queryX.size();
	   float ratio=0;
	   float Dis=0;
	   vector<float>  queryStretchX;
	   int totalm=(m-1)*stretch;
	   for (i=0;i<totalm;i++)
	   {
		   double pitch_query=0;
		   k=i/stretch;
		   ratio=i/stretch-k;
		   if (k<m-1)
		   {

			   pitch_query=(queryX[k]*(1-ratio)+queryX[k+1]*ratio);

			   queryStretchX.push_back(pitch_query);
		   }
	   }
	   Dis=StringMatch(queryStretchX,dataY);
	   //Dis=PitchDistance(queryStretchX,dataY);
	  // Dis=StringMatch(queryStretchX,dataY);
	   return Dis;
	}
	else
		return 0;
	
	//dataY.push_back(queryX[m-1]);

}






float CalculateOptimalEdge( vector<float>  &queryX,  vector<float>  &dataY,int &left,int &right,int length,float ratio)
{
	int n=queryX.size();
	int m=dataY.size();
	int bestLeft=0;
	int bestRight=0;
	float bestDis=10000;
	float Cdis=0;
	int step=4;
	/*if (ratio>0.6)
	{
		step=3;
	}*/
	//else if (ratio>0.7)
	//{
	//	step=5;
	//}
	vector<float>::iterator  Ybegin=dataY.begin();
	vector<float>::iterator  Yend=dataY.end();
	for (int i=0;i< length;i+=step)
	{
		for (int j=0;j<length;j+=step)
		{
			Mean(Ybegin+i,Yend-j);

			Cdis=LinearToDisIter(queryX.begin(),queryX.end(),Ybegin+i,Yend-j);
			if (bestDis>Cdis)
			{
				bestDis=Cdis;
				bestLeft=i;
				bestRight=j;
			}
		}
	}
	left=bestLeft;
	right=bestRight;
	if (bestLeft!=0)
	{
		dataY.erase(dataY.begin(),dataY.begin()+bestLeft);
	}
	if (bestRight!=0)
	{
		dataY.erase(dataY.end()-bestRight,dataY.end());
	}
	Mean(dataY.begin(),dataY.end());
	return bestDis;
	

}





float LinearToDisIter( vector<float>::iterator  Xbegin,  vector<float>::iterator  Xend,
					   vector<float>::iterator  Ybegin,  vector<float>::iterator  Yend)
{
	int i,j,m,n=0,k;
	m=Xend-Xbegin;
	n=Yend-Ybegin;
	float dis=0;
	if	(m>0 && n>0)
	{
		float stretch=((double)n)/m;
		float ratio=0;
		float Dis=0;
		int numY=0;
		//vector<float>  queryStretchX;
		int totalm=(m-1)*stretch;
		for (i=0;i<totalm;i++)
		{
			float pitch_query=0;
			k=i/stretch;
			ratio=i/stretch-k;
			if (k<m-1)
			{

				numY++;
				pitch_query=(*(Xbegin+k)*(1-ratio)+*(Xbegin+k+1)*ratio);
				Dis+=MyMinTwo(abs(pitch_query-*(Ybegin+i)),disConstString);

			}
		}

		if(numY!=0)
		{
			Dis/=numY;
		}
		return Dis;
	}
	else
		return 0;

	//dataY.push_back(queryX[m-1]);

}




double LinearToDisUltimate(vector< vector<double> > &queryX, vector< vector<double> > &dataY,int &ultimateNum)
{
	int i,j,m,n=0,k;
	m=queryX.size();
	if	(m>0 && dataY.size()>0)
	{
		n=queryX[0].size();
		double stretch=((double)dataY.size())/queryX.size();
		double ratio=0;
		double Dis=0;
		vector< vector<double> > queryStretchX;
		int totalm=(m-1)*stretch;
		for (i=0;i<totalm;i++)
		{
			vector<double> pitch_query;
			k=i/stretch;
			ratio=i/stretch-k;
			if (k<m-1)
			{
				for (j=0;j<n;j++)
				{
					pitch_query.push_back(queryX[k][j]*(1-ratio)+queryX[k+1][j]*ratio);
				}
				queryStretchX.push_back(pitch_query);
			}
		}
		Dis=StringMatchUltimate(queryStretchX,dataY,ultimateNum);
		//Dis=PitchDistance(queryStretchX,dataY);
		// Dis=StringMatch(queryStretchX,dataY);
		return Dis;
	}
	else
		return 0;

	//dataY.push_back(queryX[m-1]);

}



void StringToStringNoMean(vector< vector<double> > &queryX, vector< vector<double> > &dataY,double stretch)
{
	int i,m,n,k;
	m=queryX.size();
	n=queryX[0].size();
	for (i=0;i<m*stretch;i++)
	{
		k=i/stretch;
		if (k<m)
		{
			dataY.push_back(queryX[k]);
		}
	}

}

float StringMatch( vector<float>  &queryX,  vector<float>  &dataY)
{
	int i,j,m,n;
	m=queryX.size();
	n=dataY.size();
	float dis=0;
	int num=MyMinTwo(m,n);
	for (i=0;i< num;i++)
	{
		dis+=MyMinTwo(abs(queryX[i]-dataY[i]),disConstString);
	}
	//dis=dis*dis;
	if(num!=0)
	{
		dis/=num;
	}
	return dis;

}


double StringMatchUltimate(vector< vector<double> > &queryX, vector< vector<double> > &dataY,int &ultimateNum)
{
	int i,j,m,n;
	m=queryX.size();
	n=dataY.size();
	double dis=0;
	int num=MyMinTwo(m,n);
	for (i=0;i< num;i++)
	{
		//dis+=MyMinTwo(MyDistance(queryX[i],dataY[i]),disConstString);
	}
	//dis=dis*dis;
	ultimateNum+=num;
	//if(num!=0)
	//{
	//	dis/=num;
	//}
	return dis;

}



double StringMatchToDis(vector< vector<double> > &queryX, vector< vector<double> > &dataY)
{
	int n=dataY.size();
	double stretch=0.75;
	double distanceM;
	vector <double> distanceStretch;
	for (;stretch<1.3;)
	{
		vector<vector<double>> queryStretch;
		//StringToString(queryX,queryStretch,stretch);
		//distanceM=StringMatch(queryStretch,dataY);
		stretch+=0.05;
		distanceStretch.push_back(distanceM);
	}
	stable_sort(distanceStretch.begin(),distanceStretch.end());
	distanceM=distanceStretch[0];
	return distanceM;

}





double StringMatchToDisMapRALSHNewPairVariancePositionVariance( vector<float>  &queryX,  vector<float>  &dataY,
															   int recurse,int pairNum,double MidPercentage,int &ultimateNum)
{
	int i;
	int sizeX=queryX.size();
	int sizeY=dataY.size();
	int MidPos=sizeX/2;
	//int BeginMatchPos;
	//if (recurse>1)
	//{
	//	BeginMatchPos=sizeX/3;
	//}
	//else
	//	BeginMatchPos=sizeX/3;
	int BeginMatchPos=sizeX*MidPercentage*1/3;
	//int BeginMatchPos=sizeX*MidPercentage/4;
	//int pairNum=15;//��ʾλ�ö�Ӧ���ԵĴ���
	double MovePoints=(sizeX-BeginMatchPos*2)/(double)pairNum;
	//printf("sizeX%d ",sizeX);
	//printf("MovePoints%f ",MovePoints);
	if (MovePoints<1)
	{
		MovePoints=1;
	}
	int MidDataYSize=dataY.size()/2;
	int MinDisPos=0;//��С�����λ��
	double distanceM;
	map <double,int>disMap;
	vector <double> distanceStretch;
	if (sizeX<=8 || sizeY<=8)
	{
		distanceM=LinearToDis(queryX,dataY);
		ultimateNum++;
		return distanceM;
	}
	vector<float> DataY_L(dataY.begin(),dataY.begin()+MidDataYSize);
	//Mean(DataY_L);
	vector<float>  DataY_R(dataY.begin()+MidDataYSize,dataY.end());
	//Mean(DataY_R);

	for (i=0;i<=pairNum;i++)
	{
		vector<float>  queryX_L(queryX.begin(),queryX.begin()+BeginMatchPos+i*MovePoints);
		vector<float>  queryX_R(queryX.begin()+BeginMatchPos+i*MovePoints,queryX.end());
		//Mean(queryX_L);
		//Mean(queryX_R);
		distanceM=LinearToDis(queryX_L,DataY_L)+LinearToDis(queryX_R,DataY_R);
		disMap.insert(make_pair(distanceM,i));
		distanceStretch.push_back(distanceM);
		//if (0==MovePoints)
		//{
		//	i+=pairNum;
		//}
		if (queryX.begin()+BeginMatchPos+i*MovePoints==queryX.end()-2)
		{
			i+=pairNum;
		}
	}
	stable_sort(distanceStretch.begin(),distanceStretch.end());
	distanceM=distanceStretch[0];
	if (recurse==0)
	{
		//if (disMap.count(distanceM))
		//{
		//	MinDisPos=disMap[distanceM];
		//	vector< vector<double> > queryX_L(queryX.begin(),queryX.begin()+BeginMatchPos+MinDisPos*MovePoints);
		//	vector< vector<double> > queryX_R(queryX.begin()+BeginMatchPos+MinDisPos*MovePoints,queryX.end());
		//	distanceM=LinearToDisUltimate(queryX_L,DataY_L,ultimateNum)+
		//		LinearToDisUltimate(queryX_R,DataY_R,ultimateNum);
		//	//if	(recurse=3)
		//	//{
		//	//	distanceM=distanceM/sizeX;
		//	//}
		//	return distanceM;
		//}
		ultimateNum+=2;

		return distanceM;

	}
	else
	{

		if (disMap.count(distanceM))
		{
			MinDisPos=disMap[distanceM];
			recurse--;
			MidPercentage=MidPercentage*1.1;
			//MidPercentage=MidPercentage*1;
			pairNum=pairNum-2;
			if (pairNum<3)
			{
				pairNum=3;
			}
			 vector<float>  queryX_L(queryX.begin(),queryX.begin()+BeginMatchPos+MinDisPos*MovePoints);
			vector<float>  queryX_R(queryX.begin()+BeginMatchPos+MinDisPos*MovePoints,queryX.end());
			distanceM=StringMatchToDisMapRALSHNewPairVariancePositionVariance(queryX_L,DataY_L,recurse,pairNum,MidPercentage,ultimateNum)+
				StringMatchToDisMapRALSHNewPairVariancePositionVariance(queryX_R,DataY_R,recurse,pairNum,MidPercentage,ultimateNum);
			//if	(recurse=3)
			//{
			//	distanceM=distanceM/sizeX;
			//}
			return distanceM;
		}

	}
	return distanceM;




}







float RAPositionVarianceOptimal( vector<float>  &queryX,  vector<float>  &dataY,
															   int recurse,int pairNum,float MidPercentage,int &ultimateNum)
{
	int i;
	int sizeX=queryX.size();
	int sizeY=dataY.size();
	int MidPos=sizeX/2;
	//int BeginMatchPos;
	//if (recurse>1)
	//{
	//	BeginMatchPos=sizeX/3;
	//}
	//else
	//	BeginMatchPos=sizeX/3;
	int BeginMatchPos=sizeX*MidPercentage*1/3;
	//int BeginMatchPos=sizeX*MidPercentage/4;
	//int pairNum=15;//��ʾλ�ö�Ӧ���ԵĴ���
	float MovePoints=(sizeX-BeginMatchPos*2)/(double)pairNum;
	//printf("sizeX%d ",sizeX);
	//printf("MovePoints%f ",MovePoints);
	if (MovePoints<1)
	{
		MovePoints=1;
	}
	int MidDataYSize=dataY.size()/2;
	int MinDisPos=0;//��С�����λ��
	double distanceM;
	map <float,int>disMap;
	vector <float> distanceStretch;
	if (sizeX<=8 || sizeY<=8)
	{
		distanceM=LinearToDis(queryX,dataY);
		ultimateNum++;
		return distanceM;
	}
	vector<float> DataY_L(dataY.begin(),dataY.begin()+MidDataYSize);
	//Mean(DataY_L);
	vector<float>  DataY_R(dataY.begin()+MidDataYSize,dataY.end());
	//Mean(DataY_R);

	for (i=0;i<=pairNum;i++)
	{
		distanceM=LinearToDisIter(dataY.begin(),dataY.begin()+MidDataYSize,queryX.begin(),queryX.begin()+BeginMatchPos+i*MovePoints)+
			LinearToDisIter(dataY.begin()+MidDataYSize,dataY.end(),queryX.begin()+BeginMatchPos+i*MovePoints,queryX.end());
		disMap.insert(make_pair(distanceM,i));
		distanceStretch.push_back(distanceM);
		//if (0==MovePoints)
		//{
		//	i+=pairNum;
		//}
		if (queryX.begin()+BeginMatchPos+i*MovePoints==queryX.end()-2)
		{
			i+=pairNum;
		}
	}
	stable_sort(distanceStretch.begin(),distanceStretch.end());
	distanceM=distanceStretch[0];
	if (recurse==0)
	{
		//if (disMap.count(distanceM))
		//{
		//	MinDisPos=disMap[distanceM];
		//	vector< vector<double> > queryX_L(queryX.begin(),queryX.begin()+BeginMatchPos+MinDisPos*MovePoints);
		//	vector< vector<double> > queryX_R(queryX.begin()+BeginMatchPos+MinDisPos*MovePoints,queryX.end());
		//	distanceM=LinearToDisUltimate(queryX_L,DataY_L,ultimateNum)+
		//		LinearToDisUltimate(queryX_R,DataY_R,ultimateNum);
		//	//if	(recurse=3)
		//	//{
		//	//	distanceM=distanceM/sizeX;
		//	//}
		//	return distanceM;
		//}
		ultimateNum+=2;

		return distanceM;

	}
	else
	{

		if (disMap.count(distanceM))
		{
			MinDisPos=disMap[distanceM];
			recurse--;
			MidPercentage=MidPercentage*1.1;
			//MidPercentage=MidPercentage*1;
			pairNum=pairNum-2;
			if (pairNum<3)
			{
				pairNum=3;
			}
			vector<float>  queryX_L(queryX.begin(),queryX.begin()+BeginMatchPos+MinDisPos*MovePoints);
			vector<float>  queryX_R(queryX.begin()+BeginMatchPos+MinDisPos*MovePoints,queryX.end());
			distanceM=RAPositionVarianceOptimal(queryX_L,DataY_L,recurse,pairNum,MidPercentage,ultimateNum)+
				RAPositionVarianceOptimal(queryX_R,DataY_R,recurse,pairNum,MidPercentage,ultimateNum);
			//if	(recurse=3)
			//{
			//	distanceM=distanceM/sizeX;
			//}
			return distanceM;
		}

	}
	return distanceM;




}


void StringTosignature(vector<float>  &dataY,  signature_t  &Y)
{
	int n=dataY.size();
	int m=emdLength;
	n=MyMinTwoInt(n,m);
	int num=1;
	for (int i=0;i<n-1 ;i++)
	{
		if (dataY[i]!=dataY[i+1])
		{
			num++;
		}
	}
	if (num<(MAX_SIG_SIZE-1))
	{
		Y.n=num;

		
	}
	else
		Y.n=MAX_SIG_SIZE-1;

	int currentNote=0;
	if (n>0)
	{
		Y.Features[0]=dataY[0];
		Y.Weights[0]=1;
	}
	for (int i=0;i<n-1 && currentNote<MAX_SIG_SIZE-1;i++)
	{
		if (dataY[i]==dataY[i+1])
		{
			Y.Weights[currentNote]+=1;
		}
		else
		{
			++currentNote;
			Y.Features[currentNote]=dataY[i+1];
			Y.Weights[currentNote]=1;
		}
	}
}

bool CompareLastName(const pair<int,string> &p1,const pair<int,string> &p2)
{
	return p1.first>p2.first;
}

vector<pair<int,int>> DuplicateSegmentBegin(vector<float> &tone)	//����������tone����λ��ƥ�俪ͷ����ƥ�䳤�ȴ���100֡����¼��λ�úͳ���֡��������
{
	vector<pair<int,int>> dupl;
	pair<int,int> segment;
	int sizeA=tone.size();
	double temp=0;
	int dupNum=0;
	for (int i=1;i<sizeA;i++)
	{
		temp=tone[i];
		dupNum=0;
		if (temp==tone[0])
		{
			for (int j=i;j<sizeA;j++)
			{
				temp=tone[j];
				if (temp==tone[j-i])
				{
					dupNum++;
				}
				else
					j+=sizeA;
			}
		}
		if (dupNum>100)
		{
			segment.first=i;
			segment.second=i+dupNum;
			dupl.push_back(segment);
		}
	}
	return dupl;
}




void OneFileToMultiFile(string fileName,int ThreadNum)
{
	ifstream wavList(fileName.c_str());
	string oneLine;
	char buffer[20];
	int totalWav=0;
	int fileCnum=0;
	for (int i=0;i<ThreadNum;i++)
	{
		string cmd("del ");
		cmd+=fileName;
		cmd+=itoa(i,buffer,10);
		system(cmd.c_str());
	}
	while (wavList>>oneLine)
	{
		totalWav++;
		fileCnum=totalWav%ThreadNum;
		string fileN=fileName+itoa(fileCnum,buffer,10);
		ofstream fileSplit(fileN.c_str(),ofstream::app);
		fileSplit<<oneLine<<endl;
		fileSplit.close();
	}
	wavList.close();

}





void MultiFileDel(string fileName,int ThreadNum)
{
	ifstream wavList(fileName.c_str());
	string oneLine;
	char buffer[20];
	for (int i=0;i<ThreadNum;i++)
	{
		string cmd("del ");
		cmd+=fileName;
		cmd+=itoa(i,buffer,10);
		system(cmd.c_str());
	}

}

void MeanNoteLSH(vector<float> &x, float mean)
{
	int m=x.size();
	for (int i=0;i<m;i++)
	{
		x[i]-=mean;
	} 
}

float MeanLSH(vector<float> &x)
{
	float mean=0;
	int m=x.size();
	for (int i=0;i<m;i++)
	{
		mean+=x[i]/m;
	}
	return mean;

}

int WavToSongFive (char *wavename,map<string , vector<float>> &indexSongName,PRNearNeighborStructT &IndexHuming,PRNearNeighborStructT &IndexHumingNote,
								map<unsigned long , pair<string,short>> &IndexLSH,map<unsigned long , pair<string,pair<short,short>>> &IndexLSHNote,
								int stepFactor,IntT RetainNum,IntT RetainNumNote,IntT LSHFilterNum,IntT LSHFilterNumNote,float stepRatioP,vector<string>& songFive)
{

	float stepRatio=stepRatioP;
	clock_t firstTime,lastTime,firstTimeTemp,lastTimeTemp,firstTimeTempLSH1,lastTimeTempLSH1,firstTimeTempLSH2,lastTimeTempLSH2;
	double OneSongLSHTime=0;
	static double totalLSHTime=0;
	double OneSongLSTime=0;
	static double totalLSTime=0;
	double OneSonglshnoteTime=0;
	static double totalLSHNoteTime=0;
	double OneSonglshPitchTime=0;
	static double totalLSHOnlyNoteTime=0;
	static double totalLSHOnlyPitchTime=0;
	static double totalLSHRetrievalPostProcessPitchTime=0;
	static double totalLSHRetrievalPostProcessPitchTimeNote=0;
	static double totalLSHPitchTime=0;
	static double totalLSHPitchFirstTime=0;
	static double totalLSHNoteFirstTime=0;
	static double totalLSHoneTime=0;
	static double totalLSHtwoTime=0;
	static double totalLSHthreeTime=0;
	double OneSongEMDTime=0;
	static double totalEMDTime=0;
	double OneSongRATime=0;
	static double totalRATime=0;
	static int total=0;
	vector <float> queryPitch;
	vector <float> queryPitchNote;
	map<float ,string> songDis;
	songFive.clear();
	vector <float> Dis;
	bool returnN=false;
	float *pFeaBuf = NULL;
	int nFeaLen = 0;
	SNote *QueryNotes= NULL;
	int nNoteLen=0;
	float pitchNum=0;
	static int shengdaPitch=0;
	static int enhance=0;
	signature_t query;

	feature_t * NoteEmd=NULL;
	float * NoteDuration=NULL;


	int reNum=SMelodyFeatureExtraction(wavename,pFeaBuf,nFeaLen,QueryNotes,nNoteLen,0.5);

	if (reNum!=ERROR_CODE_TOO_SHORT_INPUT)
	{
		shengdaPitch++;
		for (int i=0;i<nFeaLen;i++)
		{
			pitchNum=*(pFeaBuf+i);
			queryPitch.push_back(pitchNum);//��ԭʼ��Ƶ
		}
		int emdl=MyMinTwoInt(nFeaLen,emdLength);
		int lengC=0;
		int lengNote=0;
		realPitchToToneShengda(queryPitch);
		float meanNote=Mean(queryPitch);
		NoteEmd=(feature_t *)malloc(nNoteLen*sizeof(feature_t));
		NoteDuration=(float *)malloc(nNoteLen*sizeof(float));
		for (int i=0;i<nNoteLen;i++)
		{
			lengNote++;
			if (QueryNotes[i].fNoteValue!=0)
			{
				*(NoteEmd+i)=QueryNotes[i].fNoteValue-meanNote;
				for (int j=0;j<QueryNotes[i].fNoteDuration;j++)
				{
					queryPitchNote.push_back(QueryNotes[i].fNoteValue-meanNote);
				}
			}
			else
			{
				*(NoteEmd+i)=QueryNotes[i].fNoteValue;
				for (int j=0;j<QueryNotes[i].fNoteDuration;j++)
				{
					queryPitchNote.push_back(QueryNotes[i].fNoteValue);
				}
			}
			if (lengC+QueryNotes[i].fNoteDuration<=emdLength)
			{
				*(NoteDuration+i)=QueryNotes[i].fNoteDuration;
			}
			else
				*(NoteDuration+i)=QueryNotes[i].fNoteDuration-(emdLength-lengC);
			lengC+=QueryNotes[i].fNoteDuration;
			if (lengC>=emdl)
			{
				i+=nNoteLen;
			}
			

		}
		query.n=lengNote;
		query.Features=NoteEmd;
		query.Weights=NoteDuration;

	}
	ofstream shengdaTimes("wav.result",ofstream::app);//LSHʱ��
	shengdaTimes<<"ʢ����ȡ������"<<shengdaPitch<< endl;
	//if (shengdaPitch%300==0 || shengdaPitch>352)
	//{
	//	cout<<"ʢ����ȡ������"<<shengdaPitch<< endl;
	//}
	
	shengdaTimes.close();

	if(NULL!=pFeaBuf){
		delete[] pFeaBuf;
		pFeaBuf=NULL;
	}
	if(NULL!=QueryNotes){
		delete[] QueryNotes;
		QueryNotes=NULL;
	}

	if (reNum==10)
	{
		
		
		string enhanceS("SpeechEnhance.exe ");
		string pitchname(wavename);
		string::size_type enditer=0;
		//beginiter=nameSong.rfind("\");
		enditer=pitchname.rfind(".wav");
		string nameSong;
		//nameSong.assign(pitchname,enditer-5,9);
		nameSong.assign(pitchname,enditer-5,9);
		char buffer[30];
		
		//WaitForSingleObject(hMutex,INFINITE);
		enhance++;
		string num=(itoa(enhance,buffer,10));
		nameSong=num+nameSong;
		num=(itoa(rand(),buffer,10));
		nameSong=num+nameSong;
		num=(itoa(rand(),buffer,10));
		nameSong=num+nameSong;
		char filename[300];
		strcpy(filename,nameSong.c_str());
		string cmd=enhanceS+pitchname+" "+filename+" "+"2";
		//ReleaseMutex(hMutex);
		system(cmd.c_str());
		reNum=SMelodyFeatureExtraction(filename,pFeaBuf,nFeaLen,QueryNotes,nNoteLen,0.3);
		cmd="del ";
		cmd+=nameSong;
		system(cmd.c_str());
		ofstream shengdaTimes("wav.result",ofstream::app);//LSHʱ��
		shengdaTimes<<"���������"<<enhance<< endl;
		cout<<"���������"<<enhance<< endl;
		shengdaTimes.close();
		if (reNum!=ERROR_CODE_TOO_SHORT_INPUT)
		{
			shengdaPitch++;
			for (int i=0;i<nFeaLen;i++)
			{
				pitchNum=*(pFeaBuf+i);
				queryPitch.push_back(pitchNum);//��ԭʼ��Ƶ
			}
			int emdl=MyMinTwoInt(nFeaLen,emdLength);
			int lengC=0;
			int lengNote=0;
			realPitchToToneShengda(queryPitch);
			float meanNote=Mean(queryPitch);
			NoteEmd=(float *)malloc(nNoteLen*sizeof(feature_t));
			NoteDuration=(float *)malloc(nNoteLen*sizeof(float));
			for (int i=0;i<nNoteLen;i++)
			{
				lengNote++;
				if (QueryNotes[i].fNoteValue!=0)
				{
					*(NoteEmd+i)=QueryNotes[i].fNoteValue-meanNote;
					for (int j=0;j<QueryNotes[i].fNoteDuration;j++)
					{
						queryPitchNote.push_back(QueryNotes[i].fNoteValue-meanNote);
					}
				}
				else
				{
					*(NoteEmd+i)=QueryNotes[i].fNoteValue;
					for (int j=0;j<QueryNotes[i].fNoteDuration;j++)
					{
						queryPitchNote.push_back(QueryNotes[i].fNoteValue);
					}
				}
				if (lengC+QueryNotes[i].fNoteDuration<=emdLength)
				{
					*(NoteDuration+i)=QueryNotes[i].fNoteDuration;
				}
				else
					*(NoteDuration+i)=QueryNotes[i].fNoteDuration-(emdLength-lengC);
				lengC+=QueryNotes[i].fNoteDuration;
				if (lengC>=emdl)
				{
					i+=nNoteLen;
				}


			}
			query.n=lengNote;
			query.Features=NoteEmd;
			query.Weights=NoteDuration;
		}
		if(NULL!=pFeaBuf){
			delete[] pFeaBuf;
			pFeaBuf=NULL;
		}
		if(NULL!=QueryNotes){
			delete[] QueryNotes;
			QueryNotes=NULL;
		}
	}


	if (reNum==10)
	{

		int beginHummingFrame=charToVector(wavename,queryPitch);
		realPitchToTone(queryPitch);
		ZeroToForwardThresholdAndLongZeroToHalfBefor(queryPitch,beginHummingFrame);
		smooth(queryPitch);
		VectorSmoothToHalf(queryPitch);
		smooth(queryPitch);
		Mean(queryPitch);
		query.n=0;
		query.Features=NULL;
		query.Weights=NULL;
	}
	signature_t candiY;
	//candiY.Features=NULL;
	//candiY.Weights=NULL;

	candiY.Features=(feature_t *) malloc((MAX_SIG_SIZE-1)*sizeof(feature_t));
	candiY.Weights=(float *) malloc((MAX_SIG_SIZE-1)*sizeof(float));
	


	
	//QueryPitchToLSHVector(queryPitch,LSHQueryVector);
	float FloorLevelInitial=0.6;//��ʼֵ
	float FloorLevel=FloorLevelInitial;
	float UpperLimit=1.7;
	float StretchStep=0.1; //������ʵһֱ��0.05
	int MatchBeginPos=6;//����query��dataY���������������1/MatchBeginPos��Χ֮�ڲŽ���ƥ��
	//IntT RetainNum=3;//ÿ���������3����
	static int CandidatesDTWAll=0;
	float ratioAll=2.6;
	static int filter0ne=0;
	static int filterTwo=0;
	static int filterThree=0;
	vector <string> tempList;
	vector <float >tempDis1;
	vector <float >tempDis2;
	vector <float >tempDis3;
	for (int recur=0;recur<3;recur++)
	{

		if (recur==0)
		{
			filter0ne++;
		}
		else if (recur==1)
		{
			filterTwo++;
		}
		else if (recur==2)
		{
			filterThree++;
		}
		vector <vector<vector<float>>> LSHQueryVectorLinearStretching;
		vector <vector<vector<float>>> LSHQueryVectorLinearStretchingNote;
		vector<pair<short, short>> posPair;
		if (recur==0)
		{
			FloorLevelInitial=1;//��ʼֵ
			FloorLevel=FloorLevelInitial;
			StretchStep=0.1;
			UpperLimit=1;
			QueryPitchToLSHVectorLinearStretchingShortToMoreNoteFirst(posPair,queryPitchNote,LSHQueryVectorLinearStretchingNote, 
				FloorLevel, UpperLimit,stepFactor,stepRatio, StretchStep);
		}
		else if (recur==1)
		{
			FloorLevelInitial=0.8;//��ʼֵ
			FloorLevel=FloorLevelInitial;
			StretchStep=0.2;
			UpperLimit=1.4;
		}
		else if (recur==2)
		{
			FloorLevelInitial=0.6;//��ʼֵ
			FloorLevel=FloorLevelInitial;
			StretchStep=0.1;
			UpperLimit=1.7;
			QueryPitchToLSHVectorLinearStretchingShortToMoreNote(posPair,queryPitchNote,LSHQueryVectorLinearStretchingNote, 
				FloorLevel, UpperLimit,stepFactor,stepRatio, StretchStep);
		}


		//QueryPitchToLSHVectorLinearStretchingShortToMoreNote(posPair,queryPitchNote,LSHQueryVectorLinearStretchingNote, 
		//	FloorLevel, UpperLimit,stepFactor,stepRatio, StretchStep);
		QueryPitchToLSHVectorLinearStretchingShortToMore(queryPitch,LSHQueryVectorLinearStretching, 
			FloorLevel, UpperLimit,stepFactor,stepRatio, StretchStep,recur); //query��������תΪ20ά
		//ȡ��������LSH��Ҫ�޸������ط�
		
		int LinearCoe=0;
		int LinearCoeTotal=LSHQueryVectorLinearStretching.size();
		vector<vector<IntT>> IndexCandidatesStretch;
		vector<vector<float>> IndexCandidatesDis;
		vector<vector<IntT>> CandidatesNumStretch;
		vector<IntT> CandidatesFilter;
		firstTime=clock();
		map<int,vector<int>> :: iterator samePointIte;


		firstTimeTempLSH1=clock();
		int edge=6;
		float LSratio=0.4;
		int qRecurse=1;
		if (recur==10)
		{
			edge=1;
		}
		else 
			edge=6;
if (/*0*/recur!=1) //��Ϊ�����ļ���
{
		//����Ϊ������LSH����
#if 1


		LinearCoe=0;
		PPointT *QueriesArray=NULL;//Ҫfree
		if (LSHQueryVectorLinearStretchingNote[LinearCoe].size()>0)
		{
			QueriesArray=readDataSetFromVector(LSHQueryVectorLinearStretchingNote[LinearCoe]);
			Int32T nPointsQuery=LSHQueryVectorLinearStretchingNote[LinearCoe].size();
			IntT dimension=6;
			if (nPointsQuery>0)
			{
				dimension=LSHQueryVectorLinearStretchingNote[LinearCoe][0].size();
			}
			IntT LSHFilterReturnNum=0;
			IntT *IndexArray=NULL;
			int IndexArraySize=1000000;//�����Һ�ѡ��Ŀ
			IndexArray=(IntT *)MALLOC(IndexArraySize *sizeof(IntT));
			double *IndexArrayDis=(double *)MALLOC(IndexArraySize *sizeof(double));
			IntT *IndexFilterArray=NULL;
			IndexFilterArray=(IntT *)MALLOC(IndexArraySize/2 *sizeof(IntT));
			IntT * NumArray=(IntT *)MALLOC(nPointsQuery *sizeof(IntT));//ÿ���㷵�ص���Ŀ
			IntT ResultSize=LSHStructToResultOnePointRetainSeveral(QueriesArray,nPointsQuery,IndexArraySize, 
				IndexArray,IndexHumingNote,NumArray,RetainNumNote , dimension,LSHFilterNumNote,IndexFilterArray,LSHFilterReturnNum,IndexArrayDis);//�õ������ÿ���㷵��RetainNum����ѡ
			vector<IntT> IndexCandidates;
			vector <float> DisCandidates;
			vector<IntT> CandidatesNum;//ÿ���㷵�ص���Ŀ����Vector
			int curreIndex=0;
			//for (int i=0;i<ResultSize;i++)
			//{
			//	IndexCandidates.push_back(IndexArray[i]);
			//	DisCandidates.push_back(IndexArrayDis[i]);
			//}
			for (int i=0;i<LSHFilterReturnNum;i++)
			{
				CandidatesFilter.push_back(IndexFilterArray[i]);
			}
			bool insertY=false;
			for (int i=0;i<nPointsQuery;i++)
			{
				int numS=0;
				int siz=NumArray[i];
				for (int j=0;j<siz;j++)
				{
					if (samePoint.count(IndexArray[j+curreIndex]))
					{
						numS=0;

						samePointIte=samePoint.find(IndexArray[j+curreIndex]);
						numS=samePointIte->second.size();
						IndexCandidates.push_back(IndexArray[j+curreIndex]);
						DisCandidates.push_back(IndexArrayDis[j+curreIndex]);
						for (int k=0;k<samePointIte->second.size();k++)
						{
							insertY=true;
							for (int l=0;l<siz;l++)
							{
								if (samePointIte->second[k]==IndexArray[l+curreIndex])
								{
									insertY=false;

								}

							}
							if (insertY==true)
							{
								IndexCandidates.push_back(samePointIte->second[k]);
								DisCandidates.push_back(IndexArrayDis[j+curreIndex]);
							}
							else
								numS--;

						}
						NumArray[i]+=numS;

					}
					else
					{
						IndexCandidates.push_back(IndexArray[j+curreIndex]);
						DisCandidates.push_back(IndexArrayDis[j+curreIndex]);

					}

				}

				curreIndex+=siz;
				CandidatesNum.push_back(NumArray[i]);
			}
			IndexCandidatesStretch.push_back(IndexCandidates);
			CandidatesNumStretch.push_back(CandidatesNum);
			IndexCandidatesDis.push_back(DisCandidates);
			for (int i=0;i<nPointsQuery;i++)
			{
				free(QueriesArray[i]->coordinates);
				free(QueriesArray[i]);
			}
			free(IndexArray);
			free(IndexFilterArray);
			free(NumArray);
			free(QueriesArray);
			free(IndexArrayDis);
		}
		else
		{
			vector <float> DisCandidates;
			vector<IntT> IndexCandidates;
			vector<IntT> CandidatesNum;//ÿ���㷵�ص���Ŀ����Vector
			IndexCandidatesStretch.push_back(IndexCandidates);
			CandidatesNumStretch.push_back(CandidatesNum);
			IndexCandidatesDis.push_back(DisCandidates);

		}



		



		lastTime=clock();
		OneSongLSHTime=(double)(lastTime-firstTimeTempLSH1)/CLOCKS_PER_SEC;
		totalLSHOnlyNoteTime+=OneSongLSHTime;
		OneSongLSHTime=(double)(lastTime-firstTime)/CLOCKS_PER_SEC;
		totalLSHTime+=OneSongLSHTime;
		//if (total%500==0 || total >351)
		//{
		//	ofstream outTime("wav.result",ofstream::app);//LSHʱ��
		//	outTime<<"��ǰLSHʱ�䣺"<<OneSongLSHTime<<" ȫ������LSHʱ�䣺"<<totalLSHTime<< endl;
		//	cout<<"��ǰLSHʱ�䣺"<<OneSongLSHTime<<" ȫ������LSHʱ�䣺"<<totalLSHTime<< endl;
		//	outTime.close();
		//}

#endif

		//����Ϊͳ��LSH׼ȷ��
		FloorLevel=FloorLevelInitial;
//		LSHresultRate(wavename,queryPitch.size(),stepFactor,FloorLevel,IndexCandidatesStretch,IndexLSH,IndexLSHNote,StretchStep,IndexCandidatesDis);
		//LSHresult(wavename,queryPitch.size(),stepFactor,FloorLevel,IndexCandidatesStretch,IndexLSH,StretchStep);




		//����ΪLSH�˲�
		map<string ,int> LSHFilterMap;
		LSHFilter(wavename,LSHFilterMap,CandidatesFilter,IndexLSH);
//		LSHresult(wavename,queryPitch.size(),stepFactor,FloorLevel,IndexCandidatesStretch,IndexLSH,StretchStep);


#if 1

		firstTime=clock();

		//int edge=6;
		
		for (int q=0;q<qRecurse;q++)
		{

			int sizeLSH=IndexCandidatesStretch.size();
			vector<float>  queryStretchPitch;
			vector< vector<float> >  CandidatesDataY;
			vector<float>  CandidatesDataYDis;
			vector <string> SongNameMapToDataY;
			int CandidatesSizeInDWT=0;//���ص�Ҫ��ȷƥ�����Ŀ
			map <string , short > SongMapPosition;
#if 0  //������frame�ĺ���
			for	(FloorLevel=FloorLevelInitial,LinearCoe=0;FloorLevel<UpperLimit+StretchStep && LinearCoe<sizeLSH-1 &&LinearCoe< LinearCoeTotal;FloorLevel+=StretchStep,LinearCoe++)
			{
				map <string , vector<pair<short, double>> > SongMapPositionAll;
				//if (FloorLevel>=0.8 || FloorLevel<=1.3)
				//{
				//	StretchStep=0.05;
				//}
				//else
				StretchStep=0.1;
				int StepFactorCurrent=int(stepFactor*stepRatio*FloorLevel/**FloorLevel*/);




				if (q==0)
				{
					IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
						IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
						indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
						SongMapPositionAll,edge,edge*2); //��Ϊȫ��ƥ�䣨���Ǵ�ͷ��ʼ����
				}
				else if (q==3)
				{
					IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
						IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
						indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
						SongMapPositionAll,6,0); //��Ϊȫ��ƥ�䣨���Ǵ�ͷ��ʼ����
				}
				else if (q==4)
				{
					IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
						IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
						indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
						SongMapPositionAll,-6,0); //��Ϊȫ��ƥ�䣨���Ǵ�ͷ��ʼ����
				}
				else if (q==1)
				{
					IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
						IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
						indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
						SongMapPositionAll,4,8); //��Ϊȫ��ƥ�䣨���Ǵ�ͷ��ʼ����
				}
				else if (q==2)
				{
					IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
						IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
						indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
						SongMapPositionAll,-4,-8); //��Ϊȫ��ƥ�䣨���Ǵ�ͷ��ʼ����
				}


			}
#endif	
#if 1 //����Ϊ��������չ
			sizeLSH=IndexCandidatesStretch.size();
			map <string , vector<pair<short, double>> > SongMapPositionAll;
			IndexSignToQueryAndDataVectorHummingMatchLeastALLNote( posPair,IndexCandidatesStretch[sizeLSH-1],CandidatesNumStretch[sizeLSH-1],FloorLevel,
				IndexLSHNote,5,queryPitchNote,queryStretchPitch,CandidatesDataY,
				indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
				SongMapPositionAll,edge,edge*2);
#endif	 
			lastTime=clock();
			OneSongLSHTime=(double)(lastTime-firstTimeTempLSH1)/CLOCKS_PER_SEC;
			totalLSHRetrievalPostProcessPitchTimeNote+=OneSongLSHTime;
			int sizeCandidates=SongNameMapToDataY.size();
			Mean(queryPitchNote);

			//static int CandidatesDTWAll=0;
			//CandidatesDTWAll+=sizeCandidates;
			Mean(queryStretchPitch);
			int numLSH=0;
			int thresholdMatch=0;

			//vector <string> tempList;
			//vector <float >tempDis1;
			//vector <float >tempDis2;
			//vector <float >tempDis3;

			firstTimeTemp=clock();
			vector <pair<float,int>> DisLS;
			vector <float> DisLSOriginal;
			for (int i=0;i<sizeCandidates;i++)
			{
				if (LSHFilterMap.count(SongNameMapToDataY[i]))
				{
					numLSH=LSHFilterMap[SongNameMapToDataY[i]];
					if (numLSH>thresholdMatch)
					{
						Mean(CandidatesDataY[i]);
						vector <float> candidat;
						float pitch_query=0;
						int left=0;
						int right=0;
						//int Cedge=edge*(0.7+((float)i)/sizeCandidates);
						float lsDis=CalculateOptimalEdge(queryStretchPitch,CandidatesDataY[i],left,right,edge*2,((float)i)/sizeCandidates);
						DisLSOriginal.push_back(lsDis);

						DisLS.push_back(make_pair(lsDis,i)); 
					}
				}

			}
			stable_sort(DisLS.begin(),DisLS.end(),sortRule);

			set<int> LSResult;
			int LSLeft=DisLS.size()*LSratio;
			if (LSLeft<20)
			{
				LSLeft=DisLS.size();
			}
			for (int i=0;i<LSLeft;i++)
			{

				LSResult.insert(DisLS[i].second);

			}
			lastTimeTemp=clock();
			OneSongLSTime=(double)(lastTimeTemp-firstTimeTemp)/CLOCKS_PER_SEC;
			totalLSTime+=OneSongLSTime;
			firstTimeTemp=clock();
			int disEmdNum=-1;
			lastTimeTemp=clock();
			OneSongEMDTime=(double)(lastTimeTemp-firstTimeTemp)/CLOCKS_PER_SEC;
			totalEMDTime+=OneSongEMDTime;
			firstTimeTemp=clock();
			for (int i=0;i<sizeCandidates;i++)
			{
				numLSH=LSHFilterMap[SongNameMapToDataY[i]];
				if (numLSH>thresholdMatch)
				{
					disEmdNum++;
				}

				if (query.n==0 || (LSHFilterMap.count(SongNameMapToDataY[i]) && LSResult.count(i)))
				{

					if (numLSH>thresholdMatch)
					{
						Mean(CandidatesDataY[i]);
						int QuerySize=queryStretchPitch.size();
						int DataYSize=CandidatesDataY[i].size();
						int sizeQandD=CandidatesDataY[i].size();
						int ultimateNum=0;
						float distanceM=10000000;
						float distanceF=10000000;
						float distanceM1=10000000;
						float distanceM2=10000000;
						float distanceM3=10000000;

						if (0/*recur==10*/)
						{
							MeanPlus(CandidatesDataY[i],0);
							distanceM1=RAPositionVarianceOptimal(queryStretchPitch,CandidatesDataY[i],3,5,1,ultimateNum)/ultimateNum;
						}
						else
						{
							for (int k=0;k<5;k++)
							{
								ultimateNum=0;
								float plus=(float(k-2))/2;
								MeanPlus(CandidatesDataY[i],plus);
								distanceF=RAPositionVarianceOptimal(queryStretchPitch,CandidatesDataY[i],3,5,1,ultimateNum)/ultimateNum;
								distanceM1=MyMinTwo(distanceM1,distanceF);
							}
						}
						
						tempDis1.push_back(distanceM1);
						for (int k=0;k<0;k++)
						{
							ultimateNum=0;
							float plus=(float(k-2.5))/2;
							MeanPlus(CandidatesDataY[i],plus);
							distanceF=RAPositionVarianceOptimal(queryStretchPitch,CandidatesDataY[i],3,5,1,ultimateNum)/ultimateNum;
							distanceM2=MyMinTwo(distanceM2,distanceF);
						}
						if (QuerySize<2)
						{
							QuerySize=2;
						}
						distanceM2=((float)DataYSize)/QuerySize;
						distanceM2=abs(distanceM2-1);
						tempDis2.push_back(distanceM2);
						ultimateNum=0;
						MeanPlus(CandidatesDataY[i],0);
						//distanceM3=RAPositionVarianceOptimal(queryStretchPitch,CandidatesDataY[i],3,5,1,ultimateNum)/ultimateNum;

						distanceM3=((float)DataYSize)/QuerySize;
						if (distanceM3>1)
						{
							distanceM3=distanceM3-1;
						}
						else
							distanceM3=(1-distanceM3)*2;
						tempDis3.push_back(distanceM3);
						//distanceM=distanceM1*0.25+distanceM2*0.5+distanceM3*0.25;
						distanceM=distanceM1*0.8+distanceM2*0.2;
						//float distanceM=RAPositionVarianceOptimal(queryStretchPitch,CandidatesDataY[i],3,5,1,ultimateNum)/ultimateNum;
						//float distanceMNote=RAPositionVarianceOptimal(queryPitchNote,CandidatesDataY[i],3,5,1,ultimateNum)/ultimateNum;
						//distanceM=MyMinTwo(distanceM,distanceMNote);
						//distanceM=distanceM*distanceMNote;
						//distanceM=distanceM*0.5+distanceMNote*0.0;
						//distanceM=DisLSOriginal[i];
						//distanceM=DisEmdOriginal[i];
						//if (query.n!=0 )
						//{
						//	//distanceM=DisLSOriginal[disEmdNum]*0.4/*+DisEmdOriginal[disEmdNum]*0.2*/+distanceM*0.6;
						//	//distanceM=MyMin(distanceM,100,DisEmdOriginal[disEmdNum]);
						//}




						//			vector<vector<double>> matrixD(QuerySize,vector<double>(DataYSize)); //��ΪDTWƥ�䡣Ч������
						//			double distanceM=DTWBeginQueryAndSongSeven(queryStretchPitch,CandidatesDataY[i],matrixD)/QuerySize;
						Dis.push_back(distanceM);
						string songName=SongNameMapToDataY[i];

						if (songDis.count(distanceM))
						{
							*(Dis.end()-1)+=0.00001;
							songDis.insert(make_pair(distanceM+0.00001,songName)); 
						}
						else
							songDis.insert(make_pair(distanceM,songName)); 
						songName.erase(0,6);
						int posSong=songName.rfind("pv");
						//songNameFive.erase(5,6);
						songName.erase(posSong-1,3);
						tempList.push_back(songName);
					}
				}

			}
			lastTimeTemp=clock();
			OneSongRATime=(double)(lastTimeTemp-firstTimeTemp)/CLOCKS_PER_SEC;
			totalRATime+=OneSongRATime;
			CandidatesDTWAll+=Dis.size();
			string nameSong(wavename);

			string nameSongResult;
			stable_sort(Dis.begin(),Dis.end());
			set <string> SongFindAlready;
			//float ratio=ratioAll;
			float ratio=0.0;
			string songNameFive;
			for (int i=0;i!=songDis.size()&&i<=1000 ;i++)
			{
				songNameFive=songDis.find(Dis[i])->second;
				if (SongFindAlready.count(songNameFive))
				{
					;
				}
				else
				{

					SongFindAlready.insert(songNameFive);
				}
				if (SongFindAlready.size()==2)
				{

					if (Dis[0]/Dis[i]<=ratio)
					{
						returnN=true;
						recur+=3;
					}
					i+=10000;
				}
			}
			if (returnN==true)
			{		
				nameSongResult.assign(nameSong,0,nameSong.size()-4);
				ResultDis1.insert(make_pair(nameSongResult,tempDis1));
				ResultDis2.insert(make_pair(nameSongResult,tempDis2));
				ResultDis3.insert(make_pair(nameSongResult,tempDis3));
				ResultLable1.insert(make_pair(nameSongResult,tempList));
			}


		}
} //���������ļ����Ľ�β
		lastTimeTempLSH1=clock();
		OneSonglshnoteTime=(double)(lastTimeTempLSH1-firstTimeTempLSH1)/CLOCKS_PER_SEC;
		totalLSHNoteTime+=OneSonglshnoteTime;

		if (recur==0 || recur==3)
		{
			lastTimeTempLSH1=clock();
			OneSonglshPitchTime=(double)(lastTimeTempLSH1-firstTimeTempLSH1)/CLOCKS_PER_SEC;
			totalLSHNoteFirstTime+=OneSonglshPitchTime;
		}


		if (returnN==false)
		{


			firstTimeTempLSH2=clock();
			//vector<vector<IntT>> IndexCandidatesStretch;
			//vector<vector<float>> IndexCandidatesDis;
			//vector<vector<IntT>> CandidatesNumStretch;
			//vector<IntT> CandidatesFilter;
			IndexCandidatesStretch.clear();
			IndexCandidatesDis.clear();
			CandidatesNumStretch.clear();
			CandidatesFilter.clear();

#if 1 //����Ϊ֡��LSH����


			LinearCoe=0;
			FloorLevel=FloorLevelInitial;
			//if (recur==0)
			//{
			//	StretchStep=0.2;
			//}
			//else
			//	StretchStep=0.1;
			for (;FloorLevel<UpperLimit+StretchStep && LinearCoe< LinearCoeTotal;FloorLevel+=StretchStep)
			{
				PPointT *QueriesArray=NULL;//Ҫfree
				/*if (FloorLevel>=0.8 || FloorLevel<=1.3)
				{
				StretchStep=0.05;
				}
				else*/
				//StretchStep=0.1;
				if ((recur==1 || recur==2 )&& FloorLevel<=1.01 && FloorLevel>=0.99)
				{
					continue;
				}
				if (recur==2 && ((FloorLevel<=0.81 && FloorLevel>=0.79)||(FloorLevel<=1.21 && FloorLevel>=1.19)||(FloorLevel<=1.41 && FloorLevel>=1.39)))
				{
					continue;
				}
				if (LSHQueryVectorLinearStretching[LinearCoe].size()>0)
				{
					QueriesArray=readDataSetFromVector(LSHQueryVectorLinearStretching[LinearCoe]);
					Int32T nPointsQuery=LSHQueryVectorLinearStretching[LinearCoe].size();
					IntT dimension=20;
					if (nPointsQuery>0)
					{
						dimension=LSHQueryVectorLinearStretching[LinearCoe][0].size();
					}
					IntT LSHFilterReturnNum=0;
					IntT *IndexArray=NULL;
					int IndexArraySize=100000;//�����Һ�ѡ��Ŀ
					IndexArray=(IntT *)MALLOC(IndexArraySize *sizeof(IntT));
					double *IndexArrayDis=(double *)MALLOC(IndexArraySize *sizeof(double));
					IntT *IndexFilterArray=NULL;
					IndexFilterArray=(IntT *)MALLOC(IndexArraySize/2 *sizeof(IntT));
					IntT * NumArray=(IntT *)MALLOC(nPointsQuery *sizeof(IntT));//ÿ���㷵�ص���Ŀ
					IntT ResultSize=LSHStructToResultOnePointRetainSeveral(QueriesArray,nPointsQuery,IndexArraySize, 
						IndexArray,IndexHuming,NumArray,RetainNum , dimension,LSHFilterNum,IndexFilterArray,LSHFilterReturnNum,IndexArrayDis);//�õ������ÿ���㷵��RetainNum����ѡ
					vector<IntT> IndexCandidates;
					vector <float> DisCandidates;
					vector<IntT> CandidatesNum;//ÿ���㷵�ص���Ŀ����Vector
					int curreIndex=0;
					//for (int i=0;i<ResultSize;i++)
					//{
					//	IndexCandidates.push_back(IndexArray[i]);
					//	DisCandidates.push_back(IndexArrayDis[i]);
					//}
					for (int i=0;i<LSHFilterReturnNum;i++)
					{
						CandidatesFilter.push_back(IndexFilterArray[i]);
					}
					bool insertY=false;
					for (int i=0;i<nPointsQuery;i++)
					{
						int numS=0;
						int siz=NumArray[i];
						for (int j=0;j<siz;j++)
						{
							if (samePoint.count(IndexArray[j+curreIndex]))
							{
								numS=0;

								samePointIte=samePoint.find(IndexArray[j+curreIndex]);
								numS=samePointIte->second.size();
								IndexCandidates.push_back(IndexArray[j+curreIndex]);
								DisCandidates.push_back(IndexArrayDis[j+curreIndex]);
								for (int k=0;k<samePointIte->second.size();k++)
								{
									insertY=true;
									for (int l=0;l<siz;l++)
									{
										if (samePointIte->second[k]==IndexArray[l+curreIndex])
										{
											insertY=false;

										}

									}
									if (insertY==true)
									{
										IndexCandidates.push_back(samePointIte->second[k]);
										DisCandidates.push_back(IndexArrayDis[j+curreIndex]);
									}
									else
										numS--;

								}
								NumArray[i]+=numS;

							}
							else
							{
								IndexCandidates.push_back(IndexArray[j+curreIndex]);
								DisCandidates.push_back(IndexArrayDis[j+curreIndex]);

							}

						}

						curreIndex+=siz;
						CandidatesNum.push_back(NumArray[i]);
					}
					IndexCandidatesStretch.push_back(IndexCandidates);
					CandidatesNumStretch.push_back(CandidatesNum);
					IndexCandidatesDis.push_back(DisCandidates);
					for (int i=0;i<nPointsQuery;i++)
					{
						free(QueriesArray[i]->coordinates);
						free(QueriesArray[i]);
					}
					free(IndexArray);
					free(IndexFilterArray);
					free(NumArray);
					free(QueriesArray);
					free(IndexArrayDis);
				}
				else
				{
					vector <float> DisCandidates;
					vector<IntT> IndexCandidates;
					vector<IntT> CandidatesNum;//ÿ���㷵�ص���Ŀ����Vector
					IndexCandidatesStretch.push_back(IndexCandidates);
					CandidatesNumStretch.push_back(CandidatesNum);
					IndexCandidatesDis.push_back(DisCandidates);

				}
				LinearCoe++;

			}

#endif



			lastTimeTempLSH2=clock();
			OneSonglshPitchTime=(double)(lastTimeTempLSH2-firstTimeTempLSH2)/CLOCKS_PER_SEC;
			totalLSHOnlyPitchTime+=OneSonglshPitchTime;

			for (int q=0;q<qRecurse;q++)
			{

				int sizeLSH=IndexCandidatesStretch.size();
				vector<float>  queryStretchPitch;
				vector< vector<float> >  CandidatesDataY;
				vector<float>  CandidatesDataYDis;
				vector <string> SongNameMapToDataY;
				int CandidatesSizeInDWT=0;//���ص�Ҫ��ȷƥ�����Ŀ
				map <string , short > SongMapPosition;
#if 1  //������frame�ĺ���
				for	(FloorLevel=FloorLevelInitial,LinearCoe=0;FloorLevel<UpperLimit+StretchStep && LinearCoe<sizeLSH &&LinearCoe< LinearCoeTotal;FloorLevel+=StretchStep)
				{
					map <string , vector<pair<short, double>> > SongMapPositionAll;
					//if (FloorLevel>=0.8 || FloorLevel<=1.3)
					//{
					//	StretchStep=0.05;
					//}
					//else
					/*StretchStep=0.1;*/
					if ((recur==1 || recur==2 )&& FloorLevel<=1.01 && FloorLevel>=0.99)
					{
						continue;
					}
					if (recur==2 && ((FloorLevel<=0.81 && FloorLevel>=0.79)||(FloorLevel<=1.21 && FloorLevel>=1.19)||(FloorLevel<=1.41 && FloorLevel>=1.39)))
					{
						continue;
					}
					int StepFactorCurrent=int(stepFactor*stepRatio*FloorLevel/**FloorLevel*/);




					if (q==0)
					{
						IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
							IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
							indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
							SongMapPositionAll,edge,edge*2); //��Ϊȫ��ƥ�䣨���Ǵ�ͷ��ʼ����
					}
					else if (q==3)
					{
						IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
							IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
							indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
							SongMapPositionAll,6,0); //��Ϊȫ��ƥ�䣨���Ǵ�ͷ��ʼ����
					}
					else if (q==4)
					{
						IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
							IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
							indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
							SongMapPositionAll,-6,0); //��Ϊȫ��ƥ�䣨���Ǵ�ͷ��ʼ����
					}
					else if (q==1)
					{
						IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
							IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
							indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
							SongMapPositionAll,4,8); //��Ϊȫ��ƥ�䣨���Ǵ�ͷ��ʼ����
					}
					else if (q==2)
					{
						IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
							IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
							indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
							SongMapPositionAll,-4,-8); //��Ϊȫ��ƥ�䣨���Ǵ�ͷ��ʼ����
					}
					LinearCoe++;


				}
				lastTimeTempLSH2=clock();
				OneSonglshPitchTime=(double)(lastTimeTempLSH2-firstTimeTempLSH2)/CLOCKS_PER_SEC;
				totalLSHRetrievalPostProcessPitchTime+=OneSonglshPitchTime;
#endif	
#if 0 //����Ϊ��������չ
				sizeLSH=IndexCandidatesStretch.size();
				map <string , vector<pair<short, double>> > SongMapPositionAll;
				IndexSignToQueryAndDataVectorHummingMatchLeastALLNote( posPair,IndexCandidatesStretch[sizeLSH-1],CandidatesNumStretch[sizeLSH-1],FloorLevel,
					IndexLSHNote,5,queryPitchNote,queryStretchPitch,CandidatesDataY,
					indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
					SongMapPositionAll,edge,edge*2);
#endif	 
				int sizeCandidates=SongNameMapToDataY.size();
				Mean(queryPitchNote);

				//static int CandidatesDTWAll=0;
				//CandidatesDTWAll+=sizeCandidates;
				Mean(queryStretchPitch);
				int numLSH=0;
				int thresholdMatch=0;



				firstTimeTemp=clock();
				vector <pair<float,int>> DisLS;
				vector <float> DisLSOriginal;
				for (int i=0;i<sizeCandidates;i++)
				{

					Mean(CandidatesDataY[i]);
					vector <float> candidat;
					float pitch_query=0;
					int left=0;
					int right=0;
					//int Cedge=edge*(0.7+((float)i)/sizeCandidates);
					float lsDis=CalculateOptimalEdge(queryStretchPitch,CandidatesDataY[i],left,right,edge*2,((float)i)/sizeCandidates);
					DisLSOriginal.push_back(lsDis);

					DisLS.push_back(make_pair(lsDis,i)); 
				}

				stable_sort(DisLS.begin(),DisLS.end(),sortRule);

				set<int> LSResult;
				int LSLeft=DisLS.size()*LSratio;
				if (LSLeft<20)
				{
					LSLeft=DisLS.size();
				}
				for (int i=0;i<LSLeft;i++)
				{

					LSResult.insert(DisLS[i].second);

				}
				lastTimeTemp=clock();
				OneSongLSTime=(double)(lastTimeTemp-firstTimeTemp)/CLOCKS_PER_SEC;
				totalLSTime+=OneSongLSTime;
				firstTimeTemp=clock();
				int disEmdNum=-1;
				lastTimeTemp=clock();
				OneSongEMDTime=(double)(lastTimeTemp-firstTimeTemp)/CLOCKS_PER_SEC;
				totalEMDTime+=OneSongEMDTime;
				firstTimeTemp=clock();
				for (int i=0;i<sizeCandidates;i++)
				{
					disEmdNum++;

					if ( LSResult.count(i))

					{

						Mean(CandidatesDataY[i]);
						//vector <float> candidat;
						//float pitch_query=0;

						//pitch_query=((float)queryStretchPitch.size())/CandidatesDataY[i].size();
						//StringToStringSame(CandidatesDataY[i],candidat,pitch_query);
						//StringTosignature(candidat,candiY);
						int QuerySize=queryStretchPitch.size();
						int DataYSize=CandidatesDataY[i].size();
						int sizeQandD=CandidatesDataY[i].size();
						int ultimateNum=0;
						float distanceM=10000000;
						float distanceF=10000000;
						float distanceM1=10000000;
						float distanceM2=10000000;
						float distanceM3=10000000;

						if (0/*recur==10*/)
						{
							MeanPlus(CandidatesDataY[i],0);
							distanceM1=RAPositionVarianceOptimal(queryStretchPitch,CandidatesDataY[i],3,5,1,ultimateNum)/ultimateNum;
						}
						else
						{
							for (int k=0;k<5;k++)
							{
								ultimateNum=0;
								float plus=(float(k-2))/2;
								MeanPlus(CandidatesDataY[i],plus);
								distanceF=RAPositionVarianceOptimal(queryStretchPitch,CandidatesDataY[i],3,5,1,ultimateNum)/ultimateNum;
								distanceM1=MyMinTwo(distanceM1,distanceF);
							}
						}
						tempDis1.push_back(distanceM1);
						for (int k=0;k<0;k++)
						{
							ultimateNum=0;
							float plus=(float(k-2.5))/2;
							MeanPlus(CandidatesDataY[i],plus);
							distanceF=RAPositionVarianceOptimal(queryStretchPitch,CandidatesDataY[i],3,5,1,ultimateNum)/ultimateNum;
							distanceM2=MyMinTwo(distanceM2,distanceF);
						}
						if (QuerySize<2)
						{
							QuerySize=2;
						}
						distanceM2=((float)DataYSize)/QuerySize;
						distanceM2=abs(distanceM2-1);
						tempDis2.push_back(distanceM2);
						ultimateNum=0;
						MeanPlus(CandidatesDataY[i],0);
						//distanceM3=RAPositionVarianceOptimal(queryStretchPitch,CandidatesDataY[i],3,5,1,ultimateNum)/ultimateNum;

						distanceM3=((float)DataYSize)/QuerySize;
						if (distanceM3>1)
						{
							distanceM3=distanceM3-1;
						}
						else
							distanceM3=(1-distanceM3)*2;
						//distanceM3=RAPositionVarianceOptimal(queryStretchPitch,CandidatesDataY[i],3,5,1,ultimateNum)/ultimateNum;

						tempDis3.push_back(distanceM3);
						//distanceM=distanceM1*0.25+distanceM2*0.5+distanceM3*0.25;
						distanceM=distanceM1*0.8+distanceM2*0.2;;




						Dis.push_back(distanceM);
						string songName=SongNameMapToDataY[i];

						if (songDis.count(distanceM))
						{
							*(Dis.end()-1)+=0.00001;
							songDis.insert(make_pair(distanceM+0.00001,songName)); 
						}
						else
							songDis.insert(make_pair(distanceM,songName)); 
						songName.erase(0,6);
						int posSong=songName.rfind("pv");
						//songNameFive.erase(5,6);
						songName.erase(posSong-1,3);
						tempList.push_back(songName);
					}
				}
				lastTimeTemp=clock();
				OneSongRATime=(double)(lastTimeTemp-firstTimeTemp)/CLOCKS_PER_SEC;
				totalRATime+=OneSongRATime;
				CandidatesDTWAll+=Dis.size();
				string nameSong(wavename);

				string nameSongResult;

				stable_sort(Dis.begin(),Dis.end());
				set <string> SongFindAlready;
				//float ratio=ratioAll;
				//float ratio=0.74;
				float ratio=110.00;
				if (recur==10)
				{
					ratio=1.59;
				}
				else if (recur==11)
				{
					ratio=1.47;
				}
				string songNameFive;
				float Thr[1000]={0.0};
				int num=0;
					
				for (int i=0;i!=songDis.size()&&i<=1000 ;i++)
				{
					songNameFive=songDis.find(Dis[i])->second;
					if (SongFindAlready.count(songNameFive))
					{
						;
					}
					else
					{

						num++;
						SongFindAlready.insert(songNameFive);
						if (SongFindAlready.size()==num && num<1000)
						{
							Thr[num-1]=Dis[i];
							float totalDis=0;
							for (int l=1;l<num;l++)
							{
								totalDis+=Thr[l];
							}
							if (SongFindAlready.size()==5)
							{

								if ( totalDis/(Dis[0]*(num-1)) >=ratio)
								{
									returnN=true;
									recur+=3;
								}
								i+=10000;
							}
							
						}
					}
					
					
				}
				if (returnN==true || recur>=2)
				{		
					nameSongResult.assign(nameSong,0,nameSong.size()-4);
					ResultDis1.insert(make_pair(nameSongResult,tempDis1));
					ResultDis2.insert(make_pair(nameSongResult,tempDis2));
					ResultDis3.insert(make_pair(nameSongResult,tempDis3));
					ResultLable1.insert(make_pair(nameSongResult,tempList));
				}

			}
			if (recur==0 || recur==3)
			{
				lastTimeTempLSH2=clock();
				OneSonglshPitchTime=(double)(lastTimeTempLSH2-firstTimeTempLSH2)/CLOCKS_PER_SEC;
				totalLSHPitchFirstTime+=OneSonglshPitchTime;
			}

			lastTimeTempLSH2=clock();
			OneSonglshPitchTime=(double)(lastTimeTempLSH2-firstTimeTempLSH2)/CLOCKS_PER_SEC;
			totalLSHPitchTime+=OneSonglshPitchTime;

		}
		if (recur==0 || recur==3)
		{
			lastTimeTempLSH2=clock();
			OneSonglshPitchTime=(double)(lastTimeTempLSH2-firstTimeTempLSH1)/CLOCKS_PER_SEC;
			totalLSHoneTime+=OneSonglshPitchTime;
		}
		else if (recur==1 || recur==4)
		{
			lastTimeTempLSH2=clock();
			OneSonglshPitchTime=(double)(lastTimeTempLSH2-firstTimeTempLSH1)/CLOCKS_PER_SEC;
			totalLSHtwoTime+=OneSonglshPitchTime;
		}
		else if (recur==2 || recur==5)
		{
			lastTimeTempLSH2=clock();
			OneSonglshPitchTime=(double)(lastTimeTempLSH2-firstTimeTempLSH1)/CLOCKS_PER_SEC;
			totalLSHthreeTime+=OneSonglshPitchTime;
		}

	}







	if(NULL!=NoteEmd){
		free (NoteEmd);
		NoteEmd=NULL;
		query.Features=NULL;
	}
	if(NULL!=NoteDuration){
		free (NoteDuration);
		NoteDuration=NULL;
		query.Weights=NULL;
	}
	if(NULL!=candiY.Features){
		free (candiY.Features);
		candiY.Features=NULL;
	}
	if(NULL!=candiY.Weights){
		free (candiY.Weights);
		candiY.Weights=NULL;
	}
	


	//if (total%500==0 || total >310)
	//{
	//	ofstream outTime("wav.result",ofstream::app);//LSHʱ��
	//	outTime<<" ȫ������RAʱ�䣺"<<totalRATime<<"EMDʱ�䣺"<<totalEMDTime<< "LSʱ�䣺"<<totalLSTime<<endl;
	//	cout<<" ȫ������RAʱ�䣺"<<totalRATime<<"EMDʱ�䣺"<<totalEMDTime<< "LSʱ�䣺"<<totalLSTime<<endl;
	//	outTime.close();
	//}

	stable_sort(Dis.begin(),Dis.end());
	string   songNameFive;
	ofstream outf("wav.result",ofstream::app);//ʶ���������ļ�
	ofstream outfCandidates("wavCandidates.result",ofstream::app);

	static int num1=0;
	static int num2=0;
	static int num3=0;
	static int num4=0;
	static int num5=0;
	static int num10=0;
	static int num20=0;
	static int num50=0;
	static int findall=0;
	static float thresholdC=10;
	static float dis1=0;
	static float dis2=0;
	static string tempSt;
	
	static int rightNum[1101]={0};
	static int wrongNum[1101]={0};
	int findtop5=0;
	bool wrongA=TRUE;
	static map <string,int> resultMap;
	//for (int i=0;i!=Dis.size();i++)
	set <string> SongFindAlready;
	string firstSong;
	int findNum=0;//�����������Ŀ�Լ�ͳ�Ƶڼ����ҵ���
	float ThrCal[1000]={0.0};
	int numCal=0;
	static float ThrRes[1000]={0.0};
	//for (int i=0;i<1000;i++)
	//{
	//	ThrRes[i]=10.0;
	//}
	for (int i=0,k=0;i!=songDis.size()&&k<5 && i!= Dis.size() && k<500;i++)
	{
		if (songDis.count(Dis[i]))
		{
		songNameFive=songDis.find(Dis[i])->second;
		
		if (SongFindAlready.count(songNameFive))
		{
			;
		}
		else
		{
			k++;
			
		SongFindAlready.insert(songNameFive);
		int posSong=songNameFive.rfind("pv");
		//songNameFive.erase(5,6);
		songNameFive.erase(posSong-1,3);
		songNameFive.erase(0,6);
		
		songFive.push_back(songNameFive);
		}
		}
		
	}
	while (songFive.size()<5)
	{
		songFive.push_back("NONE");
	}

	outf.close();
#endif	
	return 0;

}

//������Ϊ�������������Ϊ5000newpv355.txt �е�pv�ļ��������������ļ����ڳ�������Ŀ¼/5355P�ļ����У�
//�����param��������Ϣ
//songIDAndName������ID�͸���ӳ��
//IndexHumingLocal������֡������
//IndexHumingLocalNote����������������
int indexRead(ParamInfo *param,	map <string ,string> &songIDAndName,
	PRNearNeighborStructT& IndexHumingLocal, PRNearNeighborStructT& IndexHumingLocalNote)
{
	string fileName("wavall355.txt");	

	/*
	//���߳��·ָ�����
	int ThreadNum=1;
	if (ThreadNum>=100)
	{
		ThreadNum=100;//���100���߳�
	}
	OneFileToMultiFile(fileName,ThreadNum);	//�����߳����ָ��ļ�
	*/

	string dataIndex="5000newpv355.txt";	//pv�б��ļ�������./5355P�е�pv�ļ�����������
	vector<vector<float>> LSHVector;	//��¼LSH�㣬ÿ����Ϊһ����������

	//LSH���ȡ����
	int StepFactor=3;	//ѡ��LSH��ļ����������������ȡһ���㣩
	int LSHsize = 20;	//LSH������һ��LSH��Ĵ�С��20֡*3��2.4��
	int LSHshift = StepFactor*5;	//LSH����,15֡��0.6��
	int maxFrame = StepFactor*9*120*5;	//һά�����������֡��

	//NLSH���ȡ����
	int noteMaxFrame = 10;	//һ�����ߵ��֡�����������з֣�
	int NLSHsize = 10;	//NLSH������һ��LSH��Ĵ�С

	PPointT *dataSet=NULL; 
	PPointT *dataSetNote=NULL; 

	IntT RetainNum=3;	//ÿ����������ĵ���
	float stepRatio=1.5;	//query��LSH�任֡��
	IntT LSHFilterNum=10;	//LSH�˲������ĵ���

	//����֡�����������Ĳ����б�
	strcpy(param[0].wavename,fileName.c_str());
	strcpy(param[1].wavename,fileName.c_str());
	param[0].RetainNum=RetainNum;
	param[1].RetainNum=5;
	param[0].stepFactor=StepFactor;
	param[1].stepFactor=StepFactor;
	param[0].LSHFilterNum=LSHFilterNum;
	param[1].LSHFilterNum=200;
	param[0].stepRatio=stepRatio;
	param[1].stepRatio=stepRatio;

	//����pv�ļ�������ά��(����������ʱ��)����ת��Ϊһά��������
	//���룺dataIndex: ���������б��ļ�
	//�����indexSongName: �ļ�·����5355P\\XXX.pv���Ͷ�Ӧpv�ļ���һά�������е�map
	readIndexPitch8MinutesNewPv(dataIndex, param[0].indexSongName);	

	//��һά�������г�ȡLSH�㣬ȥ����ƥ�俪ͷ�Ĺ���
	//���룺indexSongName��pv�ļ����Ͷ�Ӧ��һά��������
	//StepFactor��ѡ��LSH��ļ�����������֡��ȡһ����
	//LSHsize��LSH������һ��LSH��Ĵ�С
	//LSHshift��LSH����
	//maxFrame��һά�����������֡��
	//�����LSHVector����¼LSH�㣬ÿ����Ϊһ����������
	//IndexLSH,	��¼LSH����������LSHVector����ţ���·�����ļ�������ʼ����λ��
	IndexPitchToLSHVector(param[0].indexSongName, StepFactor, LSHsize, LSHshift, maxFrame, 
		LSHVector, param[0].IndexLSH);

	//��LSH���������ж����ݼ�������dataSet�У�����ÿ��LSH��ʱ��¼��ź�LSH���ƽ����
	dataSet = readDataSetFromVector(LSHVector);

	//����MIT��r-NN��Դ�����������
	Int32T nPointsData = LSHVector.size();	//LSH�㼯��С
	IntT dimension = LSHsize;	//ÿ����ά������LSH����
	IntT nSampleQueries = 100;	//������������
	float thresholdR = 18;		//����
	MemVarT memoryUpperBound = 600000000;	//�ڴ�����

	if (nPointsData>0)
	{
		dimension = LSHVector[0].size();	//��¼��ʵά��
	}
	LSHVector.clear();	//������м�����������Ѷ���dataSet��

	//����MIT��r-NN��Դ�����ʼ��LSH���ݽṹ
	//���룺dataSet��LSH�㼯
	//dimension��ÿ��LSH��ά��
	//nPointsData��LSH�㼯��С
	//nSampleQueries��������������
	//thresholdR������
	//memoryUpperBound���ڴ�����
	//�����IndexHuming
	LSHDataStruct(dataSet, dimension, nPointsData, nSampleQueries,
		thresholdR, memoryUpperBound, param[0].IndexHuming);
	
	//��һά�������г�ȡ����������LSH��
	//���룺indexSongName��pv�ļ����Ͷ�Ӧ��һά��������
	//noteMaxFrame��һ�����ߵ��֡�����������з֣�
	//NLSHsize��LSH������һ��LSH��Ĵ�С
	//maxFrame��һά�����������֡��
	//�����LSHVector��LSH�㣬ÿ����Ϊһ��10ά����������
	//IndexLSH,	LSH��������¼��LSHVector����ţ�ÿ��LSH�����ʼλ�ã�����֡��
	IndexPitchToLSHVectorNote(param[0].indexSongName, noteMaxFrame, NLSHsize, maxFrame, 
		LSHVector, param[1].IndexLSHNote);

	//��NLSH���������ж����ݼ�������dataSetNote�У�����ÿ��LSH��ʱ��¼��ź�LSH���ƽ����
	dataSetNote=readDataSetFromVector(LSHVector);

	//����MIT��r-NN��Դ�����������
	nPointsData = LSHVector.size();	//NLSH�㼯��С
	dimension = NLSHsize;	//ÿ����ά������NLSH����
	nSampleQueries = 100;	//������������
	thresholdR = 7;	//����
	memoryUpperBound = 600000000;	//�ڴ�����

	if (nPointsData>0)
	{
		dimension = LSHVector[0].size();	//��¼��ʵά��
	}
	LSHVector.clear();	//������м�����������Ѷ���dataSet��
	samePoint.clear();

	//����MIT��r-NN��Դ�����ʼ��LSH���ݽṹ
	//���룺dataSetNote��NLSH�㼯
	//dimension��ÿ��LSH��ά��
	//nPointsData��LSH�㼯��С
	//nSampleQueries��������������
	//thresholdR������
	//memoryUpperBound���ڴ�����
	//�����IndexHuming
	LSHDataStruct(dataSetNote, dimension, nPointsData, nSampleQueries,
		thresholdR, memoryUpperBound, param[1].IndexHuming);

	string IDAndNameFile = "name.txt";
	songread(IDAndNameFile,songIDAndName);	//���ļ��������ID�͸���ӳ���,�ļ�ÿ��Ϊ��ID ����
	
	int totalWave=0;
	string enhance("SpeechEnhance.exe ");
	unsigned int threadID[100];	//��࿪100������

	pParamInfo pci = (ParamInfo*)param;		//��ȡLSH����
	pParamInfo pciNote = (ParamInfo*)param+1; //��ȡNLSH����
	//  do something here

	char buffer[20];
	clock_t firstTime,lastTime;
	double OneSongMatchTime=0;
	static int ThreadCNum=-1;

	WaitForSingleObject(hMutex,INFINITE);	//�̱߳�����ֱ��hMutex��ǵĶ��󱻴���

	ThreadCNum++;
	fileName = pci->wavename;

	string fileN = fileName + itoa(ThreadCNum,buffer,10);
	
	FAILIF(NULL == (IndexHumingLocal = (PRNearNeighborStructT)MALLOC(sizeof(RNearNeighborStructT))));
	IndexHumingLocal->dimension=pci->IndexHuming->dimension;
	IndexHumingLocal->hashedBuckets=pci->IndexHuming->hashedBuckets;
	IndexHumingLocal->hfTuplesLength=pci->IndexHuming->hfTuplesLength;
	IndexHumingLocal->lshFunctions=pci->IndexHuming->lshFunctions;
	IndexHumingLocal->sizeMarkedPoints=pci->IndexHuming->sizeMarkedPoints;
	FAILIF(NULL == (IndexHumingLocal->markedPoints = (BooleanT*)MALLOC(IndexHumingLocal->sizeMarkedPoints * sizeof(BooleanT))));
	for(IntT i = 0; i < IndexHumingLocal->sizeMarkedPoints; i++){
		IndexHumingLocal->markedPoints[i] = FALSE;
	}
	FAILIF(NULL == (IndexHumingLocal->markedPointsIndeces = (Int32T*)MALLOC(IndexHumingLocal->sizeMarkedPoints * sizeof(Int32T))));
	IndexHumingLocal->nHFTuples=pci->IndexHuming->nHFTuples;
	IndexHumingLocal->nPoints=pci->IndexHuming->nPoints;
	IndexHumingLocal->parameterK=pci->IndexHuming->parameterK;
	IndexHumingLocal->parameterL=pci->IndexHuming->parameterL;
	IndexHumingLocal->parameterR=pci->IndexHuming->parameterR;
	IndexHumingLocal->parameterR2=pci->IndexHuming->parameterR2;
	IndexHumingLocal->parameterT=pci->IndexHuming->parameterT;
	IndexHumingLocal->parameterW=pci->IndexHuming->parameterW;
	IndexHumingLocal->points=pci->IndexHuming->points;
	IndexHumingLocal->pointsArraySize=pci->IndexHuming->pointsArraySize;
	FAILIF(NULL == (IndexHumingLocal->pointULSHVectors = (Uns32T**)MALLOC(IndexHumingLocal->nHFTuples * sizeof(Uns32T*))));
	for(IntT i = 0; i < IndexHumingLocal->nHFTuples; i++){
		FAILIF(NULL == (IndexHumingLocal->pointULSHVectors[i] = (Uns32T*)MALLOC(IndexHumingLocal->hfTuplesLength * sizeof(Uns32T))));
	}
	FAILIF(NULL == (IndexHumingLocal->precomputedHashesOfULSHs = (Uns32T**)MALLOC(IndexHumingLocal->nHFTuples * sizeof(Uns32T*))));
	for(IntT i = 0; i < IndexHumingLocal->nHFTuples; i++){
		FAILIF(NULL == (IndexHumingLocal->precomputedHashesOfULSHs[i] = (Uns32T*)MALLOC(N_PRECOMPUTED_HASHES_NEEDED * sizeof(Uns32T))));
	}
	 FAILIF(NULL == (IndexHumingLocal->reducedPoint = (float*)MALLOC(IndexHumingLocal->dimension * sizeof(float))));
	IndexHumingLocal->reportingResult=pci->IndexHuming->reportingResult;
	
	IndexHumingLocal->useUfunctions=pci->IndexHuming->useUfunctions;

	FAILIF(NULL == (IndexHumingLocalNote = (PRNearNeighborStructT)MALLOC(sizeof(RNearNeighborStructT))));
	IndexHumingLocalNote->dimension=pciNote->IndexHuming->dimension;
	IndexHumingLocalNote->hashedBuckets=pciNote->IndexHuming->hashedBuckets;
	IndexHumingLocalNote->hfTuplesLength=pciNote->IndexHuming->hfTuplesLength;
	IndexHumingLocalNote->lshFunctions=pciNote->IndexHuming->lshFunctions;
	IndexHumingLocalNote->sizeMarkedPoints=pciNote->IndexHuming->sizeMarkedPoints;
	FAILIF(NULL == (IndexHumingLocalNote->markedPoints = (BooleanT*)MALLOC(IndexHumingLocalNote->sizeMarkedPoints * sizeof(BooleanT))));
	for(IntT i = 0; i < IndexHumingLocalNote->sizeMarkedPoints; i++){
		IndexHumingLocalNote->markedPoints[i] = FALSE;
	}
	FAILIF(NULL == (IndexHumingLocalNote->markedPointsIndeces = (Int32T*)MALLOC(IndexHumingLocalNote->sizeMarkedPoints * sizeof(Int32T))));
	IndexHumingLocalNote->nHFTuples=pciNote->IndexHuming->nHFTuples;
	IndexHumingLocalNote->nPoints=pciNote->IndexHuming->nPoints;
	IndexHumingLocalNote->parameterK=pciNote->IndexHuming->parameterK;
	IndexHumingLocalNote->parameterL=pciNote->IndexHuming->parameterL;
	IndexHumingLocalNote->parameterR=pciNote->IndexHuming->parameterR;
	IndexHumingLocalNote->parameterR2=pciNote->IndexHuming->parameterR2;
	IndexHumingLocalNote->parameterT=pciNote->IndexHuming->parameterT;
	IndexHumingLocalNote->parameterW=pciNote->IndexHuming->parameterW;
	IndexHumingLocalNote->points=pciNote->IndexHuming->points;
	IndexHumingLocalNote->pointsArraySize=pciNote->IndexHuming->pointsArraySize;
	FAILIF(NULL == (IndexHumingLocalNote->pointULSHVectors = (Uns32T**)MALLOC(IndexHumingLocalNote->nHFTuples * sizeof(Uns32T*))));
	for(IntT i = 0; i < IndexHumingLocalNote->nHFTuples; i++){
		FAILIF(NULL == (IndexHumingLocalNote->pointULSHVectors[i] = (Uns32T*)MALLOC(IndexHumingLocalNote->hfTuplesLength * sizeof(Uns32T))));
	}
	FAILIF(NULL == (IndexHumingLocalNote->precomputedHashesOfULSHs = (Uns32T**)MALLOC(IndexHumingLocalNote->nHFTuples * sizeof(Uns32T*))));
	for(IntT i = 0; i < IndexHumingLocalNote->nHFTuples; i++){
		FAILIF(NULL == (IndexHumingLocalNote->precomputedHashesOfULSHs[i] = (Uns32T*)MALLOC(N_PRECOMPUTED_HASHES_NEEDED * sizeof(Uns32T))));
	}
	FAILIF(NULL == (IndexHumingLocalNote->reducedPoint = (float*)MALLOC(IndexHumingLocalNote->dimension * sizeof(float))));
	IndexHumingLocalNote->reportingResult=pciNote->IndexHuming->reportingResult;

	IndexHumingLocalNote->useUfunctions=pciNote->IndexHuming->useUfunctions;

	return 1;
}


#include<windows.h>
#include<tlhelp32.h>
#include<iostream>
using namespace std;

int CreateNewProcess(LPCSTR pszExeName,PROCESS_INFORMATION &piProcInfoGPS)
{
	STARTUPINFO siStartupInfo;
	SECURITY_ATTRIBUTES saProcess, saThread;
	ZeroMemory( &siStartupInfo, sizeof(siStartupInfo) );
	siStartupInfo.cb = sizeof(siStartupInfo);
	saProcess.nLength = sizeof(saProcess);
	saProcess.lpSecurityDescriptor = NULL;
	saProcess.bInheritHandle = true;
	saThread.nLength = sizeof(saThread);
	saThread.lpSecurityDescriptor = NULL;
	saThread.bInheritHandle = true;
	return ::CreateProcess( NULL, (LPTSTR)pszExeName, &saProcess,
		&saThread, false,
		CREATE_NO_WINDOW, NULL, NULL,
		&siStartupInfo, &piProcInfoGPS );
}


void main()
{
	ParamInfo param[2];		//������Ϣ
	map <string ,string> songIDAndName;		//����ID�͸���ӳ���
	PRNearNeighborStructT IndexHumingLocal;	//����
	PRNearNeighborStructT IndexHumingLocalNote;	//��������������
	vector<string> songFive;	//�洢���صļ������

	//������Ϊ�������������Ϊ5000newpv355.txt �е�pv�ļ��������������ļ�����./5355P�ļ����У�
	indexRead( param,songIDAndName,IndexHumingLocal, IndexHumingLocalNote);
	int i;

	string fileName("query.txt");	//�ļ�ÿ����һ���߳�wav��·��������ѯ��
	ifstream pitchFile(fileName.c_str());

	string OutName("result.txt");	//��Ž�����ļ�
	ofstream resultFile(OutName.c_str());

	string pitchname;	//wav·��
	while(getline(pitchFile,pitchname))	//��ȡÿ�е�wav·��
	{
		char filename[300];		//�߳�wav�ļ�������·����
		pParamInfo pci = param;	//֡���������������Ĳ�����Ϣ
		pParamInfo pciNote = param+1; 

		strcpy(filename,pitchname.c_str());
		//��ѯ
		WavToSongFive (filename,
			param->indexSongName,
			IndexHumingLocal,
			IndexHumingLocalNote,
			pci->IndexLSH,
			pciNote->IndexLSHNote,
			pci->stepFactor,
			pci->RetainNum,
			pciNote->RetainNum,
			pci->LSHFilterNum,
			pciNote->LSHFilterNum,
			pci->stepRatio,
			songFive);
		
		//������
		resultFile<<"query:"<<pitchname<<endl;
		for (int j=0;j<5 && j<=songFive.size();j++)
		{
			resultFile<<songFive[j]<<endl;
		}
		resultFile<<endl;
	}
	pitchFile.close();
	resultFile.close();
	//return 1;
}