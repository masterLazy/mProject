#include <iostream>
#include <iomanip>
#include "../mLib/mCode.h"
#include "../mLib/mFunction.h"
#include "../mLib/mFile.h"
using namespace std;
using namespace mlib;

struct Data
{
	long begin;
	long end;
	string name;
};
const long npos = -1;

const long min_size = 64;	//Ѱ�ҵ����ļ���С�Ĵ�С
const long find_range = 64;	//Ѱ���ļ��������Χ

int main()
{
	ios::sync_with_stdio(false);
	cout << "Base64 Encoder-decoder (v2.0) by mLazy" << endl << endl;
	cout << "Commands: " << endl;
	cout << "en [string]\tEncode string." << endl;
	cout << "de [string]\tDecode string." << endl;
	cout << "enf [filename]\tEncode file." << endl;
	cout << "def [filename]\tDecode file" << endl;
	cout << "scan [filename]\tScan file, then decode the files encode in base64 in it." << endl;
	cout << "exit\t\tExit." << endl << endl;

	string cmd, par, temp, filename;
	File f1, f2;
	bool res, success;
	char buf[1024] = { 0 };
	DWORD readSize;
	clock_t t0 = 0;
	float speed;
	long size, eta;
	int h, m, s;
	size_t decodeSize;
	//�ַ�������
	string cmds[] = { "en","de","enf","def","scan","exit" };
	while (true)
	{
		cout << "> ";
		cin >> cmd;

		if (cmd == "exit")break;

		cout << "< ";
		//�����ַ���
		if (cmd == "en")
		{
			cin >> par;
			cout << Base64_encode(par, par.size()) << endl;
		}
		//�����ַ���
		else if (cmd == "de")
		{
			cin >> par;
			temp = Base64_decode(par);
			if (!par.empty() && temp.empty())
			{
				cout << "Failed to decode. Please check the input." << endl;
			}
			else cout << temp << endl;
		}
		//�����ļ�
		else if (cmd == "enf")
		{
			getline(cin, par);
			par = par.substr(1);//ȥ���ո�
			filename = par + "_en.txt";
			//���ļ�
			res = true;
			res &= f1.open(par, "rb");
			if (res)res &= f2.open(filename, "wt");
			if (!res)
			{
				cout << "Failed to open file." << endl;
				f1.close();
				f2.delete_self();
				continue;
			}
			size = f1.get_size();
			//��ʼ����
			cout << "Encoding. . .";
			t0 = clock();
			while (true)
			{
				//��ȡ
				readSize = f1.read(buf, 1023, sizeof(char), 1023);//����1024����ΪҪȡ3�ı���

				//�����д��
				temp = Base64_encode(string(buf, readSize), readSize);
				f2.write(temp);

				//��ʾ״̬
				if (f1.get_cur() % (1024 * 1024 - 1) == 0)
				{
					cout << "\r< Encoding. . .";
					printPB((float)f1.get_cur() / size);

					//��ʱ��Ԥ��
					speed = float(1024 * 1024 - 1) / (clock() - t0) * 1000;
					cout << " ";
					printSize(speed);
					cout << "/s";

					eta = (size - f1.get_cur()) / speed;
					h = eta / 60 / 60;
					m = eta / 60 % 60;
					s = eta % 60;
					cout << "\tETA: " << h << ":" << ((m < 10) ? "0" : "") << m << ":" << ((s < 10) ? "0" : "") << s << " ";
					t0 = clock();
				}

				if (readSize < 3 || f1.get_cur() == f1.get_size())break;
			}
			//�ر��ļ�
			f1.close();
			f2.close();

			cout << endl;
			cout << "Encoding completed! Save at \"" << filename << "\"." << endl;
		}
		//�����ļ�
		else if (cmd == "def")
		{
			getline(cin, par);
			par = par.substr(1);//ȥ���ո�
			filename = par + "_de";
			//���ļ�
			res = true;
			res &= f1.open(par, "rt");
			if (res)res &= f2.open(filename, "wb");
			if (!res)
			{
				cout << "Failed to open file." << endl;
				f1.close();
				f2.delete_self();
				continue;
			}
			size = f1.get_size();
			//��ʼ����
			cout << "Decoding. . .";
			t0 = clock();
			while (true)
			{
				//��ȡ
				readSize = f1.read(buf, 1024, sizeof(char), 1024);//Ҫȡ4�ı���

				//�����д��
				temp = Base64_decode(string(buf, readSize), &decodeSize, &success);
				if (!success)
				{
					cout << endl << "Failed to decode. Please check the input file.";
					f1.close();
					f2.delete_self();
					break;
				}
				f2.write((void*)temp.c_str(), sizeof(char), decodeSize);

				//��ʾ״̬
				if (f1.get_cur() % (1024 * 1024) == 0)
				{
					cout << "\r< Decoding. . .";
					printPB((float)f1.get_cur() / size);

					//��ʱ��Ԥ��
					speed = float(1024 * 1024 - 1) / (clock() - t0) * 1000;
					cout << " ";
					printSize(speed);
					cout << "/s";

					eta = (size - f1.get_cur()) / speed;
					h = eta / 60 / 60;
					m = eta / 60 % 60;
					s = eta % 60;
					cout << "\tETA: " << h << ":" << ((m < 10) ? "0" : "") << m << ":" << ((s < 10) ? "0" : "") << s << " ";
					t0 = clock();
				}

				if (readSize < 3 || f1.get_cur() == f1.get_size())break;
			}
			//�ر��ļ�
			f1.close();
			f2.close();

			cout << endl;
			if (success)
			{
				cout << "Decoding completed! Save at \"" << filename << "\"." << endl;
			}
		}
		//ɨ���ļ�
		else if (cmd == "scan")
		{
			getline(cin, par);
			par = par.substr(1);//ȥ���ո�
			filename = par + "_de";
			//���ļ�
			res = f1.open(par, "rt");
			if (!res)
			{
				cout << "Failed to open file." << endl;
				f1.close();
				continue;
			}
			size = f1.get_size();

			//��ʼɨ��
			vector<Data> v;
			Data dTemp = { npos,npos };
			long last = 0;
			cout << "Scaning. . ." << endl;
			while (f1.get_cur() != f1.get_size())
			{
				//��ȡ
				readSize = f1.read(buf, 4, sizeof(char), 4);

				//���Խ���
				temp = Base64_decode(string(buf, readSize), &decodeSize, &success);
				if (success && f1.get_cur() != f1.get_size())
				{
					if (dTemp.begin == npos)
					{
						dTemp.begin = f1.get_cur() - 4;
					}
				}
				else
				{
					if (dTemp.begin != npos)
					{
						//��ǰ�ļ�������
						dTemp.end = f1.get_cur() - 5;
						if (dTemp.begin != dTemp.end &&
							//(dTemp.end - dTemp.begin + 1) % 4 == 0 &&
							dTemp.end - dTemp.begin + 1 >= min_size)
						{
							//��ѧ�����Դ���ѧbug...
							if ((dTemp.end - dTemp.begin + 1) % 4 == 1)
							{
								cout << "Doing wonder operations . . . ";
								dTemp.end--;
							}

							//����Ѱ�����ļ�������(�ļ���ͷ-1)��ʼ��ǰ�ҵ�˫���ţ�
							long now = f1.get_cur();
							long cnt = 0, begin = npos, end = npos;
							char ch;
							f1.set_cur(dTemp.begin - 1, SEEK_SET);
							for (long i = 0; i < find_range; i++)
							{
								f1.read(&ch, 1, 1, 1);
								//�ж�
								if (ch == '\'' || ch == '\"')
								{
									if (cnt == 1)
									{
										end = f1.get_cur() - 1;
									}
									else if (cnt == 2)
									{
										begin = f1.get_cur() - 1;
										break;
									}
									cnt++;
								}
								//ǰ��1���ֽڣ�����ڵ�ǰ��2���ֽڣ�
								f1.set_cur(-2, SEEK_CUR);
							}
							if (begin != npos && end != npos)
							{
								while (f1.get_cur() < end)
								{
									f1.read(&ch, 1, 1, 1);
									dTemp.name += ch;
								}
							}
							else
							{
								//���ļ���
								if (dTemp.name == "")
								{
									char buf[16];
									sprintf_s(buf, "data_%d", v.size() + 1);
									dTemp.name = buf;
								}
							}
							f1.set_cur(now, SEEK_SET);
							//����vector
							v.push_back(dTemp);

							//��ӡ����
							cout << dTemp.name;

							cout << "\tBegin: ";
							printSize(dTemp.begin);
							cout << " Size: ";
							printSize(dTemp.end - dTemp.begin + 1);
							cout << "  " << endl;
						}
						else
						{
							//�Ƶ����(�ļ���ͷ+1)����
							f1.set_cur(dTemp.begin + 1, SEEK_SET);
						}
						//���dTemp
						dTemp.begin = npos;
						dTemp.end = npos;
						dTemp.name = "";
					}
					else if (f1.get_cur() != f1.get_size())
					{
						//ǰ��3���ֽ�
						f1.set_cur(-3, SEEK_CUR);
					}
				}
			}

			cout << endl << "Scan completed. Found " << v.size() << " files. ";
			if (v.size() == 0)
			{
				cout << endl;
				continue;
			}
			cout << "Do you want to decode them? [y/n]" << endl;
			cin >> temp;
			if (temp == "y" || temp == "Y")
			{
				for (int i = 0; i < v.size(); i++)
				{
					//���ļ�
					res = f2.open(v[i].name, "wb");
					if (!res)
					{
						cout << "Failed to open file." << endl;
						f2.close();
						continue;
					}
					//���׼��
					size = v[i].end - v[i].begin + 1;
					f1.set_cur(v[i].begin, SEEK_SET);
					//��ʼ����
					cout << "Decoding. . .";
					t0 = clock();
					while (f1.get_cur() < v[i].end)
					{
						//��ȡ
						long less = v[i].end - f1.get_cur() + 1;
						readSize = f1.read(buf, 1024, sizeof(char), 1024 < less ? 1024 : less);

						//�����д��
						temp = Base64_decode(string(buf, readSize), &decodeSize, &success);
						if (!success)
						{
							cout << "Failed to decode. Please check the input file." << endl;
							f2.delete_self();
							break;
						}
						f2.write((void*)temp.c_str(), sizeof(char), decodeSize);
						//f2.write(buf, 1, readSize);
					}

					//�ر��ļ�
					f2.close();

					if (success)
					{
						cout << "\rDecoding completed! Save at \"" << v[i].name << "\"." << endl;
					}
				}
			}
			else
			{
				cout << "Decode canceled." << endl;
			}
			//�ر��ļ�
			f1.close();
		}
		//��������
		else
		{
			cout << "\"" << cmd << "\" is not a command. ";
			int min = INT_MAX, minI, temp;
			for (int i = 0; i < sizeof(cmds) / sizeof(string); i++)
			{
				temp = levenshtein::compare(cmds[i], cmd);
				if (temp < min)
				{
					min = temp;
					minI = i;
				}
			}
			cout << "Did you mean \"" << cmds[minI] << "\"?" << endl;
		}
	}

	return 0;
}