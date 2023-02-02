#pragma once
/////////////////////////////////////////////////////
//												   //
//				    Network.h					   //
//												   //
//				     神经网络					   //
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
//						层						   //
//												   //
/////////////////////////////////////////////////////

class NetLayer
{
private:
	int size;
	int fun;	//使用的激活函数

	//激活函数
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
	//其导数
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
	NetLayer* parent;	//父层(上一层)
	vector<Net_F> a;	//激活值
	vector<Net_F> w;	//权重, w_j(本层)k(上层)=w[j*parent.size+k]
	vector<Net_F> b;	//偏置

	vector<Net_F> yC;	//激活值 期望改变值
	vector<Net_F> wC;	//权重 改变值
	vector<Net_F> bC;	//偏置 改变值

	//初始化
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

	//获取层大小(节点数)
	int Size()
	{
		return size;
	}

	//计算激活值
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

	//学习新的训练样本(反向传播)
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
	//应用训练样本(随机梯度下降)
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
//				     神经网络					   //
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
		//偏置
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

		//权重
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

		//上一层(y)
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
	int times;		//训练次数
	Net_F times_h;	//训练次数(累计)

	//初始化(sizes表示各层的节点数)
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

	//获取输出
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

	//学习新的训练样本
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
	//应用训练样本
	bool EndTrain(Net_F learningRate = 0.01)
	{
		bool res = layer[layer.size() - 1].EndTrain(times, learningRate);
		times = 0;
		return res;
	}

	//保存到文件
	bool Save(const char* filename)
	{
		FILE* f;
		fopen_s(&f, filename, "w");
		if (f == NULL)return false;
		
		//数据类型
		fprintf_s(f, "Data_type: Network\n");
		//使用的激活函数
		fprintf_s(f, "Activation_function: ");
		if (fun == 0)fprintf_s(f, "Sigmoid\n");
		else if (fun == 1)fprintf_s(f, "Tanh\n");
		else if (fun == 2)fprintf_s(f, "ReLU\n");
		else if (fun == 3)fprintf_s(f, "LReLU\n");
		//层数
		fprintf_s(f, "Layers: %d\n", layer.size());
		//训练次数
		fprintf_s(f, "Training_times: %g\n", times_h);
		
		//各个层
		for (int i = 0; i < layer.size(); i++)
		{
			fprintf_s(f, "\nLayer: %d\n", i);
			//节点数
			fprintf_s(f, "Nodes: %d\n", layer[i].Size());
			//激活值
			for (int j = 0; j < layer[i].Size(); j++)
			{
				fprintf_s(f, "a%d: %g\n", j, layer[i].a[j]);
			}
			//权重
			if (i > 0)
			{
				for (int j = 0; j < layer[i].Size(); j++)
				{
					for (int k = 0; k < layer[i - 1].Size(); k++)
					{
						fprintf_s(f, "w%d_%d: %g\n", j, k, layer[i].w[j * layer[i - 1].Size() + k]);
					}
				}
				//偏置
				for (int j = 0; j < layer[i].Size(); j++)
				{
					fprintf_s(f, "b%d: %g\n", j, layer[i].b[j]);
				}
			}
		}
		fclose(f);
		return true;
	}
	//从文件读取
	bool Load(const char* filename)
	{
		FILE* f;
		fopen_s(&f, filename, "r");
		if (f == NULL)return false;

		char temp[100];
		int size;

		//数据类型
		fscanf_s(f, "%s", temp, sizeof(temp));
		fscanf_s(f, "%s", temp, sizeof(temp));
		if (strcmp(temp, "Network") != 0)return false;
		//使用的激活函数
		fscanf_s(f, "%s", temp, sizeof(temp));
		fscanf_s(f, "%s", temp, sizeof(temp));
		if (strcmp(temp, "Sigmoid") == 0)fun = 0;
		else if (strcmp(temp, "Tanh") == 0)fun = 1;
		else if (strcmp(temp, "ReLU") == 0)fun = 2;
		else if (strcmp(temp, "LReLU") == 0)fun = 3;
		//层数
		fscanf_s(f, "%s %d", temp, sizeof(temp), &size);
		layer.resize(size);
		//训练次数
		fscanf_s(f, "%s %g", temp, sizeof(temp), &times_h);
		//各个层
		for (int i = 0; i < layer.size(); i++)
		{
			fscanf_s(f, "%s %s", temp, sizeof(temp), temp, sizeof(temp));
			//节点数
			fscanf_s(f, "%s %d", temp, sizeof(temp), &size);
			layer[i].Init(size, fun, i > 0 ? &layer[i - 1] : nullptr);
			//激活值
			for (int j = 0; j < layer[i].Size(); j++)
			{
				fscanf_s(f, "%s", temp, sizeof(temp));
				fscanf_s(f, "%g", &layer[i].a[j]);
			}
			//权重
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
				//偏置
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