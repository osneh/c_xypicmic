#!/usr/bin/python
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

data = pd.read_csv("xlines.csv",delimiter=";")
df= pd.read_csv("inter.csv", delimiter=";")
df_clus = pd.read_csv("centroid.csv",delimiter=';')

clist = ['red','pink','purple','blue','cyan','green','olive','orange','brown','black','gray','grey','yellow',
         'silver','tan','navy','lightpink','peru']
for idx in range(data.index.stop) :
    color='black'
    way=data.track[idx]
    
    if way[0]=='B' :
        tcolor='blue'
    elif way[0]=='R':
        tcolor='red'
    elif way[0]=='Y':
        tcolor='yellow'
        
    line0 = (eval(data.pt0[idx]),eval(data.pt1[idx]))
    xs,ys = zip(*line0)
    plt.plot(xs,ys,'--', markersize=0, color=tcolor, linewidth=1.2)

for jdx in range(df.index.stop):
    px = df.x[jdx]
    py  = df.y[jdx]
    #plt.scatter(px,py,20,color='black',marker='o')
    
for kdx in range(df_clus.index.stop):
    cx = df_clus.x[kdx]
    cy = df_clus.y[kdx]
    #ll = '('+str(cy)+','+str(cy)+')'
    ll = str(cy)+','+str(cy)
    plt.scatter(cx,cy,facecolors='none',s= 150,color=clist[kdx],label=ll)

plt.ylim(-4000,4000)
plt.xlim(-4000,5000)
plt.xlabel('X-axis [$\mu$m]')
plt.ylabel('Y-axis [$\mu$m]')
plt.title('PICMIC$0$ Intersections, Centroids and Clusters')
#plt.grid()
plt.legend( fontsize=9,bbox_to_anchor=(0.5, 0., 0.5, 0.5) )
#plt.axis('off')
plt.savefig('plot.png')
plt.show()

exit()

