/*
* 项目属性 > 配置属性 > C/C++ > 常规 > 调试信息格式 = 无
*/
//#include "../mLib/mWeb.h"	//把它放在开头，不然有100+bug
#include <Windows.h>
#include <windowsx.h>
//using namespace std
//神奇代码，能让控件风格跟随系统
#pragma comment(linker,"/manifestdependency:\"type='win32' \
						name='Microsoft.Windows.Common-Controls' \
						version='6.0.0.0' \
						processorArchitecture='*' \
						publicKeyToken='6595b64144ccf1df' \
						language='*'\"")

#include "../mLib/mGraphics.h"
#include "../mLib/mFunction.h"
#include "../mLib/mFile.h"
using namespace mlib;

//C++ AMP
#include <amp.h>
#include <amp_math.h>
using namespace concurrency;
/*
* fast_math		单精度
* precise_math	双精度
*/
//using namespace precise_math;

const int cxWindow = 1920, cyWindow = 1080;

//屏幕中央位置(因为常用，所以特别摆出来)
float mX = (float)cxWindow / 2;
float mY = (float)cyWindow / 2;

HINSTANCE m_hInstance;
Graphics gfx;
Image img;

#define GPU_RGB(r,g,b) ((COLORREF)(r) + (COLORREF)(g) * 0x100 + (COLORREF)(b) * 0x10000)
//色彩映射
int style = 1;//映射样式
/*style
* 0		线性，黑-白
* 1		循环，-[蓝-白-橙]-
*/
inline COLORREF CChange(int r0, int g0, int b0, int r1, int g1, int b1, double r) restrict(amp)
{
	return GPU_RGB(
		(1 - r) * r0 + r * r1,
		(1 - r) * g0 + r * g1,
		(1 - r) * b0 + r * b1
	);
}
inline COLORREF CMap(int style, int t, int n) restrict(amp)
{
	switch (style)
	{
	case 0:
		return CChange(0, 0, 0, 255, 255, 255, (float)t / n);
	case 1:
	{
		float x = float(t % 100) / 100;
		if (x < 1.0 / 3)
		{
			x = x * 3;
			return CChange(17, 40, 126, 255, 255, 255, x);
		}
		else if (x < 2.0 / 3)
		{
			x = (x - 1.0 / 3) * 3;
			return CChange(255, 255, 255, 233, 132, 10, x);
		}
		else
		{
			x = (x - 2.0 / 3) * 3;
			return CChange(233, 132, 10, 17, 40, 126, x);
		}
	}
	}
}

