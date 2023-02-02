#pragma once
/////////////////////////////////////////////////////
//												   //
//				    Network.h					   //
//												   //
//				     ������					   //
//												   //
/////////////////////////////////////////////////////
typedef float Net_F;

#include <cmath>
#include <corecrt_math_defines.h>
#include <vector>
#include <iostream>
#include <amp.h>
#include <amp_math.h>
using namespace concurrency::fast_math;
using namespace std;

/////////////////////////////////////////////////////
//												   //
//						��						   //
//												   //
/////////////////////////////////////////////////////

class NetLayer
{
private:
	int size;
	int fun;	//ʹ�õļ����

	//�����
	Net_F Map(Net_F x)
	{
		if (fun == 0)
		{
			//Sigmoid
			return 1 / (1 + pow(M_E, -x));
		}
		else if (fun == 1)
		{
			//Tanh
			return tanh(x);
		}
		else if (fun == 2)
		{
			//ReLU
			return max<Net_F>(0, x);
		}
		else if (fun == 3)
		{
			//Leaky_LU
			if (x <= 0)return 0.01 * x;
			else return x;
		}
	}
	//�䵼��
	Net_F dMap(Net_F x)
	{
		if (fun == 0)
		{
			//Sigmoid
			return Map(x) * (1 - Map(x));
		}
		else if (fun == 1)
		{
			//Tanh
			return 1 - pow(tanh(x), 2);
		}
		else if (fun == 2)
		{
			//ReLU
			if (x <= 0)return 0;
			else return 1;
		}
		else if (fun == 3)
		{
			//Leaky_ReLU
			if (x <= 0)return 0.01;
			else return 1;
		}
	}

public:
	NetLayer* parent;	//����(��һ��)
	vector<Net_F> a;	//����ֵ
	vector<Net_F> w;	//Ȩ��, w_j(����)k(�ϲ�)=w[j*parent.size+k]
	vector<Net_F> b;	//ƫ��

	vector<Net_F> yC;	//����ֵ �����ı�ֵ
	vector<Net_F> wC;	//Ȩ�� �ı�ֵ
	vector<Net_F> bC;	//ƫ�� �ı�ֵ

	//��ʼ��
	void Init(const int _size, int _fun = 0, NetLayer* const _parent = nullptr)
	{
		fun = _fun;
		size = _size;
		parent = _parent;

		a.resize(size);
		if (parent != nullptr)
		{
			w.resize(size * parent->size);
			wC.resize(size * parent->size);

			b.resize(size);
			bC.resize(size);
		}

		yC.resize(size);

		for (int i = 0; i < w.size(); i++)
		{
			w[i] = (Net_F)rand() / RAND_MAX * 2 - 1;
			wC[i] = 0;
		}
		for (int i = 0; i < b.size(); i++)
		{
			b[i] = (Net_F)rand() / RAND_MAX * 2 - 1;
			bC[i] = 0;
		}
	}

	//��ȡ���С(�ڵ���)
	int Size()
	{
		return size;
	}

	//���㼤��ֵ
	bool Work()
	{
		if (parent == nullptr)
		{
			return false;
		}
		for (int i = 0; i < size; i++)
		{
			a[i] = 0;
			for (int j = 0; j < parent->size; j++)
			{
				a[i] += parent->a[j] * w[i * parent->size + j];
			}
			a[i] = Map(a[i] + b[i]);
		}
		return true;
	}

	//ѧϰ�µ�ѵ������(���򴫲�)
	void NewTrain(vector<Net_F> y)
	{
		if (parent == nullptr)return;
		if (y.size() != size)return;

		for (int i = 0; i < size; i++)
		{
			yC[i] = y[i] - a[i];

			Net_F temp = dMap(Map(a[i])) * 2 * -yC[i];
			//cout << a[i] + yC[i] << endl;
			//cin.get();
			for (int j = 0; j < parent->size; j++)
			{
				wC[i * parent->size + j] -= parent->a[j] * temp;
				parent->yC[j] -= w[i * parent->size + j] * temp;
			}
			bC[i] -= temp;
		}
		parent->NewTrain();
	}
	void NewTrain()
	{
		if (parent == nullptr)return;

		for (int i = 0; i < size; i++)
		{
			Net_F temp = dMap(Map(a[i])) * 2 * -yC[i];
			//cout << a[i] + yC[i] << endl;
			//cin.get();
			for (int j = 0; j < parent->size; j++)
			{
				wC[i * parent->size + j] -= parent->a[j] * temp;
				parent->yC[j] -= w[i * parent->size + j] * temp;
			}
			bC[i] -= temp;
		}
		parent->NewTrain();
	}
	//Ӧ��ѵ������(����ݶ��½�)
	bool EndTrain(int times, Net_F learningRate = 0.01)
	{
		if (parent == nullptr)return true;
		if (times == 0)return false;

		yC.clear();
		yC.resize(size);
		for (int i = 0; i < w.size(); i++)
		{
			w[i] += wC[i] / times * learningRate;
			wC[i] = 0;
		}
		for (int i = 0; i < b.size(); i++)
		{
			b[i] += bC[i] / times * learningRate;
			bC[i] = 0;
		}
		return parent->EndTrain(times, learningRate);
	}
};

