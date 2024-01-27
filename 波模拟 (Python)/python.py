import matplotlib.pyplot as plt
import numpy as np

SIZE = 100	#点阵的宽度

y_map = np.zeros([SIZE,SIZE],dtype='float32')	#各个点的纵坐标值（y值）
v_map = np.zeros([SIZE,SIZE],dtype='float32')	#各个点的速度
plt.ion()
i = 0

#绘图
def draw(title):
	plt.clf()
	
	#画出图像
	plt.imshow(abs(y_map),cmap='inferno',interpolation='gaussian')
	plt.clim(0,10)
	plt.colorbar()

	plt.title(title,fontsize=18)

	plt.show()

	plt.pause(1.0/30)

while True:
	draw(str(i))
	i+=1

	#迭代
	for x in range(1,SIZE - 1):
		for y in range(1,SIZE - 1):
			#让一些点不参与迭代计算，以让它们成为碰撞体
			if	(x < SIZE // 3 - SIZE // 20 or x > SIZE // 3 + SIZE // 20) or ((y > SIZE // 4) and
				(y < SIZE - 1 - SIZE // 4) and
				(y < SIZE // 2 - SIZE // 8 or y > SIZE // 2 + SIZE // 8)):

				#计算力
				F = 0.

				F += (y_map[y - 1][x] - y_map[y][x])
				F += (y_map[y + 1][x] - y_map[y][x])
				F += (y_map[y][x - 1] - y_map[y][x])
				F += (y_map[y][x + 1] - y_map[y][x])

				#计算速度
				v_map[y][x] += F / 4

	#更新y值
	for x in range(1,SIZE - 1):
		for y in range(1,SIZE - 1):
			y_map[y][x] += v_map[y][x]
			
	#波源
	for j in range(SIZE):
		v_map[j][1] += np.sin(i / 3)