//渲染(到COLORREF)
enum RenderMode { FAST, PRECISE };
RenderMode mode = FAST;//渲染模式，快速/准确
double cX = -0.5;		//图像中心x坐标(实轴)
double cY = 0;			//图像中心y坐标(虚轴)
double cX0, cY0;
double zoom = 500;		//像素每单位长度(px/单位长度)
int iterN = 100;		//迭代次数
const int rW = cxWindow;//渲染输出宽度
const int rH = cyWindow;//渲染输出高度
COLORREF rRes[rW * rH];	//渲染结果
//快速渲染
void FastRender()
{
	using namespace fast_math;
	typedef float Flt;

	//变量局部化
	Flt cx = cX, cy = cY, _zoom = zoom;
	int itern = iterN, _style = style, rw = rW, rh = rH;

	//单位长度宽和高
	Flt w = rW / zoom;
	Flt h = rH / zoom;
	//左上角所对应的复数值
	Flt lt_r = cx - w / 2;
	Flt lt_i = cy - h / 2;

	//准备
	array_view <COLORREF, 1> gpu_res(rW * rH, rRes);
	gpu_res.discard_data();//放弃同步到 GPU

	//GPU 渲染，启动！
	parallel_for_each(
		gpu_res.extent,
		[=](index<1> idx) restrict(amp)
		{
			int x = idx[0] / rh, y = idx[0] % rh;
			if (true)
			{
				//迭代子 z
				Flt z0_r = 0, z0_i = 0;
				Flt z1_r = 0, z1_i = 0;

				//本像素所对的 c 值
				Flt c_r = (Flt)x / rw * w + lt_r;
				Flt c_i = (1 - (Flt)y / rh) * h + lt_i;//不反转虚轴

				//迭代
				int j;
				for (j = 0; j < itern; j++)
				{
					//记录上次迭代结果
					z0_r = z1_r;
					z0_i = z1_i;

					//计算新的迭代结果
					z1_r = pow(z0_r, 2) - pow(z0_i, 2);
					z1_i = 2.0 * z0_r * z0_i;

					z1_r += c_r;
					z1_i += c_i;

					//检测是否要炸到无穷大了
					if (fabs(z1_r) >= 2.0 || fabs(z1_i) >= 2.0)
					{
						break;
					}
				}

				//计算颜色
				if (j == itern)
				{
					gpu_res[idx] = GPU_RGB(0, 0, 0);
				}
				else
				{
					gpu_res[idx] = CMap(_style, j, itern);
				}
			}
		}
	);

	//把数据同步回来
	gpu_res.synchronize();
}
//精确渲染
void PrecisRender()
{
	using namespace precise_math;
	typedef double Flt;

	//变量局部化
	Flt cx = cX, cy = cY, _zoom = zoom;
	int itern = iterN, _style = style, rw = rW, rh = rH;

	//单位长度宽和高
	Flt w = rW / zoom;
	Flt h = rH / zoom;
	//左上角所对应的复数值
	Flt lt_r = cx - w / 2;
	Flt lt_i = cy - h / 2;

	//准备
	array_view <COLORREF, 1> gpu_res(rW * rH, rRes);
	gpu_res.discard_data();//放弃同步到 GPU

	//GPU 渲染，启动！
	parallel_for_each(
		gpu_res.extent,
		[=](index<1> idx) restrict(amp)
		{
			int x = idx[0] / rh, y = idx[0] % rh;
			if (true)
			{
				//迭代子 z
				Flt z0_r = 0, z0_i = 0;
				Flt z1_r = 0, z1_i = 0;

				//本像素所对的 c 值
				Flt c_r = (Flt)x / rw * w + lt_r;
				Flt c_i = (1 - (Flt)y / rh) * h + lt_i;//不反转虚轴

				//迭代
				int j;
				for (j = 0; j < itern; j++)
				{
					//记录上次迭代结果
					z0_r = z1_r;
					z0_i = z1_i;

					//计算新的迭代结果
					z1_r = pow(z0_r, 2) - pow(z0_i, 2);
					z1_i = 2.0 * z0_r * z0_i;

					z1_r += c_r;
					z1_i += c_i;

					//检测是否要炸到无穷大了
					if (fabs(z1_r) >= 2.0 || fabs(z1_i) >= 2.0)
					{
						break;
					}
				}

				//计算颜色
				if (j == itern)
				{
					gpu_res[idx] = GPU_RGB(0, 0, 0);
				}
				else
				{
					gpu_res[idx] = CMap(_style, j, itern);
				}
			}
		}
	);

	//把数据同步回来
	gpu_res.synchronize();
}
//渲染
void Render()
{
	if (mode == FAST)
	{
		FastRender();
	}
	else
	{
		PrecisRender();
	}
}

//绘制(所有)
bool inited = false;	//可以开始绘制
bool needRender = true;	//需要渲染
COLORREF colHUD = RGB(153, 217, 234);	//HUD 的颜色
/*中心指示器*/
bool bCursor = true;
const int cwCursor = 100;	//中心指示器的宽高
/*图例*/
bool bLegend = true;
const int minScale = 80;	//比例尺宽度的阈值下限
const int maxScale = 100;	//比例尺宽度的阈值上限
/*设置栏*/
bool bSet = true;			//是否显示设置栏
const int cxSet = 500;		//设置栏的宽
const int cySet = 200;		//设置栏的高
//面板内侧的四个角
int setL = mX - cxSet / 2 + 2;
int setT = cyWindow - cySet + 2;
int setR = mX + cxSet / 2 - 2;
int setB = cyWindow;