/////////////////////////////////////////////////////
//												   //
//				     ������					   //
//												   //
/////////////////////////////////////////////////////

#ifdef GPU
Net_F Map(Net_F x, int fun) restrict(amp)
{
	if (fun == 0)
	{
		//Sigmoid
		return 1 / (1 + pow(M_E, -x));
	}
	else if (fun == 1)
	{
		//Tanh
		return tanh(x);
	}
	else if (fun == 2)
	{
		//ReLU
		if (x <= 0)return 0;
		else return x;
	}
	else if (fun == 3)
	{
		//Leaky_LU
		if (x <= 0)return 0.01 * x;
		else return x;
	}
}
Net_F dMap(Net_F x, int fun) restrict(amp)
{
	if (fun == 0)
	{
		//Sigmoid
		return Map(x, fun) * (1 - Map(x, fun));
	}
	else if (fun == 1)
	{
		//Tanh
		return 1 - pow(tanh(x), 2);
	}
	else if (fun == 2)
	{
		//ReLU
		if (x <= 0)return 0;
		else return 1;
	}
	else if (fun == 3)
	{
		//Leaky_ReLU
		if (x <= 0)return 0.01;
		else return 1;
	}
}
void Train(vector<NetLayer>& layer, vector<Net_F>& output, int fun)
{
	using namespace concurrency;

	array_view<Net_F, 1> gpu_y(layer[layer.size() - 1].Size(), output);
	for (int i = layer.size() - 1; i > 0; i--)
	{
		array_view<Net_F, 1> temp(layer[i].Size());
		//ƫ��
		array_view<Net_F, 1> gpu_a(layer[i].Size(), layer[i].a);
		array_view<Net_F, 1> gpu_bC(layer[i].Size(), layer[i].bC);
		parallel_for_each(
			gpu_a.extent,
			[=](index<1> idx) restrict(amp)
			{
				temp[idx] = dMap(Map(gpu_a[idx], fun), fun) * 2 * (gpu_a[idx] - gpu_y[idx]);
				gpu_bC[idx] -= temp[idx];
			}
		);
		gpu_bC.synchronize();

		gpu_y.synchronize();
		for (int i = 0; i < layer[i].Size(); i++)
		{
			std::cout << gpu_y[index<1>{i}] << std::endl;
		}
		std::cin.get();

		//Ȩ��
		array_view<Net_F, 1> gpu_pa(layer[i - 1].Size(), layer[i - 1].a);
		array_view<Net_F, 2> gpu_wC(layer[i].Size(), layer[i - 1].Size(),layer[i].wC);
		parallel_for_each(
			gpu_wC.extent,
			[=](index<2> idx) restrict(amp)
			{
				gpu_wC[idx] -= gpu_pa[idx[1]] * temp[idx[0]];
			}
		);
		gpu_wC.synchronize();

		//��һ��(y)
		array_view<Net_F, 2> gpu_w(layer[i].Size(), layer[i - 1].Size(),layer[i].w);
		array_view<Net_F, 1> gpu_par_a(layer[i - 1].Size(), layer[i - 1].a);
		array_view<Net_F, 1> y_temp(layer[i - 1].Size());
		gpu_y = y_temp;
		parallel_for_each(
			gpu_w.extent,
			[=](index<2> idx) restrict(amp)
			{
				if (idx[0] == 0)gpu_y[idx[1]] = 0;
				gpu_y[idx[1]] += gpu_par_a[idx[1]] - gpu_w[idx] * temp[idx[0]];
			}
		);
	}
}
#endif

class Network
{
private:
	int fun;
	
public:
	vector<NetLayer> layer;
	int times;		//ѵ������
	Net_F times_h;	//ѵ������(�ۼ�)

	//��ʼ��(sizes��ʾ����Ľڵ���)
	/*fun
	* 0	Sigmoid
	* 1	Tanh
	* 2 ReLU
	* 3 LReLU
	*/
	void Init(vector<int> size, int _fun = 0)
	{
		fun = _fun;
		layer.resize(size.size());
		for (int i = 0; i < size.size(); i++)
		{
			layer[i].Init(size[i], _fun, i > 0 ? &layer[i - 1] : nullptr);
		}
	}

	//��ȡ���
	bool Work(vector<Net_F> input, vector<Net_F>* output = nullptr)
	{
		if (input.size() != layer[0].Size())
		{
			return false;
		}
		layer[0].a = input;
		for (int i = 0; i < layer.size(); i++)
		{
			layer[i].Work();
		}

		if (output != nullptr)
		{
			output->resize(layer[layer.size() - 1].Size());
			for (int i = 0; i < layer[layer.size() - 1].Size(); i++)
			{
				(*output)[i] = layer[layer.size() - 1].a[i];
			}
		}
		return true;
	}

	//ѧϰ�µ�ѵ������
	bool NewTrain(vector<Net_F> output)
	{
#ifndef GPU
		if (output.size() != layer[layer.size() - 1].Size())
		{
			return false;
		}
		layer[layer.size() - 1].NewTrain(output);
#else
		Train(layer, output, fun);
#endif
		times++;
		times_h++;
		return true;
	}
	//Ӧ��ѵ������
	bool EndTrain(Net_F learningRate = 0.01)
	{
		bool res = layer[layer.size() - 1].EndTrain(times, learningRate);
		times = 0;
		return res;
	}

	//���浽�ļ�
	bool Save(const char* filename)
	{
		FILE* f;
		fopen_s(&f, filename, "w");
		if (f == NULL)return false;
		
		//��������
		fprintf_s(f, "Data_type: Network\n");
		//ʹ�õļ����
		fprintf_s(f, "Activation_function: ");
		if (fun == 0)fprintf_s(f, "Sigmoid\n");
		else if (fun == 1)fprintf_s(f, "Tanh\n");
		else if (fun == 2)fprintf_s(f, "ReLU\n");
		else if (fun == 3)fprintf_s(f, "LReLU\n");
		//����
		fprintf_s(f, "Layers: %d\n", layer.size());
		//ѵ������
		fprintf_s(f, "Training_times: %g\n", times_h);
		
		//������
		for (int i = 0; i < layer.size(); i++)
		{
			fprintf_s(f, "\nLayer: %d\n", i);
			//�ڵ���
			fprintf_s(f, "Nodes: %d\n", layer[i].Size());
			//����ֵ
			for (int j = 0; j < layer[i].Size(); j++)
			{
				fprintf_s(f, "a%d: %g\n", j, layer[i].a[j]);
			}
			//Ȩ��
			if (i > 0)
			{
				for (int j = 0; j < layer[i].Size(); j++)
				{
					for (int k = 0; k < layer[i - 1].Size(); k++)
					{
						fprintf_s(f, "w%d_%d: %g\n", j, k, layer[i].w[j * layer[i - 1].Size() + k]);
					}
				}
				//ƫ��
				for (int j = 0; j < layer[i].Size(); j++)
				{
					fprintf_s(f, "b%d: %g\n", j, layer[i].b[j]);
				}
			}
		}
		fclose(f);
		return true;
	}
	//���ļ���ȡ
	bool Load(const char* filename)
	{
		FILE* f;
		fopen_s(&f, filename, "r");
		if (f == NULL)return false;

		char temp[100];
		int size;

		//��������
		fscanf_s(f, "%s", temp, sizeof(temp));
		fscanf_s(f, "%s", temp, sizeof(temp));
		if (strcmp(temp, "Network") != 0)return false;
		//ʹ�õļ����
		fscanf_s(f, "%s", temp, sizeof(temp));
		fscanf_s(f, "%s", temp, sizeof(temp));
		if (strcmp(temp, "Sigmoid") == 0)fun = 0;
		else if (strcmp(temp, "Tanh") == 0)fun = 1;
		else if (strcmp(temp, "ReLU") == 0)fun = 2;
		else if (strcmp(temp, "LReLU") == 0)fun = 3;
		//����
		fscanf_s(f, "%s %d", temp, sizeof(temp), &size);
		layer.resize(size);
		//ѵ������
		fscanf_s(f, "%s %g", temp, sizeof(temp), &times_h);
		//������
		for (int i = 0; i < layer.size(); i++)
		{
			fscanf_s(f, "%s %s", temp, sizeof(temp), temp, sizeof(temp));
			//�ڵ���
			fscanf_s(f, "%s %d", temp, sizeof(temp), &size);
			layer[i].Init(size, fun, i > 0 ? &layer[i - 1] : nullptr);
			//����ֵ
			for (int j = 0; j < layer[i].Size(); j++)
			{
				fscanf_s(f, "%s", temp, sizeof(temp));
				fscanf_s(f, "%g", &layer[i].a[j]);
			}
			//Ȩ��
			if (i > 0)
			{
				for (int j = 0; j < layer[i].Size(); j++)
				{
					for (int k = 0; k < layer[i - 1].Size(); k++)
					{
						fscanf_s(f, "%s", temp, sizeof(temp));
						fscanf_s(f, "%g", &layer[i].w[j * layer[i - 1].Size() + k]);
					}
				}
				//ƫ��
				for (int j = 0; j < layer[i].Size(); j++)
				{
					fscanf_s(f, "%s", temp, sizeof(temp));
					fscanf_s(f, "%g", &layer[i].b[j]);
				}
			}
		}
		fclose(f);
		return true;
	}
};