//绘制
//bool needDraw = true;
void Draw()
{
	//needDraw = false;
	if (needRender)
	{
		Render();
		needRender = false;
	}
	img.load_from_mem(rRes, rW, rH);
	gfx.draw_image(img, 0, 0, cxWindow, cyWindow);
	//字体等
	char szText[64];
	Font font;
	font.szFontName = L"JetBrains Mono";
	font.weight = FontWeight::LIGHT;
	font.size = 20;

	/*中心指示器*/
	if (bCursor)
	{
		gfx.draw_line(mX, mY - cwCursor / 2, mX, mY + cwCursor / 2, gfx.brush(colHUD, 0.8), 2);
		gfx.draw_line(mX - cwCursor / 2, mY, mX + cwCursor / 2, mY, gfx.brush(colHUD, 0.8), 2);
	}
	/*图例*/
	if (bLegend)
	{
		//比例尺
		double num = 1;				//单位长度数
		double width = num * zoom;	//比例尺的宽
		while (width > maxScale)
		{
			num /= 10;
			width = num * zoom;
		}
		while (width < minScale)
		{
			num *= 2;
			width = num * zoom;
		}
		gfx.draw_line(100, cyWindow - 110, 100 + width, cyWindow - 110, gfx.brush(colHUD), 2);
		gfx.draw_line(100, cyWindow - 110, 100, cyWindow - 120, gfx.brush(colHUD), 2);
		gfx.draw_line(100 + width, cyWindow - 110, 100 + width, cyWindow - 120, gfx.brush(colHUD), 2);
		//数字
		sprintf_s(szText, "%.1e", num);
		gfx.draw_text(100, cyWindow - 100, StrToWstr(szText).c_str(), gfx.brush(colHUD), font);

		//视野信息
		if (cY >= 0)sprintf_s(szText, "%f + %fi\nZoom:%.1e", cX, cY, zoom / 500);
		else sprintf_s(szText, "%f - %fi\nZoom:%.1e", cX, -cY, zoom / 500);
		gfx.draw_text(100, 100, StrToWstr(szText).c_str(), gfx.brush(colHUD), font);
	}
	/*设置栏*/
	if (bSet)
	{
		gfx.fill_rectangle(mX - cxSet / 2, cyWindow - cySet, mX + cxSet / 2, cyWindow + 4, gfx.brush(0, 0, 0, 0.8));
		gfx.draw_rectangle(mX - cxSet / 2, cyWindow - cySet, mX + cxSet / 2, cyWindow + 4, gfx.brush(colHUD, 0.8), 4);

		//渲染模式
		sprintf_s(szText, "渲染模式: %s", mode == FAST ? "快速" : "精确");
		if (gfx.m_x > setL && gfx.m_x < setR &&
			gfx.m_y > setT + 10 && gfx.m_y < setT + 10 * 2 + 20)
		{
			gfx.fill_rectangle(setL, setT + 10, setR, setT + 10 * 2 + 20, gfx.brush(colHUD, 0.8));
			gfx.draw_text(setL + 10, setT + 12.5, StrToWstr(szText).c_str(), gfx.brush(0, 0, 0), font);
		}
		else
		{
			gfx.draw_text(setL + 10, setT + 12.5, StrToWstr(szText).c_str(), gfx.brush(colHUD), font);
		}

		//迭代步数
		sprintf_s(szText, "迭代步数: %d", iterN);
		if (gfx.m_x > setL && gfx.m_x < setR &&
			gfx.m_y > setT + 10 * 2 + 20 && gfx.m_y < setT + 10 * 3 + 20 * 2)
		{
			gfx.fill_rectangle(setL, setT + 10 * 2 + 20, setR - 100, setT + 10 * 3 + 20 * 2, gfx.brush(colHUD, 0.8));
			gfx.draw_text(setL + 10, setT + 12.5 + 10 + 20, StrToWstr(szText).c_str(), gfx.brush(0, 0, 0), font);
			//-+号
			if (gfx.m_x >= setR - 100 && gfx.m_x < setR - 50)
			{
				gfx.fill_rectangle(setR - 100, setT + 10 * 2 + 20, setR - 50, setT + 10 * 3 + 20 * 2, gfx.brush(colHUD, 0.6));
				gfx.draw_text(setR - 100 + 20, setT + 12.5 + 10 + 20, L"-", gfx.brush(0, 0, 0), font);
			}
			else
			{
				gfx.draw_text(setR - 100 + 20, setT + 12.5 + 10 + 20, L"-", gfx.brush(colHUD), font);
			}
			if (gfx.m_x >= setR - 50)
			{
				gfx.fill_rectangle(setR - 50, setT + 10 * 2 + 20, setR, setT + 10 * 3 + 20 * 2, gfx.brush(colHUD, 0.6));
				gfx.draw_text(setR - 50 + 20, setT + 12.5 + 10 + 20, L"+", gfx.brush(0, 0, 0), font);
			}
			else
			{
				gfx.draw_text(setR - 50 + 20, setT + 12.5 + 10 + 20, L"+", gfx.brush(colHUD), font);
			}
		}
		else
		{
			gfx.draw_text(setL + 10, setT + 12.5 + 10 + 20, StrToWstr(szText).c_str(), gfx.brush(colHUD), font);
			gfx.draw_text(setR - 100 + 20, setT + 12.5 + 10 + 20, L"-", gfx.brush(colHUD), font);
			gfx.draw_text(setR - 50 + 20, setT + 12.5 + 10 + 20, L"+", gfx.brush(colHUD), font);
		}

		//图例
		sprintf_s(szText, "图例: %s", bLegend ? "显示" : "隐藏");
		if (gfx.m_x > setL && gfx.m_x < setR &&
			gfx.m_y > setT + 10 * 3 + 20 * 2 && gfx.m_y < setT + 10 * 4 + 20 * 3)
		{
			gfx.fill_rectangle(setL, setT + 10 * 3 + 20 * 2, setR, setT + 10 * 4 + 20 * 3, gfx.brush(colHUD, 0.8));
			gfx.draw_text(setL + 10, setT + 12.5 + 10 * 2 + 20 * 2, StrToWstr(szText).c_str(), gfx.brush(0, 0, 0), font);
		}
		else
		{
			gfx.draw_text(setL + 10, setT + 12.5 + 10 * 2 + 20 * 2, StrToWstr(szText).c_str(), gfx.brush(colHUD), font);
		}

		//重置
		sprintf_s(szText, "重置");
		if (gfx.m_x > setL && gfx.m_x < setR &&
			gfx.m_y > setT + 10 * 4 + 20 * 3 && gfx.m_y < setT + 10 * 5 + 20 * 4)
		{
			gfx.fill_rectangle(setL, setT + 10 * 4 + 20 * 3, setR, setT + 10 * 5 + 20 * 4, gfx.brush(colHUD, 0.8));
			gfx.draw_text(setL + 10, setT + 12.5 + 10 * 3 + 20 * 3, StrToWstr(szText).c_str(), gfx.brush(0, 0, 0), font);
		}
		else
		{
			gfx.draw_text(setL + 10, setT + 12.5 + 10 * 3 + 20 * 3, StrToWstr(szText).c_str(), gfx.brush(colHUD), font);
		}

		//保存到图片
		sprintf_s(szText, "保存到图片");
		if (gfx.m_x > setL && gfx.m_x < setR &&
			gfx.m_y > setT + 10 * 5 + 20 * 4 && gfx.m_y < setT + 10 * 6 + 20 * 5)
		{
			gfx.fill_rectangle(setL, setT + 10 * 5 + 20 * 4, setR, setT + 10 * 6 + 20 * 5, gfx.brush(colHUD, 0.8));
			gfx.draw_text(setL + 10, setT + 12.5 + 10 * 4 + 20 * 4, StrToWstr(szText).c_str(), gfx.brush(0, 0, 0), font);
		}
		else
		{
			gfx.draw_text(setL + 10, setT + 12.5 + 10 * 4 + 20 * 4, StrToWstr(szText).c_str(), gfx.brush(colHUD), font);
		}

		//提示
		sprintf_s(szText, "*按Esc退出,按S关闭/打开设置栏;版本:1.01*");
		gfx.draw_text(setL + 10, setT + 12.5 + 10 * 5 + 20 * 5, StrToWstr(szText).c_str(), gfx.brush(colHUD), font);
	}
}

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	m_hInstance = hInstance;//保存一下实例句柄

	wchar_t szAppName[] = L"探索复平面";
	HWND hWnd;
	MSG msg;
	WNDCLASS wndclass;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);//有控件时，要改成这样，更美观
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;

	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, L"窗口注册失败！", szAppName, MB_ICONERROR);
		return 0;
	}

	hWnd = CreateWindow(szAppName,
		szAppName,//窗口的标题
		WS_POPUP,
		CW_USEDEFAULT,//窗口的x坐标
		CW_USEDEFAULT,//窗口的y坐标
		cxWindow,  //窗口的宽
		cyWindow, //窗口的高
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);

	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			//绘制
			if (inited)
			{
				gfx.begin_draw();
				Draw();
				gfx.end_draw();
			}
		}
	}

	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//自动操作
	if (gfx.proc(hWnd, message, wParam, lParam))return 0;

	switch (message)
	{
	case WM_CREATE:
	{
		//修改窗口大小和位置
		RECT rect;
		GetClientRect(hWnd, &rect);
		MoveWindow(hWnd, 0, 0, cxWindow + (cxWindow - rect.right), cyWindow + (cyWindow - rect.bottom), false);

		inited = true;
		return 0;
	}

	/*鼠标操作*/
	case WM_LBUTTONDOWN:
	{
		cX0 = cX;
		cY0 = cY;

		//设置栏按钮
		if (bSet)
		{
			//渲染模式
			if (gfx.m_x > setL && gfx.m_x < setR &&
				gfx.m_y > setT + 10 && gfx.m_y < setT + 10 + 20 + 5)
			{
				if (mode == FAST)mode = PRECISE;
				else mode = FAST;
				needRender = true;
			}
			//迭代次数
			if (gfx.m_x > setL && gfx.m_x < setR &&
				gfx.m_y > setT + 10 * 2 + 20 && gfx.m_y < setT + 10 * 3 + 20 * 2)
			{
				if (gfx.m_x >= setR - 100 && gfx.m_x < setR - 50)
				{
					if (iterN > 10)iterN -= 10;
					needRender = true;
				}
				if (gfx.m_x >= setR - 50)
				{
					iterN += 10;
					needRender = true;
				}
			}
			//图例
			if (gfx.m_x > setL && gfx.m_x < setR &&
				gfx.m_y > setT + 10 * 3 + 20 * 2 && gfx.m_y < setT + 10 * 4 + 20 * 3)
			{
				bLegend = !bLegend;
			}
			//重置
			if (gfx.m_x > setL && gfx.m_x < setR &&
				gfx.m_y > setT + 10 * 4 + 20 * 3 && gfx.m_y < setT + 10 * 5 + 20 * 4)
			{
				mode = FAST;
				cX = -0.5;
				cY = 0;
				zoom = 500;
				iterN = 80;
				bLegend = true;
				needRender = true;
			}
			//保存到图片
			if (gfx.m_x > setL && gfx.m_x < setR &&
				gfx.m_y > setT + 10 * 5 + 20 * 4 && gfx.m_y < setT + 10 * 6 + 20 * 5)
			{
				std::wstring filename = std::to_wstring(time(NULL)) + L".png";
				img.save_to_file(filename.c_str());
				std::wstring temp = L"已保存到 ";
				temp += File::get_dir();
				temp += filename;
				temp += L"。";
				MessageBox(hWnd, temp.c_str(), L"提示", MB_OK | MB_ICONINFORMATION);
				gfx.m_down = false;//手动修复bug
			}
		}
		return 0;
	}
	case WM_MOUSEMOVE:
	{
		if (gfx.m_down)
		{
			cX = cX0 - gfx.m_dx / zoom * ((float)rW / cxWindow);
			cY = cY0 + gfx.m_dy / zoom * ((float)rH / cyWindow);//不反转虚轴
			needRender = true;
		}
		return 0;
	}
	case WM_MOUSEWHEEL:
	{
		zoom += gfx.m_whl * zoom / 10;
		needRender = true;
		return 0;
	}

	/*退出*/
	case WM_KEYDOWN:
	{
		if (wParam == VK_ESCAPE &&
			MessageBox(hWnd, L"确定要退出吗？", L"提示", MB_YESNO | MB_ICONINFORMATION) == IDYES)
		{
			PostQuitMessage(0);
		}
		else if (wParam == 'S')
		{
			bSet = !bSet;
		}
		return 0;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}